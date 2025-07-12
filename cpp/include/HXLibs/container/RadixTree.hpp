#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-27 11:52:39
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
 * */
#ifndef _HX_RADIX_TREE_H_
#define _HX_RADIX_TREE_H_

#include <string>
#include <optional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace HX::container {

/**
 * @brief 基数树结点
 * @tparam T 存储的类型
 */
template <typename K, typename T>
struct RadixTreeNode {
    std::optional<T> val;
    std::unordered_map<
        K, 
        std::shared_ptr<RadixTreeNode<K, T>>
    > child;

    explicit RadixTreeNode() 
        : val(std::nullopt)
        , child()
    {}
};

/* 技术/时间有限, 这个严格来说只能算是字典树, 但是字符串版本.
 * 只能为了路由可以动态解析`/home/{id}/name/{str}\**`的内容, (将URL分为静态段和动态段, 通过`/`分割)
 * 然后放到vector<std::string_view>里面, 然后就像字典树一样匹配
 * 如果哈希下一个失败, 再思考是否是通配符`{id}`, 再失败, 才考虑是否是`\**` 
 */

/**
 * @brief 基数树(压缩前缀树) as 字典树
 * @tparam T 存储的类型
 */
template <typename T>
class RadixTree {
protected:
    using Node = RadixTreeNode<std::string, T>;
    std::shared_ptr<Node> _root;
public:
    explicit RadixTree() : _root(std::make_shared<Node>())
    {}

    RadixTree& operator=(const RadixTree&) = delete;
    RadixTree(const RadixTree& ) = delete;

    /**
     * @brief 构建字典树
     * @param buildLink 字典树构建链路, 如["home", "name", "{id}"], 即 <root> -> home -> name -> {id}
     * @param val 链路末端所具有的值
     * @warning 理论上必定插入成功, 只是可能会覆盖原来已有的!
     */
    void insert(const std::vector<std::string>& buildLink, const T& val) {
        auto node = _root;
        for (const auto& it : buildLink) {
            auto findIt = node->child.find(it);
            if (findIt == node->child.end()) {
               node = node->child[it] = std::make_shared<Node>();
            } else {
                node = findIt->second;
            }
        }
        node->val = val;
    }

    /**
     * @brief 从字典树中查找
     * @param findLink 查找链路, 如["home", "name", "{id}"], 即 <root> -> home -> name -> {id}
     * @return 查找结果的引用
     */
    std::optional<T> find(const std::vector<std::string>& findLink) const {
        auto node = _root;
        for (const auto& it : findLink) {
            auto findIt = node->child.find(it);
            if (findIt == node->child.end()) {
               return std::nullopt;
            } else {
                node = findIt->second;
            }
        }
        return node->val;
    }
};

} // namespace HX::container

#endif // _HX_RADIX_TREE_H_
