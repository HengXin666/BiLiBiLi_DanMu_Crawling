#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-01-22 21:17:18
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
#ifndef _HX_ROUTER_TREE_H_
#define _HX_ROUTER_TREE_H_

#include <string>
#include <vector>
#include <stack>
#include <functional>

#include <HXLibs/net/protocol/http/Request.hpp>
#include <HXLibs/net/protocol/http/Response.hpp>
#include <HXLibs/coroutine/task/Task.hpp>
#include <HXLibs/container/RadixTree.hpp>

namespace HX::net {

using EndpointFunc = std::function<coroutine::Task<>(Request&, Response&)>;

class RouterTree {
    using Node = container::RadixTreeNode<std::string_view, EndpointFunc>;
public:
    explicit RouterTree() 
        : _root(std::make_shared<Node>())
        , _notFoundHandler([](Request &req,
                              Response &res) 
        -> coroutine::Task<> {
            static_cast<void>(req);
            co_return co_await res
                .setResLine(Status::CODE_404)
                .setContentType(HTML)
                .setBodyData(
    "<!DOCTYPE html><html lang=\"en\">"
    "<head><meta charset=\"UTF-8\"/>"
    "<title>404 Not Found</title>"
    "<style>"
        "body{font-family:Arial,sans-serif;text-align:center;padding:50px;background-color:#f4f4f4;}"
        "h1{font-size:100px;margin:0;color:#333;}"
        "p{font-size:24px;color:#666;}"
    "</style></head>"
    "<body>"
        "<h1>404</h1>"
        "<p>Not Found</p>"
        "<hr/>"
        "<p>HXLibs</p>"
    "</body></html>")
                .sendRes();
        })
    {}

    /**
     * @brief 设置`找不到路由`时候, 调用的端点
     * @param func 
     */
    void setNotFoundHandler(EndpointFunc&& func) {
        _notFoundHandler = std::move(func);
    }

    RouterTree& operator=(const RouterTree&) = delete;
    RouterTree(const RouterTree& ) = delete;

    void insert(
        std::vector<std::string_view>& buildLink,
        EndpointFunc&& endpoint
    ) {
        auto node = _root;
        for (auto& key : buildLink) {
            if (key.front() == '{') { // 特别处理: 如果是 {xxx}, 那么映射到 "*"
                key = "*";
            }
            auto findIt = node->child.find(key);
            if (findIt == node->child.end()) {
                node = node->child[key] = std::make_shared<Node>();
            } else {
                node = findIt->second;
            }
        }
        node->val = endpoint;
    }

    template <bool IsWildcard = false>
    const EndpointFunc& find(const std::vector<std::string_view>& findLink) const {
        const std::size_t n = findLink.size();
        std::size_t i = 0;
        std::stack<std::tuple<std::shared_ptr<Node>, std::size_t>> st;
        st.push({_root, static_cast<std::size_t>(0)});
        auto node = _root;
        while (st.size() && i < n) {
            auto& top = st.top();
            node = std::move(std::get<0>(top));
            i = std::get<1>(top);
            st.pop();
            auto findIt = node->child.end(); 
            if (i == n)
                goto End;

            for (; i < n; ++i) {
                findIt = node->child.find(findLink[i]);
                if (findIt == node->child.end()) {
                    // 以 {} 尝试
                    findIt = node->child.find("*");
                    if (findIt != node->child.end()) {
                        if constexpr (IsWildcard) { // 注意`/`
                            if (findLink[i].empty()) [[unlikely]] {
                                // 特殊的尾部标记, 表示 `/home/**`中
                                // 访问了`/home/`的情况
                                goto End;
                            }
                        }
                        st.push({node, n});
                        node = findIt->second;
                        if constexpr (IsWildcard) {
                            // 为了剔除`/`在末尾的影响
                            // 这时候如果存在 /files 与 /files/**
                            // 那么就会冲突
                            if (i == n - 2 && node->val.has_value()) {
                                return *node->val;
                            }
                        }
                        continue;
                    }
                End:
                    // 只能看看是否有**了
                    findIt = node->child.find("**");
                    if (findIt == node->child.end()) {
                        if (st.empty()) {
                            return _notFoundHandler;
                        }
                        break; // 回溯之前的
                    }
                    return *findIt->second->val;
                } else {
                    node = findIt->second;
                    if constexpr (IsWildcard) {
                        // 为了剔除`/`在末尾的影响
                        // 这时候如果存在 /files 与 /files/**
                        // 那么就会冲突
                        if (i == n - 2 && node->val.has_value()) {
                            return *node->val;
                        }
                    }
                }
            }
        }
        return node->val.has_value() ? *node->val : _notFoundHandler;
    }

private:
    std::shared_ptr<Node> _root;

    /**
     * @brief 路由找不到时, 调用的端点; 俗称`404`
     */
    EndpointFunc _notFoundHandler;
};

} // namespace HX::net

#endif // !_HX_ROUTER_TREE_H_