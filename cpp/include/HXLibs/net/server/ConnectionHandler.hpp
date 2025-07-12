#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-08 10:45:58
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
#ifndef _HX_CONNECTION_HANDLER_H_
#define _HX_CONNECTION_HANDLER_H_

#include <HXLibs/coroutine/task/RootTask.hpp>
#include <HXLibs/coroutine/loop/EventLoop.hpp>
#include <HXLibs/net/socket/SocketFd.hpp>
#include <HXLibs/net/router/Router.hpp>
#include <HXLibs/net/socket/IO.hpp>
#include <HXLibs/net/protocol/http/Request.hpp>
#include <HXLibs/net/protocol/http/Response.hpp>

#include <HXLibs/log/Log.hpp>

namespace HX::net {

struct ConnectionHandler {

    template <typename Timeout>
        requires(requires { Timeout::Val; })
    static coroutine::RootTask<> start(
        SocketFdType fd,
        std::atomic_bool const& isRun,
        Router const& router,
        coroutine::EventLoop& eventLoop
    ) {
        using namespace std::string_view_literals;
        IO io{fd, eventLoop};
        Request  req{io};
        Response res{io};

        try {
            for (;;) {
                // 读
                if (!co_await req.parserReq<Timeout>()) [[unlikely]] {
                    break;
                }
                // 路由
                co_await router.getEndpoint(
                    req.getReqType(), 
                    req.getReqPath()
                )(req, res);
                
                // 只要不是明确写 close 的, 我就复用连接 (keep-alive)
                if (auto it = req.getHeaders().find(CONNECTION_SV);
                    (it != req.getHeaders().end() && it->second == "close"sv)
                    || !isRun.load(std::memory_order_acquire)
                ) [[unlikely]] {
                    break;
                }

                // 写 (由端点内部完成)

                // 清空
                req.clear();
                res.clear();
            }
        } catch (std::exception const& err) {
            // ps: 连接被对方重置 说明对方已经关闭连接, 而我还在等待读取, 这时候会异常, 可以忽视
            log::hxLog.error("发生异常:", err.what());
        } catch (...) {
            log::hxLog.error("发生未知错误!");
        }
        log::hxLog.debug("连接已断开");

        co_await io.close(); // @todo close 需要检查!
        co_return;
    }
};

} // namespace HX::net

#endif // !_HX_CONNECTION_HANDLER_H_