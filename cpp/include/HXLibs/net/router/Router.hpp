#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-01-22 21:24:10
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
#ifndef _HX_ROUTER_H_
#define _HX_ROUTER_H_

#include <functional>

#include <HXLibs/net/protocol/http/Http.hpp>
#include <HXLibs/net/protocol/http/Request.hpp>
#include <HXLibs/net/protocol/http/Response.hpp>
#include <HXLibs/net/router/RouterTree.hpp>
#include <HXLibs/net/router/RequestParsing.hpp>
#include <HXLibs/coroutine/task/Task.hpp>
#include <HXLibs/utils/StringUtils.hpp>

namespace HX::net {

class Router {
public:
    Router() = default;
    Router& operator=(Router&&) = delete;

    /**
     * @brief 获取路由
     * @param method 
     * @param path 
     * @return EndpointFunc 
     */
    const EndpointFunc& getEndpoint(
        std::string_view method,
        std::string_view path
    ) const {
        auto findLink = utils::StringUtil::split<std::string_view>(
            path, 
            "/", 
            {method}
        );
        if (path.back() == '/') { // 为了适配 /** 的情况
            findLink.emplace_back("");
            return _routerTree.find<true>(findLink);
        }
        return _routerTree.find(findLink);
    }

    /**
     * @brief 设置路由找不到时候的端点函数
     * @tparam Func 
     * @tparam Interceptors 
     * @param endpoint 端点函数
     * @param interceptors 拦截器
     */
    template <typename Func, typename... Interceptors>
    void setErrorEndpointFunc(Func&& endpoint, Interceptors&&... interceptors) {
        std::function<coroutine::Task<>(
            Request &, Response &)>
            realEndpoint =
                [this, endpoint = std::move(endpoint),
                 ... interceptors = interceptors] (Request &req,
                                                   Response &res) mutable
            -> coroutine::Task<> {
            static_cast<void>(this);
            bool ok = true;
            static_cast<void>((doBefore(interceptors, ok, req, res) && ...));
            if (ok) {
                co_await endpoint(req, res);
            }
            ok = true;
            static_cast<void>((doAfter(interceptors, ok, req, res) && ...));
        };
        _routerTree.setNotFoundHandler(std::move(realEndpoint));
    }

    /**
    * @brief 为服务器添加一个端点
    * @tparam Methods 请求类型, 如`GET`、`POST`
    * @tparam Func 端点函数类型
    * @tparam Interceptors 拦截器类型
    * @param key url, 如"/"、"home/{id}"
    * @param endpoint 端点函数
    * @param interceptors 拦截器
    */
    template <HttpMethod... Methods,
        typename Func,
        typename... Interceptors>
    void addEndpoint(std::string_view path, Func endpoint, Interceptors&&... interceptors) {
        if constexpr (sizeof...(Methods) == 1) {
            (_addEndpoint<Methods>(
                std::move(path),
                std::move(endpoint),
                std::forward<Interceptors>(interceptors)...
            ), ...);
        } else {
            (_addEndpoint<Methods>(
                path, 
                endpoint, 
                std::forward<Interceptors>(interceptors)...
            ), ...);
        }
    }

private:
    /**
     * @brief 添加路由端点
     * @tparam Method 
     * @tparam Func 
     * @tparam Interceptors 
     * @param path 
     * @param endpoint 
     * @param interceptors 
     */
    template <HttpMethod Method,
        typename Func,
        typename... Interceptors>
    void _addEndpoint(std::string_view path, Func&& endpoint, Interceptors&&... interceptors) {
        using namespace std::string_view_literals;
        auto isResolvePathVariable = path.find_first_of("{"sv) != std::string_view::npos;
        auto isParseWildcardPath = path.find("/**"sv) != std::string_view::npos;
        std::function<coroutine::Task<>(
            Request &, Response &)> realEndpoint;
        switch (isResolvePathVariable | (isParseWildcardPath << 1)) {
            case 0x0: // 不解析任何参数
                realEndpoint = [this, endpoint = std::move(endpoint),
                                ... interceptors = interceptors](
                                   Request &req,
                                   Response &res) mutable
                    -> coroutine::Task<> {
                    static_cast<void>(this);
                    bool ok = true;
                    static_cast<void>((doBefore(interceptors, ok, req, res) && ...));
                    if (ok) {
                        co_await endpoint(req, res);
                    }
                    ok = true;
                    static_cast<void>((doAfter(interceptors, ok, req, res) && ...));
                };
                break;
            case 0x1: { // 仅解析{}参数
                auto indexArr = RequestTemplateParsing::getPathWildcardAnalysisArr(path);
                realEndpoint = [this, endpoint = std::move(endpoint),
                                indexArr = std::move(indexArr),
                                ... interceptors = interceptors](
                                   Request &req,
                                   Response &res) mutable
                    -> coroutine::Task<> {
                    static_cast<void>(this);
                    bool ok = true;
                    auto pureRequesPath = req.getPureReqPath();
                    auto pathSplitArr = utils::StringUtil::split<std::string_view>(pureRequesPath, "/");
                    std::vector<std::string_view> wildcarArr;
                    for (auto idx : indexArr) {
                        wildcarArr.emplace_back(pathSplitArr[idx]);
                    }
                    req._wildcarDataArr = wildcarArr;
                    static_cast<void>((doBefore(interceptors, ok, req, res) && ...));
                    if (ok) {
                        co_await endpoint(req, res);
                    }
                    ok = true;
                    static_cast<void>((doAfter(interceptors, ok, req, res) && ...));
                };
                break;
            }
            case 0x2: {// 仅解析通配符
                auto UWPIndex = path.find("/**"sv);
                realEndpoint = [this, endpoint = std::move(endpoint), UWPIndex,
                                ... interceptors = interceptors](
                                   Request &req,
                                   Response &res) mutable
                    -> coroutine::Task<> {
                    static_cast<void>(this);
                    bool ok = true;
                    auto pureRequesPath = req.getPureReqPath();
                    std::string_view pureRequesPathView = pureRequesPath;
                    req._urlWildcardData = pureRequesPathView.substr(UWPIndex);
                    static_cast<void>((doBefore(interceptors, ok, req, res) && ...));
                    if (ok) {
                        co_await endpoint(req, res);
                    }
                    ok = true;
                    static_cast<void>((doAfter(interceptors, ok, req, res) && ...));
                };
                break;
            }
            case 0x3: { // 全都要解析
                auto indexArr = RequestTemplateParsing::getPathWildcardAnalysisArr(path);
                auto UWPIndex = RequestTemplateParsing::getUniversalWildcardPathBeginIndex(path);
                realEndpoint = [this, endpoint = std::move(endpoint),
                                indexArr = std::move(indexArr), UWPIndex,
                                ... interceptors = interceptors](
                                   Request &req,
                                   Response &res) mutable
                    -> coroutine::Task<> {
                    static_cast<void>(this);
                    bool ok = true;
                    auto pureRequesPath = req.getPureReqPath();
                    std::string_view pureRequesPathView = pureRequesPath;
                    auto pathSplitArr = utils::StringUtil::splitWithPos<std::string_view>(pureRequesPathView, "/");
                    std::vector<std::string_view> wildcarArr;
                    for (auto idx : indexArr) {
                        wildcarArr.emplace_back(pathSplitArr[idx].second);
                    }
                    req._urlWildcardData = pathSplitArr.size() > UWPIndex 
                            ? pureRequesPathView.substr(pathSplitArr[UWPIndex].first)
                            : ""sv;
                    req._wildcarDataArr = wildcarArr;
                    static_cast<void>((doBefore(interceptors, ok, req, res) && ...));
                    if (ok) {
                        co_await endpoint(req, res);
                    }
                    ok = true;
                    static_cast<void>((doAfter(interceptors, ok, req, res) && ...));
                };
                break;
            }
        }
        auto buildLink = utils::StringUtil::split<std::string_view>(
            path, 
            "/",
            {getMethodStringView(Method)}
        );
        _routerTree.insert(buildLink, std::move(realEndpoint));
    }

    template <typename T>
    bool doBefore(
        T& interceptors, 
        bool& ok, 
        Request& req, 
        Response& res
    ) {
        if constexpr (requires {
            { interceptors.before(req, res) } -> std::convertible_to<bool>;
        }) {
            ok = interceptors.before(req, res);
        } else if constexpr (requires {
            interceptors.before(req, res);
        }) {
            // 如果存在 before(req, res) 函数, 那么需要保证其返回值可以转化为 bool
            static_assert(!sizeof(T), "before() should return boolean");
        }
        return ok;
    }

    template <typename T>
    bool doAfter(
        T& interceptors, 
        bool& ok, 
        Request& req, 
        Response& res
    ) {
        if constexpr (requires {
            { interceptors.after(req, res) } -> std::convertible_to<bool>;
        }) {
            ok = interceptors.after(req, res);
        } else if constexpr (requires {
            interceptors.after(req, res);
        }) {
            // 如果存在 after(req, res) 函数, 那么需要保证其返回值可以转化为 bool
            static_assert(!sizeof(T), "after() should return boolean");
        }
        return ok;
    }

    RouterTree _routerTree{};
};

} // namespace HX::net

#endif // !_HX_ROUTER_H_