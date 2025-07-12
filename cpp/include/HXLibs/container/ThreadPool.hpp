#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-10 09:49:57
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
#ifndef _HX_THREAD_POOL_H_
#define _HX_THREAD_POOL_H_

#include <thread>
#include <chrono>
#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <HXLibs/container/FutureResult.hpp>
#include <HXLibs/container/SafeQueue.hpp>
#include <HXLibs/container/MoveOnlyFunction.hpp>
#include <HXLibs/container/MoveApply.hpp>

namespace HX::container {

/**
 * @brief 线程池数据
 */
struct ThreadPoolData {
    uint32_t minThreadNum;      // 最小线程数
    uint32_t maxThreadNum;      // 最大线程数

    uint32_t taskNum;           // (未执行的)任务数
    uint32_t nowThreadNum;      // 当前线程数

    uint32_t runThreadNum;      // 运行的线程数
    uint32_t sleepThreadNum;    // 挂起的线程数
};

inline auto ThreadPoolDefaultStrategy = [](ThreadPoolData const& data) -> int {
    if (data.taskNum <= data.minThreadNum) {
        if (data.sleepThreadNum > data.runThreadNum) {
            return -static_cast<int>(std::max<std::size_t>(
                static_cast<std::size_t>(data.sleepThreadNum * 0.25),
                data.nowThreadNum - data.minThreadNum
            ));
        }
        return 0;
    }
    return static_cast<int>(std::min<std::size_t>(
        data.maxThreadNum - data.nowThreadNum,
        static_cast<std::size_t>(
            data.sleepThreadNum ? 0 : 2
        ) 
    ));
};

struct ThreadPool {
    enum class Model {
        FixedSizeAndNoCheck, // 不检查, 并且为恒定大小
        AsyncCheck           // 异步检查 (另开一个管理员线程, 以动态扩容)
    };

    ThreadPool()
        : _taskQueue{}
        , _opThread{}
        , _workers{}
        , _delIdQueue{}
        , _cv{}
        , _mtx{}
        , _minThreadNum{1}
        , _maxThreadNum{std::thread::hardware_concurrency()}
        , _runCnt{0}
        , _delCnt{0}
        , _isRun{false}
    {}

    /**
     * @brief 设置固定的线程数量
     * @warning 当且仅当 `run<Model::FixedSizeAndNoCheck>` 时候生效!
     * @param size 
     * @return ThreadPool& 
     */
    ThreadPool& setFixedThreadNum(uint32_t size) {
        _minThreadNum.store(size);
        return *this;
    }

    /**
     * @brief 设置最小的线程数量
     * @param size 
     * @return ThreadPool& 
     */
    ThreadPool& setMinThreadNum(uint32_t size) {
        _minThreadNum.store(size);
        return *this;
    }

    /**
     * @brief 设置最大的线程数量
     * @param size 
     * @return ThreadPool& 
     */
    ThreadPool& setMaxThreadNum(uint32_t size) {
        _maxThreadNum.store(size);
        return *this;
    }

    /**
     * @brief 添加任务, 任务应当为右值传入
     * @tparam Func 右值传入
     * @tparam Args 如果为左值, 请自行权衡生命周期, 以防止悬垂引用
     * @param func 
     * @param args 
     */
    template <typename Func, typename... Args, typename Res = std::invoke_result_t<Func, Args...>>
    FutureResult<Res> addTask(Func&& func, Args&&... args) {
        FutureResult<Res> res;
        auto cb = [func = std::move(func), 
                   ans = res.getFutureResult(),
                   argWap = makePreserveTuple(std::forward<Args>(args)...)
        ]() mutable {
            try {
                if constexpr (std::is_void_v<Res>) {
                    static_cast<void>(ans);
                    moveApply(std::move(func), std::move(argWap));
                } else {
                    ans->setData(moveApply(std::move(func), std::move(argWap)));
                }
            } catch (...) {
                ans->unhandledException();
            }
        };
        _taskQueue.emplace(std::make_unique<MoveOnlyFunctionAny<decltype(cb)>>(std::move(cb)));
        _cv.notify_one();
        return res;
    }

    /**
     * @brief 启动线程池
     * @tparam Md 运行模式 (默认为新建一个管理者线程 (异步))
     * @tparam Strategy 评判是否增删线程的方法 (返回值为 int(表示增删的线程数量), 传参为`ThreadPoolData const&`)
     * @param checkTimer 管理者轮询线程的时间间隔
     * @param strategy 是否增删线程的回调函数
     */
    template <Model Md = Model::AsyncCheck, typename Strategy = decltype(ThreadPoolDefaultStrategy),
        typename = std::enable_if_t<
            std::is_same_v<decltype(std::declval<Strategy>()(std::declval<ThreadPoolData const&>())), int>>>
    void run(
        [[maybe_unused]] std::chrono::milliseconds checkTimer = std::chrono::milliseconds {1000}, 
        [[maybe_unused]] Strategy strategy = ThreadPoolDefaultStrategy
    ) {
        _isRun = true;
        if constexpr (Md == Model::AsyncCheck) {
            _opThread = std::thread{&ThreadPool::check<decltype(strategy)>,
                                    this, checkTimer, std::move(strategy)};
        } else if constexpr (Md == Model::FixedSizeAndNoCheck) {
            // init
            makeWorker(_minThreadNum - _workers.size());
        } else {
            // 内部错误
            static_assert(!sizeof(Md), "Internal Errors");
        }
    }

    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool() noexcept {
        _isRun = false;
        if (_opThread.joinable()) {
            _opThread.join();
        }
        _cv.notify_all();
        for (const auto& [_, t] : _workers) {
            t->join();
        }
        if (!_delIdQueue.empty()) {
            clearDelIdQueue();
        }
    }
private:
    /**
     * @brief 管理者检查线程
     * @param checkTimer 检查 间隔的时间 (ms)
     */
    template <typename Strategy>
    void check(std::chrono::milliseconds checkTimer, Strategy&& strategy) {
        if (_workers.size() < _minThreadNum) {
            // init
            makeWorker(_minThreadNum - _workers.size());
        }
        while (_isRun) [[likely]] {
            auto taskSize = static_cast<uint32_t>(_taskQueue.size());
            auto runCnt = _runCnt.load();
            auto workerSize = static_cast<uint32_t>(_workers.size());
            if (int add = strategy(ThreadPoolData{
                _minThreadNum,
                _maxThreadNum,
                taskSize,
                workerSize,
                runCnt,
                workerSize - runCnt
            })) {
                if (add > 0) {
                    makeWorker(static_cast<std::size_t>(add));
                } else {
                    add = -add;
                    _delCnt = static_cast<uint32_t>(add);
                    for (int i = 0; i < add; ++i) {
                        _cv.notify_one();
                    }
                }
            }
            if (_delIdQueue.size()) {
                clearDelIdQueue();
            }
            std::this_thread::sleep_for(checkTimer);
        }
    }

    /**
     * @brief 创建生产者
     * @param num 创建的数量
     */
    void makeWorker(std::size_t num) {
        for (std::size_t i = 0; i < num; ++i) {
            auto up = std::make_unique<std::thread>([this] {
                std::decay_t<decltype(_taskQueue.front())> task;
                while (_isRun) [[likely]] {
                    {
                        // 挂起, 并等待任务
                        std::unique_lock lck{_mtx};
                        _cv.wait(lck, [&] {
                            return !_taskQueue.empty() || _delCnt > 0 || !_isRun;
                        });
                        if (!_isRun) [[unlikely]] {
                            break;
                        }
                        if (tryDecrementIfPositive(_delCnt)) {
                            _delIdQueue.emplace(std::this_thread::get_id());
                            break;
                        }
                        task = _taskQueue.frontAndPop();
                    }
                    ++_runCnt;
                    task->call(); // 执行任务
                    --_runCnt;
                }
            });
            auto id = up->get_id();
            _workers.emplace(id, std::move(up));
        }
    }

    /**
     * @brief 从待删除的队列中取出id, 并且从红黑树中删除
     */
    void clearDelIdQueue() {
        while (!_delIdQueue.empty()) {
            auto id = _delIdQueue.frontAndPop();
            auto it = _workers.find(id);
            if (it == _workers.end()) [[unlikely]] {
                throw std::runtime_error{"iterator failure"};
            }
            it->second->join();
            _workers.erase(it);
        }
    }

    /**
     * @brief 如果 val > 0, 则减小其值; CAS, 整个过程是原子的
     * @param val 
     * @return true  执行成功
     * @return false 执行失败, 不满足 val > 0
     */
    bool tryDecrementIfPositive(std::atomic_uint32_t& val) {
        uint32_t now = val.load(std::memory_order_relaxed);
        while (now > 0) {
            // 尝试原子替换 now -> now - 1
            if (val.compare_exchange_weak(
                now,                // 当前期望值
                now - 1,            // 设置的新值
                std::memory_order_acquire,
                std::memory_order_relaxed
            )) {
                return true;
            }
            // compare_exchange_weak 修改了 now
            // 循环重试直到 now <= 0 或替换成功
        }
        return false; // now <= 0
    }

    // 任务队列
    SafeQueue<std::unique_ptr<MoveOnlyFunction>> _taskQueue;

    // 管理者线程
    std::thread _opThread;

    // 消费者线程
    std::map<std::thread::id, std::unique_ptr<std::thread>> _workers;

    // 待删除队列
    SafeQueue<std::thread::id> _delIdQueue;

    // 消费者的信号量 & 对应的互斥锁
    std::condition_variable _cv;
    std::mutex _mtx;

    // 线程数设置
    std::atomic_uint32_t _minThreadNum; // 最小线程数
    std::atomic_uint32_t _maxThreadNum; // 最大线程数

    // 线程池状态
                                    // 总消费者线程数 = _consumerQueue.size()
    std::atomic_uint32_t _runCnt;   // 当前工作的线程数
                                    // 当前挂起的线程数 = _consumerQueue.size() - _runCnt
    std::atomic_uint32_t _delCnt;   // 需要删除的线程数                     
    std::atomic_bool _isRun;        // 线程池是否在运行
};

} // namespace HX::container

#endif // !_HX_THREAD_POOL_H_