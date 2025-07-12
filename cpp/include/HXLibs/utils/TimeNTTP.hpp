#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-09 21:47:39
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
#ifndef _HX_TIME_NTTP_H_
#define _HX_TIME_NTTP_H_

#include <chrono>

namespace HX::utils {

namespace internal {

#if 0

/**
 * @brief 编译期快速幂
 * @param a 
 * @param b 
 * @return consteval 
 */
consteval std::size_t constexprPow(std::size_t a, std::size_t b) {
    std::size_t res = 1;
    while (b) {
        if (b & 1)
            res *= a;
        b >>= 1;
        a *= a;
    }
    return res;
}

/**
 * @brief 编译期字符转数字
 */
template <char... Cs>
struct CharToNum;

template <char C, char... Cs>
struct CharToNum<C, Cs...> {
    inline static constexpr std::size_t Val 
        = static_cast<std::size_t>(C - '0') * constexprPow(10, sizeof...(Cs))
        + CharToNum<Cs...>::Val;
};

template <char C>
struct CharToNum<C> {
    inline static constexpr std::size_t Val = C - '0';
};

/**
 * @brief 判断字面量字符是否都是数字 ['0', '9']
 * @tparam Cs 
 * @return true 
 * @return false 
 */
template <char... Cs>
consteval bool checkCharIsNum() {
    // return [] <std::size_t... Idx> (std::index_sequence<Idx...>) -> bool {
    //     return ([] <char C> () -> bool {
    //         return '0' <= C && C <= '9';
    //     }<Cs>() && ...);
    // }(std::make_index_sequence<sizeof...(Cs)>{});
    return (('0' <= Cs && Cs <= '9') && ...);
}

template <char... Cs>
consteval std::size_t constexprCharToNum() {
    if constexpr (!checkCharIsNum<Cs...>() || sizeof...(Cs) > 18) {
        // 文本字面量不能表示成数字
        static_assert(!sizeof...(Cs), "Text literal cannot be represented as numbers");
    }
    return CharToNum<Cs...>::Val;
}

#else

template <char... Cs>
consteval std::size_t constexprCharToNum() {
    static_assert((('0' <= Cs && Cs <= '9') && ...) && sizeof...(Cs) <= 18,
        "Only numeric characters allowed");
    constexpr char str[] = {Cs...};
    std::size_t value = 0;
    for (std::size_t i = 0; i < sizeof...(Cs); ++i) {
        value = value * static_cast<std::size_t>(10) 
              + static_cast<std::size_t>(str[i] - '0');
    }
    return value;
}

#endif

} // namespace internal

template <typename TimeType, std::size_t Time>
struct TimeNTTP {
    using Rep = typename TimeType::rep;
    using Period = typename TimeType::period;
    static constexpr auto Val = std::chrono::duration<Rep, Period>{Time};

    constexpr decltype(Val) toChrono() const noexcept {
        return Val;
    }

    constexpr operator decltype(Val)() {
        return Val;
    }
};

template <char... Time>
constexpr TimeNTTP<std::chrono::seconds,
                   internal::constexprCharToNum<Time...>()>
operator""_s() {
    return {};
}

template <char... Time>
constexpr TimeNTTP<std::chrono::milliseconds,
                   internal::constexprCharToNum<Time...>()>
operator""_ms() {
    return {};
}

template <char... Time>
constexpr TimeNTTP<std::chrono::microseconds,
                   internal::constexprCharToNum<Time...>()>
operator""_us() {
    return {};
}

template <char... Time>
constexpr TimeNTTP<std::chrono::nanoseconds,
                   internal::constexprCharToNum<Time...>()>
operator""_ns() {
    return {};
}

} // namespace HX::utils

#endif // !_HX_TIME_NTTP_H_