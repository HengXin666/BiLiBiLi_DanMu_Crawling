#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-07 13:42:47
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
#ifndef _HX_CONTAINER_CONCEPTS_H_
#define _HX_CONTAINER_CONCEPTS_H_

#include <ostream>
 
namespace HX::utils {

// 概念: 判断类型 T 是否是键值对的关联容器
template <typename T>
concept KeyValueContainer = requires(T t) {
    typename T::key_type;
    typename T::mapped_type;
};

template <typename T>
concept PairContainer = requires(T t) {
    typename T::first_type;
    typename T::second_type;
};

// 定义概念: 可输出到 std::ostream
template <typename T>
concept OStreamWritable = requires(std::ostream& os, T t) {
    { os << t } -> std::same_as<std::ostream&>;
};

// 概念: 如果这个类型和str沾边, 那么使用""包裹, 注: 普通的const char * 是不会包裹的
template <typename T>
concept StringType = OStreamWritable<T> && (requires(T t) {
    t.substr();                 // 大部分字符串类都有这个方法
} || requires(T t) {
    t.c_str();                  // std::string_view 没有这个
} || requires(T t) {
    typename T::traits_type;    // 标准库的字符串均有这个
});

// 概念: 如果这个类型和wstr沾边, 那么使用""包裹, 注: 普通的const w_char * 是不会包裹的
template <typename T>
concept WStringType = !OStreamWritable<T> && (requires(T t) {
    t.substr();                 // 大部分字符串类都有这个方法
} || requires(T t) {
    t.c_str();                  // std::wstring_view 没有这个
} || requires(T t) {
    typename T::traits_type;    // 标准库的字符串均有这个
});

// 概念: 判断类型 T 是否是单元素容器
template <typename T>
concept SingleElementContainer = (requires(T t) {
    typename T::value_type;
} && !KeyValueContainer<T>
  && !(StringType<T> || WStringType<T>)) 
  || requires (T t) {
    // std::span
    typename  T::element_type;
    t.subspan();
  };

} // namespace HX::utils

#endif // !_HX_CONTAINER_CONCEPTS_H_