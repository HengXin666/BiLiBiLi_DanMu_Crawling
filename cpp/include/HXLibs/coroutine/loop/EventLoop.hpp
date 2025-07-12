#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-07 21:56:41
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HX_EVENT_LOOP_H_
#define _HX_EVENT_LOOP_H_

#include <thread>
#include <chrono>
#include <coroutine>

#include <HXLibs/platform/EventLoopApi.hpp>

#include <HXLibs/coroutine/task/Task.hpp>
#include <HXLibs/coroutine/task/AioTask.hpp>
#include <HXLibs/coroutine/loop/TimerLoop.hpp>
#include <HXLibs/coroutine/awaiter/WhenAny.hpp>
#include <HXLibs/exception/ErrorHandlingTools.hpp>

namespace HX::coroutine {

template <typename Rep, typename Period>
constexpr struct ::__kernel_timespec durationToKernelTimespec(
    std::chrono::duration<Rep, Period> dur
) noexcept {
    struct ::__kernel_timespec ts;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(dur - secs);
    ts.tv_sec = static_cast<__kernel_time64_t>(secs.count());
    ts.tv_nsec = static_cast<__kernel_time64_t>(nsecs.count());
    return ts;
}

namespace internal {

#if defined(__linux__)

/**
 * @brief 获取当前系统支持的最大io_uring环形队列的长度 (不要频繁调用, 这个只是测试使用的)
 * @warning 如果频繁调用会导致之前的东西没有在内核上完全释放 (说白了就是需要等会)
 * @return unsigned int 
 */
inline unsigned int getIoUringMaxSize() {
    struct ::io_uring ring;
    for (unsigned int entries = 64; entries; entries <<= 1) [[likely]] {
        int ret = ::io_uring_queue_init(entries, &ring, 0);
        if (!ret) {
            ::io_uring_queue_exit(&ring);
        } else {
            return entries >> 1;
        }
    }
    [[unlikely]] throw std::runtime_error{"IoUringMaxSize not find"}; // 找不到可用的大小
}

struct IoUring {
    explicit IoUring(unsigned int size = 1024U)
        : _ring{}
        , _numSqesPending{}
    {
        unsigned int flags = 0;
        exception::IoUringErrorHandlingTools::check(
            ::io_uring_queue_init(size, &_ring, flags)
        );
    }

    ~IoUring() noexcept {
        ::io_uring_queue_exit(&_ring);
    }

    AioTask makeAioTask() {
        // mandatory copy elision 场景
        // 编译器强制使用 RVO (返回值优化)
        // https://en.cppreference.com/w/cpp/language/copy_elision.html
        return AioTask{getSqe()};
    }

    bool isRun() const noexcept {
        return _numSqesPending;
    }

    void run(std::optional<std::chrono::system_clock::duration> timeout) {
        ::io_uring_cqe* cqe = nullptr;

        ::__kernel_timespec timespec; // 设置超时为无限阻塞
        ::__kernel_timespec* timespecPtr = nullptr;
        if (timeout.has_value()) {
            timespec = durationToKernelTimespec(*timeout);
            timespecPtr = &timespec;
        }

        // 阻塞等待内核, 返回是错误码; cqe是完成队列, 为传出参数
        int res = ::io_uring_submit_and_wait_timeout(
            &_ring, &cqe, 1, timespecPtr, nullptr);
        
        // 超时
        if (res == -ETIME) {
            // 内部直接 if (nr) { ... }, 此处调用传参 0, 毫无作用
            // ::io_uring_cq_advance(&_ring, 0); // 确保完成队列状态同步
            return;
        } else if (res < 0) [[unlikely]] { // 其他错误
            if (res == -EINTR) { // 被信号中断
                return;
            }
            throw std::system_error(-res, std::system_category());
        }

        unsigned head, numGot = 0;
        io_uring_for_each_cqe(&_ring, head, cqe) {
            ++numGot;
            if (cqe->res == -ECANCELED) { // 操作已取消 (比如超时了)
                continue;
            }
            auto* task = reinterpret_cast<AioTask*>(cqe->user_data);
            task->_res = cqe->res;
            tasks.push_back(task->_previous);
        }

        // 手动前进完成队列的头部 (相当于批量io_uring_cqe_seen)
        ::io_uring_cq_advance(&_ring, numGot);
        _numSqesPending -= static_cast<std::size_t>(numGot);
        for (const auto& it : tasks) {
            it.resume();
        }
        tasks.clear();
    }

private:
    ::io_uring_sqe* getSqe() {
        // 获取一个任务
        ::io_uring_sqe* sqe = ::io_uring_get_sqe(&_ring);
        while (!sqe) {
#if 0
            // 提交任务队列给内核 (为什么不是sqe, 因为sqe是从ring中get出来的, 故其本身就包含了sqe)
            int res = ::io_uring_submit(&_ring);
            if (res < 0) [[unlikely]] {
                if (res == -EINTR) {
                    continue;
                }
                throw std::system_error(-res, std::system_category());
            }
            // 提交了, 应该有空位, 那可以
            sqe = ::io_uring_get_sqe(&_ring);
#else
            ::io_uring_submit_and_wait(&_ring, 1); // 直接挂起等待操作系统完成了再说
            sqe = ::io_uring_get_sqe(&_ring);
            if (!sqe) {
                throw std::runtime_error("Still failed to get sqe after wait");
            }
#endif
        }
        ++_numSqesPending;
        return sqe;
    }

    ::io_uring _ring;
    std::size_t _numSqesPending; // 未完成的任务数
    std::vector<std::coroutine_handle<>> tasks; // 协程任务队列
                                                // 提取为成员, 避免频繁构造临时变量导致频繁扩容
};

#elif defined(_WIN32)
    /// @todo ...
#else
    #error "Does not support the current operating system."
#endif

using EventDrive

#if defined(__linux__)
    = IoUring;
#elif defined(_WIN32)
    = Iocp;
#else
    #error "Does not support the current operating system."
#endif

} // namespace internal

/**
 * @brief 协程事件循环
 */
struct EventLoop {
    EventLoop() 
        : _eventDrive{}
        , _timerLoop{}
    {}

    template <CoroutineObject T>
    void start(T& mainTask) {
        static_cast<std::coroutine_handle<>>(mainTask).resume();
    }
    
    void run() {
        for (;;) {
            auto timeout = _timerLoop.run();
            if (_eventDrive.isRun()) [[likely]] {
                _eventDrive.run(timeout);
            } else if (timeout) {
                std::this_thread::sleep_for(*timeout);
            } else [[unlikely]] {
                break;
            }
        }
    }

    auto makeTimer() {
        return TimerLoop::makeTimer(_timerLoop);
    }

    decltype(auto) makeAioTask() {
        return _eventDrive.makeAioTask();
    }
private:
    internal::EventDrive _eventDrive;
    TimerLoop _timerLoop;
};

} // namespace HX::coroutine

#endif // !_HX_EVENT_LOOP_H_