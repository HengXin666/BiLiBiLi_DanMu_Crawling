#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-09 17:42:18
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
#ifndef _HX_HTTP_CLIENT_H_
#define _HX_HTTP_CLIENT_H_

#include <HXLibs/net/client/HttpClientOptions.hpp>
#include <HXLibs/net/protocol/http/Request.hpp>
#include <HXLibs/net/protocol/http/Response.hpp>
#include <HXLibs/net/socket/IO.hpp>
#include <HXLibs/net/socket/AddressResolver.hpp>
#include <HXLibs/net/protocol/url/UrlParse.hpp>
#include <HXLibs/coroutine/loop/EventLoop.hpp>
#include <HXLibs/container/ThreadPool.hpp>
#include <HXLibs/utils/ContainerConcepts.hpp>
#include <HXLibs/exception/ErrorHandlingTools.hpp>

#include <HXLibs/log/Log.hpp>

/**
 * @todo 还有问题: 如果客户端断线了, 我怎么检测到他是断线了?
 * @note 无法通过通知检测, 只能在读/写的时候发现
 * @note 实际上, 可以写一个读取, 在空闲的时候挂后台做哨兵
 * @note 但是iocp不能取消; 那就只能复用这个读, 但是如果是 https, 我们读取的内容要给openssl啊
 * @warning 感觉技术实现比较难... 而且用处不大, 不如直接判断是否出问题了来得快...
 */

namespace HX::net {

template <typename Timeout>
    requires(requires { Timeout::Val; })
class HttpClient {
public:
    /**
     * @brief 构造一个 HTTP 客户端
     * @param options 选项
     * @param threadNum 线程数
     */
    HttpClient(HttpClientOptions<Timeout>&& options = HttpClientOptions{}, uint32_t threadNum = 1) 
        : _options{std::move(options)}
        , _eventLoop{}
        , _cliFd{kInvalidSocket}
        , _pool{}
        , _host{}
        , _headers{}
    {
        _pool.setFixedThreadNum(threadNum);
        _pool.run<container::ThreadPool::Model::FixedSizeAndNoCheck>();
    }

    void run(coroutine::Task<> const& coMain) {
        _eventLoop.start(coMain);
        _eventLoop.run();
    }

    /**
     * @brief 判断是否需要重新建立连接
     * @return true 
     * @return false 
     */
    bool needConnect() const {
        return _cliFd == kInvalidSocket;
    }

    /**
     * @brief 发送一个 GET 请求, 其会在后台线程协程池中执行
     * @param url 请求的 URL
     * @return container::FutureResult<ResponseData> 
     */
    container::FutureResult<ResponseData> get(
        std::string url, 
        HeaderHashMap headers = {}
    ) {
        return requst<GET>(std::move(url), std::move(headers));
    }

    /**
     * @brief 发送一个 GET 请求, 其以协程的方式运行
     * @param url 请求的 URL
     * @return coroutine::Task<ResponseData> 
     */
    coroutine::Task<ResponseData> coGet(
        std::string url,
        HeaderHashMap headers = {}
    ) {
        co_return co_await coRequst<GET>(std::move(url), std::move(headers));
    }

    /**
     * @brief 发送一个 POST 请求, 其会在后台线程协程池中执行
     * @param url 请求的 URL
     * @param body 请求正文
     * @param contentType 请求正文类型
     * @return container::FutureResult<ResponseData> 
     */
    container::FutureResult<ResponseData> post(
        std::string url,
        HeaderHashMap headers,
        std::string body,
        HttpContentType contentType
    ) {
        return requst<POST>(
            std::move(url), std::move(headers), 
            std::move(body), contentType);
    }

    /**
     * @brief 发送一个 POST 请求, 其以协程的方式运行
     * @param url 请求的 URL
     * @param body 请求正文
     * @param contentType 请求正文类型
     * @return coroutine::Task<ResponseData> 
     */
    coroutine::Task<ResponseData> coPost(
        std::string url,
        HeaderHashMap headers,
        std::string body,
        HttpContentType contentType
    ) {
        co_return co_await coRequst<POST>(
            std::move(url), std::move(headers),
            std::move(body), contentType);
    }

    /**
     * @brief 异步的发送一个请求
     * @tparam Method 请求类型
     * @tparam Str 正文字符串类型
     * @param url url 或者 path (以连接的情况下)
     * @param body 正文
     * @param contentType 正文类型 
     * @return container::FutureResult<ResponseData> 响应数据
     */
    template <HttpMethod Method, utils::StringType Str = std::string>
    container::FutureResult<ResponseData> requst(
        std::string url,
        HeaderHashMap headers = {},
        Str&& body = {},
        HttpContentType contentType = HttpContentType::None
    ) {
        return _pool.addTask([this, _url = std::move(url),
                              _body = std::move(body), _headers = std::move(headers), 
                              contentType] {
            return coRequst<Method>(
                std::move(_url), std::move(_headers),
                std::move(_body), contentType
            ).start();
        });
    }

    /**
     * @brief 协程的发送一个请求
     * @tparam Method 请求类型
     * @tparam Str 正文字符串类型
     * @param url url 或者 path (以连接的情况下)
     * @param body 正文
     * @param contentType 正文类型 
     * @return coroutine::Task<ResponseData> 响应数据
     */
    template <HttpMethod Method, utils::StringType Str = std::string>
    coroutine::Task<ResponseData> coRequst(
        std::string url,
        HeaderHashMap headers = {},
        Str&& body = {},
        HttpContentType contentType = HttpContentType::None
    ) {
        container::FutureResult<ResponseData> res;
        _headers = std::move(headers);
        auto task = [this, ans = res.getFutureResult()](
            std::string _url, Str&& _body, HttpContentType _contentType
        ) -> coroutine::Task<> {
            if (needConnect()) {
                co_await makeSocket(_url);
            }
            ans->setData(co_await sendReq<Method>(
                _url, std::move(_body), _contentType)
            );
            co_return;
        }(std::move(url), std::move(body), contentType);
        _eventLoop.start(task);
        _eventLoop.run();
        co_return res.get();
    }

    /**
     * @brief 建立连接
     * @param url 
     * @return coroutine::Task<> 
     */
    coroutine::Task<> connect(std::string url) {
        co_await makeSocket(url);
        co_await sendReq<GET>(url);
    }

    HttpClient& operator=(HttpClient&&) noexcept = delete;
private:
    /**
     * @brief 建立 TCP 连接
     * @param url 
     * @return coroutine::Task<> 
     */
    coroutine::Task<> makeSocket(std::string_view url) {
        AddressResolver resolver;
        UrlInfoExtractor parser{_options.proxy.size() ? _options.proxy : url};
        auto entry = resolver.resolve(parser.getHostname(), parser.getService());
        _cliFd = exception::IoUringErrorHandlingTools::check(
            co_await _eventLoop.makeAioTask().prepSocket(
                entry._curr->ai_family,
                entry._curr->ai_socktype,
                entry._curr->ai_protocol,
                0
            )
        );
        auto sockaddr = entry.getAddress();
        co_await _eventLoop.makeAioTask().prepConnect(
            _cliFd,
            sockaddr._addr,
            sockaddr._addrlen
        );
    }

    template <HttpMethod Method, utils::StringType Str = std::string_view>
    coroutine::Task<ResponseData> sendReq(
        std::string const& url,
        Str&& body = std::string_view{},
        HttpContentType contentType = HttpContentType::None
    ) {
        IO io{_cliFd, _eventLoop};
        Request req{io};
        req.setReqLine<Method>(UrlParse::extractPath(url));
        preprocessHeaders(url, contentType, req);
        req._requestHeaders = std::move(_headers);
        if (body.size()) {
            // @todo 请求体还需要支持一些格式!
            req.setBody(std::forward<Str>(body));
        }
        do {
            try {
                co_await req.sendHttpReq<Timeout>();
                Response res{io};
                if (co_await res.parserRes<Timeout>() == false) [[unlikely]] {
                    break;
                }
                io.reset();
                co_return res.makeResponseData();
            } catch (std::exception const& e) {
                break;
            }
        } while (false);
        
        log::hxLog.error("解析出错");
        co_await io.close();
        _cliFd = kInvalidSocket;
        // @todo 日后此次改为异常
        co_return {};
    }

    /**
     * @brief 预处理请求头
     * @param url 
     * @param req 
     */
    void preprocessHeaders(std::string const& url, HttpContentType contentType, Request& req) {
        using namespace std::string_literals;
        try {
            auto host = UrlParse::extractDomainName(url);
            req.tryAddHeaders("Host"s, host);
            _host = std::move(host);
        } catch (std::exception const& e) {
            if (_host.size()) [[likely]] {
                req.tryAddHeaders("Host"s, _host);
            }
        }
        req.tryAddHeaders("Accept"s, "*/*"s);
        req.tryAddHeaders("Connection"s, "keep-alive"s);
        req.tryAddHeaders("User-Agent"s, "HXLibs/1.0"s);
        req.tryAddHeaders("Content-Type"s, getContentTypeStrView(contentType));
        req.tryAddHeaders("Date"s, utils::DateTimeFormat::makeHttpDate());
    }

    HttpClientOptions<Timeout> _options;
    coroutine::EventLoop _eventLoop;
    SocketFdType _cliFd;
    container::ThreadPool _pool;

    // 上一次解析的 Host
    std::string _host;

    // 请求头
    HeaderHashMap _headers;
};

HttpClient() -> HttpClient<decltype(utils::operator""_ms<'5', '0', '0', '0'>())>;

} // namespace HX::net

#endif // !_HX_HTTP_CLIENT_H_