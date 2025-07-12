#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-07 21:09:45
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
#ifndef _HX_IO_H_
#define _HX_IO_H_

#include <HXLibs/coroutine/task/Task.hpp>
#include <HXLibs/coroutine/loop/EventLoop.hpp>
#include <HXLibs/net/socket/SocketFd.hpp>

#ifndef NDEBUG
    #include <HXLibs/log/Log.hpp>
#endif // !NDEBUG

namespace HX::net {

namespace internal {

#if defined(__linux__)

template <typename Timeout>
auto* getTimePtr() noexcept {
    // 为了对外接口统一, 并且尽可能的减小调用次数, 故模板 多实例 特化静态成员, 达到 @cache 的效果
    static auto to = coroutine::durationToKernelTimespec(
        Timeout::Val
    );
    return &to;
}

#endif

} // namespace internal

/**
 * @brief Socket IO 接口, 仅提供写入和读取等操作, 不会进行任何解析和再封装
 * @note 如果需要进行读写解析, 可以看 Res / Req 的接口
 */
class IO {
public:
    inline constexpr static std::size_t kBufMaxSize = 1 << 10;

    IO(coroutine::EventLoop& eventLoop)
        : _fd{kInvalidSocket}
        , _eventLoop{eventLoop}
    {}

    IO(SocketFdType fd, coroutine::EventLoop& eventLoop)
        : _fd{fd}
        , _eventLoop{eventLoop}
    {}

    IO& operator=(IO&&) noexcept = delete;

    coroutine::Task<int> recv(std::span<char> buf) {
        co_return co_await _eventLoop.makeAioTask()
                                     .prepRecv(_fd, buf, 0);
    }

    coroutine::Task<int> recv(std::span<char> buf, std::size_t n) {
        co_return co_await _eventLoop.makeAioTask()
                                     .prepRecv(_fd, buf.subspan(0, n), 0);
    }

    template <typename Timeout>
        requires(requires { Timeout::Val; })
    coroutine::Task<coroutine::WhenAnyReturnType<
        coroutine::AioTask,
        decltype(std::declval<coroutine::AioTask>().prepLinkTimeout({}, {}))
    >> recvLinkTimeout(std::span<char> buf) {
#if defined(__linux__)
        co_return co_await coroutine::AioTask::linkTimeout(
            _eventLoop.makeAioTask().prepRecv(_fd, buf, 0),
            _eventLoop.makeAioTask().prepLinkTimeout(
                internal::getTimePtr<Timeout>(), 0)
        );
#elif defined(_WIN32)
        #error "@todo"
#else
        #error "Does not support the current operating system."
#endif
    }

    /**
     * @brief 写入数据, 内部保证完全写入
     * @param buf 
     * @return coroutine::Task<> 
     */
    coroutine::Task<> send(std::span<char const> buf) {
        // io_uring 也不保证其可以完全一次性写入...
        for (std::size_t n = buf.size(); n; n -= static_cast<std::size_t>(
                exception::IoUringErrorHandlingTools::check(
                    co_await _eventLoop.makeAioTask()
                                       .prepSend(_fd, buf, 0)
            ))) {
            ;
        }
    }

    /**
     * @brief 写入数据, 内部保证完全写入
     * @param buf 
     * @param n 
     * @return coroutine::Task<> 
     */
    coroutine::Task<> send(std::span<char const> buf, std::size_t n) {
        co_await send(buf.subspan(0, n));
    }

    template <typename Timeout>
        requires(requires { Timeout::Val; })
    coroutine::Task<coroutine::WhenAnyReturnType<
        coroutine::AioTask,
        decltype(std::declval<coroutine::AioTask>().prepLinkTimeout({}, {}))
    >> sendLinkTimeout(std::span<char const> buf) {
#if defined(__linux__)
        co_return co_await coroutine::AioTask::linkTimeout(
            _eventLoop.makeAioTask().prepSend(_fd, buf, 0),
            _eventLoop.makeAioTask().prepLinkTimeout(
                internal::getTimePtr<Timeout>(), 0)
        );
#elif defined(_WIN32)
        #error "@todo"
#else
        #error "Does not support the current operating system."
#endif
    }

    /**
     * @brief 关闭套接字
     * @warning 注意, 内部不会抛异常! 需要外部自己检查
     * @return coroutine::Task<int> close的返回值
     */
    coroutine::Task<int> close() noexcept {
        /// @todo 此处可能也需要特化 ckose ? 因为 win 下的超时实际上就已经close了(?)
        auto res = co_await _eventLoop.makeAioTask()
                                           .prepClose(_fd);
        _fd = kInvalidSocket;
        co_return res;
    }

    /**
     * @brief 清空套接字
     * @warning 请注意, 希望你知道你在做什么! 而不是泄漏套接字!
     * @note 期望编译器可以自己优化下面方法为不调用, 仅为 debug 时候使用.
     */
    constexpr void reset() noexcept {
#ifndef NDEBUG
        _fd = kInvalidSocket;
#endif
    }

    operator coroutine::EventLoop&() {
        return _eventLoop;
    }

#ifdef NDEBUG
    ~IO() noexcept = default;
#else
    ~IO() noexcept {
        if (_fd != kInvalidSocket) [[unlikely]] {
            log::hxLog.error("IO 没有进行 close");
        }
    }
#endif // !NDEBUG

private:
    SocketFdType _fd;
    coroutine::EventLoop& _eventLoop;
};

inline auto checkTimeout(
    coroutine::WhenAnyReturnType<
        coroutine::AioTask,
        decltype(std::declval<coroutine::AioTask>().prepLinkTimeout({}, {}))
    >&& res
) {
    if (res.index() == 0) {
        return res.get<0, exception::ExceptionMode::Nothrow>();
    } else [[unlikely]] {
        throw std::runtime_error{"is Timeout"}; // 超时了
    }
}

} // namespace HX::net

#endif // !_HX_IO_H_