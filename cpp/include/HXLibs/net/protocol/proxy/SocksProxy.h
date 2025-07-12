#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-29 22:18:01
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
#ifndef _HX_SOCKS_PROXY_H_
#define _HX_SOCKS_PROXY_H_

#include <HXWeb/protocol/proxy/ProxyBase.h>

namespace HX { namespace web { namespace protocol { namespace proxy {

class Socks5Proxy : public HX::web::protocol::proxy::ProxyBash {
protected:
    explicit Socks5Proxy(const HX::web::client::IO<void>& io)
        : HX::web::protocol::proxy::ProxyBash(io)
    {}

    virtual HX::STL::coroutine::task::Task<> _connect(
        const std::string& url,
        const std::string& targetUrl
    ) override;

    friend HX::web::protocol::proxy::ProxyBash;
private:

    /**
     * @brief 子协商
     * @param username 
     * @param password 
     * @throw 失败
     */
    HX::STL::coroutine::task::Task<> subNegotiation(
        const std::string& username, 
        const std::string& password
    );

    /**
     * @brief 握手 | 协商
     * @param authentication 是否使用用户/密码进行验证
     * @throw 失败
     */
    HX::STL::coroutine::task::Task<> handshake(bool authentication);

    /**
     * @brief 发送代理请求
     * @param targetUrl 通过代理访问的目标服务器 URL
     * @throw 失败
     */
    HX::STL::coroutine::task::Task<> socks5ConnectRequest(const std::string& targetUrl);
};

}}}} // namespace HX::web::protocol::proxy

#endif // !_HX_SOCKS_PROXY_H_