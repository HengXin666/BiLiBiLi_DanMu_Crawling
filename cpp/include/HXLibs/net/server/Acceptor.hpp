#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-08 09:48:35
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
#ifndef _HX_ACCEPTOR_H_
#define _HX_ACCEPTOR_H_

#include <HXLibs/net/socket/SocketFd.hpp>
#include <HXLibs/coroutine/task/Task.hpp>
#include <HXLibs/coroutine/loop/EventLoop.hpp>
#include <HXLibs/net/socket/AddressResolver.hpp>
#include <HXLibs/net/router/Router.hpp>
#include <HXLibs/net/server/ConnectionHandler.hpp>
#include <HXLibs/exception/ErrorHandlingTools.hpp>

#include <HXLibs/log/Log.hpp>

namespace HX::net {

struct Acceptor {
    Acceptor(
        Router const& router,
        coroutine::EventLoop& eventLoop,
        AddressResolver::AddressInfo const& entry
    )
        : _router{router}
        , _eventLoop{eventLoop}
        , _entry{entry}
    {}

    Acceptor& operator=(Acceptor&&) noexcept = delete;

    template <typename Timeout>
        requires(requires { Timeout::Val; })
    coroutine::Task<> start(std::atomic_bool const& isRun) {
        auto serverFd = co_await makeServerFd();
        for (;;) [[likely]] {
            auto fd = exception::IoUringErrorHandlingTools::check(
                co_await _eventLoop.makeAioTask().prepAccept(
                    serverFd,
                    nullptr,    // 如果需要, 可以 getpeername(fd, ...) 获取的说...
                    nullptr,
                    0
                )
            );
            log::hxLog.debug("有新的连接:", fd);
            ConnectionHandler::start<Timeout>(fd, isRun, _router, _eventLoop).detach();
            if (!isRun.load(std::memory_order_acquire)) [[unlikely]] {
                break;  // 最在乎性能的关闭方式是, 关闭时候通过请求来解决 prepAccept 的阻塞
                        // 而不是写一个 whenAny 然后再写很复杂的逻辑什么的, 它浪费性能, 并且不是永远必须的
                        // 我们牺牲一个 atomic_bool 的性能已经很让步了..., @todo 这甚至应该作为一个编译选项(?)
            }
        }
        co_await _eventLoop.makeAioTask().prepClose(serverFd);
        log::hxLog.info("已退出...", serverFd);
    }

private:
    coroutine::Task<SocketFdType> makeServerFd() {
#if defined(__linux__)
        int serverFd = exception::IoUringErrorHandlingTools::check(
            co_await _eventLoop.makeAioTask().prepSocket(
                _entry._curr->ai_family,
                _entry._curr->ai_socktype,
                _entry._curr->ai_protocol,
                0
            )
        );

        auto serve_addr = _entry.getAddress();
        int on = 1;
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));

        exception::LinuxErrorHandlingTools::convertError<int>(
            ::bind(serverFd, serve_addr._addr, serve_addr._addrlen)
        ).expect("bind");

        exception::LinuxErrorHandlingTools::convertError<int>(
            ::listen(serverFd, SOMAXCONN)
        ).expect("listen");

        co_return serverFd;
#elif defined(_WIN32)
        co_return kInvalidSocket;
#else
    #error "Unsupported operating system"
#endif
    }

    Router const& _router;
    coroutine::EventLoop& _eventLoop;
    AddressResolver::AddressInfo const& _entry;
};

} // namespace HX::net

#endif // !_HX_ACCEPTOR_H_