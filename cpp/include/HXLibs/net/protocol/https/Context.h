#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-09-01 22:19:58
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
#ifndef _HX_HTTPS_CONTEXT_H_
#define _HX_HTTPS_CONTEXT_H_

#include <string>

#ifdef __GNUC__
#define HOT_FUNCTION [[gnu::hot]]
#else
#define HOT_FUNCTION
#endif

// 前置声明
typedef struct bio_st BIO;
typedef struct ssl_ctx_st SSL_CTX;

namespace HX { namespace web { namespace protocol { namespace https {

/**
 * @brief Https验证模式设置参数包
 */
struct HttpsVerifyBuilder {
    std::string certificate;
    std::string privateKey;

    /**
     * @brief 校验模式
     * SSL_VERIFY_NONE: 不验证对方证书 (通常不推荐)
     * SSL_VERIFY_PEER: 验证对方证书
     * SSL_VERIFY_FAIL_IF_NO_PEER_CERT: 如果对方证书不存在, 则验证失败
     */
    int verifyMod = 0x01; // = SSL_VERIFY_PEER
};

/**
 * @brief Https 上下文, 用于管理公钥/秘钥, 以及提供`sslCtx`
 */
class Context {
    explicit Context() {};
    Context& operator=(Context&&) = delete;
public:
    /**
     * @brief 获取 Https 上下文
     * @return Context& 
     */
    HOT_FUNCTION static Context& getContext() {
        thread_local static Context context;
        return context;
    }

    /**
     * @brief 初始化服务端的SSL
     * @param verifyBuilder Https验证模式设置参数包
     * @throw 加载证书出现问题
     * @throw 证书不匹配
     */
    void initServerSSL(
        const HttpsVerifyBuilder& verifyBuilder
    );

    /**
     * @brief 初始化客户端的SSL
     * @param verifyBuilder Https验证模式设置参数包
     * @throw 加载证书出现问题
     * @throw 证书不匹配
     */
    void initClientSSL(
        const HttpsVerifyBuilder& verifyBuilder
    );

    BIO* getErrBio() const {
        return _errBio;
    }

    SSL_CTX* getSslCtx() const {
        return _sslCtx;
    }
private:
    BIO* _errBio = nullptr;     // 用于 SSL 错误输出的 BIO
    SSL_CTX* _sslCtx = nullptr; // 全局 SSL 上下文
};

}}}} // namespace HX::web::protocol::http

#undef HOT_FUNCTION

#endif // !_HX_HTTPS_CONTEXT_H_