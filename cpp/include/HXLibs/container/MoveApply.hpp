#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-10 11:24:17
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

#ifndef _HX_MOVE_APPLY_H_
#define _HX_MOVE_APPLY_H_

#include <tuple>

namespace HX::container {

namespace internal {

template <typename T>
struct HasPreserveTupleType : std::true_type {};

template <typename T>
struct HasPreserveTupleType<T&> : std::false_type {};

template <typename T>
struct HasPreserveTupleType<T const&> : std::false_type {};

template <std::size_t Idx, typename Tuple>
decltype(auto) getPreserveTupleByIdx(Tuple& t) {
    using T = std::tuple_element_t<Idx, Tuple>;
    if constexpr (HasPreserveTupleType<T>::value) {
        return std::move(std::get<Idx>(t));
    } else {
        return std::get<Idx>(t);
    }
}

template <typename Lambda, typename Tuple, std::size_t... Idx>
decltype(auto) moveApplyImpl(Lambda&& func, Tuple&& pt,
                             std::index_sequence<Idx...>) {
    return func(getPreserveTupleByIdx<Idx>(pt)...);
}

} // namespace internal

/**
 * @brief 判断这个类型是否有它自己的所有权
 * @tparam T 
 */
template <typename T>
constexpr bool HasPreserveTupleTypeVal =
    internal::HasPreserveTupleType<T>::value;

/**
 * @brief 将完美转发的类型萃取为所有权类型
 * @tparam T 
 * @note 此次的所有权类型是指, 除了 const& 和 & 我们会保留引用, 其他类型全部通过 std::move 转发
 */
template <typename T>
using PreserveTupleType = std::conditional_t<HasPreserveTupleTypeVal<T>,
                                             std::remove_reference_t<T>, T>;

/**
 * @brief 将完美转发的类型萃取为所有权类型的元组
 * @tparam Ts 
 * @param ts 
 * @return auto 
 */
template <typename... Ts>
std::tuple<PreserveTupleType<Ts>...> makePreserveTuple(Ts&&... ts) {
    return {std::forward<Ts>(ts)...};
}

/**
 * @brief 按照所有权语义对 Lambda 进行调用
 * @note 如果我们拥有所有权, 则直接 std::move 对象作为传参
 * @note 如果我们没有所有权, 则是继续传递引用
 * @warning 请调用 makePreserveTuple 把参数转化为所有权类型, 再调用此函数
 * @tparam Lambda 
 * @tparam Tuple 
 * @param func 
 * @param pt 
 * @return decltype(auto) 
 */
template <typename Lambda, typename Tuple>
decltype(auto) moveApply(Lambda&& func, Tuple&& pt) {
    constexpr std::size_t N = std::tuple_size_v<Tuple>;
    return internal::moveApplyImpl(
        std::forward<Lambda>(func), 
        std::forward<Tuple>(pt),
        std::make_index_sequence<N>{}
    );
};

} // namespace HX::container

#endif // !_HX_MOVE_APPLY_H_