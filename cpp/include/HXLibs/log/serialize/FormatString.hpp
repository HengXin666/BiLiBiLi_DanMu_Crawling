#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-12 11:04:09
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
#ifndef _HX_FORMAT_STRING_H_
#define _HX_FORMAT_STRING_H_

#include <optional>
#include <tuple>
#include <variant>
#include <thread>
#include <filesystem>
#include <format>

#include <HXLibs/utils/ContainerConcepts.hpp>
#include <HXLibs/reflection/MemberName.hpp>
#include <HXLibs/utils/NumericBaseConverter.hpp>

namespace HX::log {

namespace internal {

inline constexpr std::string_view DELIMIER          = ", ";
inline constexpr std::string_view PARENTHESES_LEFT  = "(";
inline constexpr std::string_view PARENTHESES_RIGHT = ")";
inline constexpr std::string_view BRACKET_LEFT      = "[";
inline constexpr std::string_view BRACKET_RIGHT     = "]";
inline constexpr std::string_view KEY_VAK_PAIR      = ": ";
inline constexpr std::string_view BRACE_LEFT        = "{\n";
inline constexpr std::string_view BRACE_DELIMIER    = ",\n";
inline constexpr std::string_view BRACE_RIGHT       = "}";

struct FormatString {
    /**
     * @brief 辅助类
     */
    struct DepthRAII {
        constexpr DepthRAII(std::size_t& v) noexcept
            : _v{v}
        {
            ++_v;
        }

        DepthRAII& operator=(DepthRAII&&) noexcept = delete;

        constexpr ~DepthRAII() noexcept {
            --_v;
        }

        std::size_t& _v;
    };

    /**
     * @brief 前置缩进
     * @tparam Stream 
     * @param s 
     */
    template <bool isEnter = false, typename Stream>
    constexpr void addIndent(Stream& s) {
        using namespace std::string_view_literals;
        if constexpr (isEnter) {
            s.append("\n"sv);
        }
        for (std::size_t i = 0; i < _depth; ++i) {
            s.append(_indentStr);
        }
    }

    // 线程id 或者 路径
    template <typename T>
        requires (std::is_same_v<T, std::thread::id>
            || std::is_same_v<T, std::filesystem::path>)
    constexpr std::string make(T const& t) {
        return std::format("{}", t);
    }

    // bool
    constexpr std::string make(bool t) {
        using namespace std::string_literals;
        return t ? "true"s : "false"s;
    }

    // null
    template <typename NullType>
        requires (std::is_same_v<NullType, std::nullopt_t>
               || std::is_same_v<NullType, std::nullptr_t>
               || std::is_same_v<NullType, std::monostate>)
    constexpr std::string make(NullType const&) {
        using namespace std::string_literals;
        return "null"s;
    }

    // 数字类型 NF 表示浮点数的位数 (-1表示默认选项, 使用 std::format("{}", t) 格式化)
    template <typename T, size_t NF = static_cast<size_t>(-1)>
        requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
    constexpr std::string make(T const& t) {
        if constexpr (NF != static_cast<size_t>(-1)) {
            return std::format("{:." + std::format("{}", NF) + "f}", t);
        } else {
            return std::format("{}", t);
        }
    }
    
    // C风格数组
    template <typename T, std::size_t N>
    constexpr std::string make(const T (&arr)[N]) {
        std::string res;
        bool once = false;
        res += BRACKET_LEFT;
        for (const auto& it : arr) {
            if (once)
                res += DELIMIER;
            else
                once = true;
            res += make(it);
        }
        res += BRACKET_RIGHT;
        return res;
    }
    
    // std常见的支持迭代器的单元素容器
    template <utils::SingleElementContainer Container>
    constexpr std::string make(Container const& arr) {
        std::string res;
        bool once = false;
        res += BRACKET_LEFT;
        for (const auto& it : arr) {
            if (once)
                res += DELIMIER;
            else
                once = true;
            res += make(it);
        }
        res += BRACKET_RIGHT;
        return res;
    }

    // 聚合类
    template <typename T>
        requires (std::is_aggregate_v<T> 
              && !std::is_same_v<T, std::monostate>)
    constexpr std::string make(T const& obj) {
        constexpr std::size_t Cnt = reflection::membersCountVal<T>;
        std::string res;
        res += BRACE_LEFT;
        {
            if constexpr (Cnt > 0) {
                DepthRAII _{_depth};
                reflection::forEach(const_cast<T&>(obj), [&] <std::size_t I> (
                    std::index_sequence<I>, auto name, auto& val
                ) {
                    addIndent(res);
                    res += make(name);
                    res += KEY_VAK_PAIR;
                    res.append(make(val));
    
                    if constexpr (I < Cnt - 1) {
                        res += BRACE_DELIMIER;
                    }
                });
            }
        }
        addIndent<true>(res);
        res += BRACE_RIGHT;
        return res;
    }

    // std常见的支持迭代器的键值对容器
    template <utils::KeyValueContainer Container>
    constexpr std::string make(const Container& map) {
        std::string res;
        res += BRACE_LEFT;
        {
            DepthRAII _{_depth};
            bool once = false;
            for (const auto& [k, v] : map) {
                if (once)
                    res += BRACE_DELIMIER;
                else
                    once = true;
                addIndent(res);
                res += make(k);
                res += KEY_VAK_PAIR;
                res += make(v);
            }
        }
        addIndent<true>(res);
        res += BRACE_RIGHT;
        return res;
    }

    // str相关的类型
    template <utils::StringType ST>
    constexpr std::string make(const ST& t) {
        std::string res;
        res += '"';
        res += t;
        res += '"';
        return res;
    }

    // const char* C字符串指针
    constexpr std::string make(const char* t) {
        return {t};
    }

    // char[N] C字符数组
    template <std::size_t N>
    constexpr std::string make(char (&t)[N]) {
        return {t, N};
    }

    // 普通指针
    template <typename T>
    constexpr std::string make(T* const& p) {
        std::string res = "0x";
        res += utils::NumericBaseConverter::hexadecimalConversion(reinterpret_cast<std::size_t>(p));
        return res;
    }

    // std::optional
    template <typename T>
    constexpr std::string make(std::optional<T> const& opt) {
        return opt ? make(*opt) : make(std::nullopt);
    }

    // std::pair
    template <typename T1, typename T2>
    constexpr std::string make(std::pair<T1, T2> const& p2) {
        std::string res;
        res += PARENTHESES_LEFT;
        res += make(std::get<0>(p2));
        res += DELIMIER;
        res += make(std::get<1>(p2));
        res += PARENTHESES_RIGHT;
        return res;
    }

    // std::tuple
    template <typename... Ts>
    constexpr std::string make(std::tuple<Ts...> const& tp) {
        constexpr std::size_t N = sizeof...(Ts);
        std::string res;
        res += PARENTHESES_LEFT;
        [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            ((res += make(std::get<Is>(tp)), 
              static_cast<void>(Is + 1 < N ? res += DELIMIER : res)
            ), ...);
        }(std::make_index_sequence<N>{});
        res += PARENTHESES_RIGHT;
        return res;
    }

    // std::variant
    template <typename... Ts>
    constexpr std::string make(std::variant<Ts...> const& v) {
        return std::visit([&](auto&& val) {
            return make(val);
        }, v);
    }

    std::size_t _depth = 0; // 嵌套深度
    std::string_view _indentStr{"    ", 4}; // 缩进字符串
};

} // namespace internal

template <typename... Ts>
    requires(sizeof...(Ts) > 0)
std::string formatString(Ts const&... ts) {
    std::string res;
    internal::FormatString fs{};
    ((res += fs.make(ts)), ...);
    return res;
}

} // namespace HX::log

#endif // !_HX_FORMAT_STRING_H_