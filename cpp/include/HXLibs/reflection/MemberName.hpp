#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-01-09 19:16:01
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
#ifndef _HX_MEMBER_NAME_H_
#define _HX_MEMBER_NAME_H_

#include <string_view>
#include <tuple>
#include <array>
#include <utility>

#include <HXLibs/reflection/MemberCount.hpp>

namespace HX::reflection {

namespace internal {

/**
 * @brief 获取编译期成员变量指针的符号, 并且提取其成员名称
 * @tparam ptr 编译期成员变量指针
 * @return constexpr std::string_view 
 */
template <auto ptr>
inline constexpr std::string_view getMemberName() {
#if defined(_MSC_VER)
    constexpr std::string_view func_name = __FUNCSIG__;
#else
    constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#endif

// 包裹一层`wrap`的原因: non-type template parameters of scalar type是在clang18才开始的,
// 而Class types as non-type template parameters是在clang12就支持了
#if defined(__clang__) && __clang_major__ >= 18
    auto split = func_name.substr(0, func_name.size() - 1);
    return split.substr(split.find_last_of(":.") + 1);
#elif defined(__GNUC__)
    auto split = func_name.substr(0, func_name.rfind(");"));
    return split.substr(split.find_last_of(":") + 1);
#elif defined(_MSC_VER)
    auto split = func_name.substr(0, func_name.rfind(">"));
    return split.substr(split.rfind("->") + 2);
#else
    static_assert(
        false, 
        "You are using an unsupported compiler. Please use GCC, Clang "
        "or MSVC or switch to the rfl::Field-syntax."
    );
#endif
}

/**
 * @brief 对于类型 T, 提供类静态成员
 * @tparam T 
 */
template <typename T>
struct StaticObj {
    inline static utils::remove_cvref_t<T> obj;
};

/**
 * @brief 获取类静态成员T
 * @tparam T 
 * @return constexpr utils::remove_cvref_t<T>& 
 */
template <typename T>
inline constexpr utils::remove_cvref_t<T>& getStaticObj() {
    return StaticObj<T>::obj;
}

/**
 * @brief 访问者类 主模版 (不可使用)
 * @tparam T `聚合类`类型
 * @tparam N 成员个数
 * @warning 如果匹配到此模版, 那么有两种可能:
 * @warning 1) 你的类有255+个成员
 * @warning 2) 你的类不是聚合类
 */
template <typename T, std::size_t N>
struct ReflectionVisitor {
    static constexpr auto visit() {
        static_assert(
            !sizeof(T),
            "\n\nThis error occurs for one of two reasons:\n\n"
            "1) You have created a struct with more than 255 fields, which is "
            "unsupported. \n\n"
            "2) Your struct is not an aggregate type.\n\n"
        );
    }

    static constexpr auto visit(T&) {
        static_assert(
            !sizeof(T),
            "\n\nThis error occurs for one of two reasons:\n\n"
            "1) You have created a struct with more than 255 fields, which is "
            "unsupported. \n\n"
            "2) Your struct is not an aggregate type.\n\n"
        );
    }

    template <typename Visitor>
    static constexpr auto visit(T&, Visitor&&) {
        static_assert(
            !sizeof(T),
            "\n\nThis error occurs for one of two reasons:\n\n"
            "1) You have created a struct with more than 255 fields, which is "
            "unsupported. \n\n"
            "2) Your struct is not an aggregate type.\n\n"
        );
    }
};

/**
 * @brief 偏特化模版
 * @tparam T `聚合类`类型
 */
template <typename T> 
struct ReflectionVisitor<T, 0> {
    static constexpr auto visit() {
        return std::tie();
    }

    static constexpr auto visit(T&) {
        return std::tie();
    }

    template <typename Visitor>
    static constexpr auto visit(T&, Visitor&& visitor) {
        return visitor();
    }
};

/**
 * @brief 偏特化模版生成 工具宏
 */
#define _HX_GENERATE_TEMPLATES_WITH_SPECIALIZATION_(N, ...) \
template <typename T>                                       \
struct ReflectionVisitor<T, N> {                            \
    static constexpr auto visit() {                         \
        auto& [__VA_ARGS__] = internal::getStaticObj<T>();  \
        constexpr auto t = std::tie(__VA_ARGS__);           \
        constexpr auto f = [](auto&... fs) {                \
            return std::make_tuple((&fs)...);               \
        };                                                  \
        return std::apply(f, t);                            \
    }                                                       \
                                                            \
    static constexpr auto visit(T& obj) {                   \
        auto& [__VA_ARGS__] = obj;                          \
        return std::tie(__VA_ARGS__);                       \
    }                                                       \
};

/**
 * @brief 展开工具宏, 然后 #undef 掉
 */
#include <HXLibs/reflection/MemberMacro.hpp>

/**
 * @brief 获取聚合类T的`tuple<成员指针...>`
 * @tparam T `聚合类`类型
 * @return constexpr tuple<成员指针...> 
 */
template <typename T>
inline constexpr auto getStaticObjPtrTuple() {
    return ReflectionVisitor<utils::remove_cvref_t<T>, membersCountVal<T>>::visit();
}

/**
 * @brief Get the Obj Tie object
 * @tparam T `聚合类`类型
 * @param obj 对象实例
 * @return tuple<obj成员的左值引用...>
 */
template <typename T>
inline constexpr auto getObjTie(T& obj) {
    return ReflectionVisitor<utils::remove_cvref_t<T>, membersCountVal<T>>::visit(obj);
}

/**
 * @brief 编译期静态for循环, 利用逗号表达式展开以将`tuple<成员指针...>`std::get<i>赋值到arr[i]
 * @tparam T `聚合类`类型
 * @tparam Arr std::array<>
 * @tparam Is std::array<>.size()
 * @param arr 需要赋值的`std::array`
 */
template <typename T, typename Arr, std::size_t... Is>
inline constexpr void staticForTupleSetArr(Arr& arr, std::index_sequence<Is...>) {
    constexpr auto tp = internal::getStaticObjPtrTuple<T>();
    ((arr[Is] = getMemberName<std::get<Is>(tp)>()), ...);
}

} // namespace internal

/**
 * @brief 获取`聚合类`所有成员变量的名称`array`
 * @tparam T `聚合类`类型
 * @return 所有成员变量的名称`array`
 */
template <typename T>
inline constexpr std::array<std::string_view, membersCountVal<T>> getMembersNames() {
    constexpr auto Cnt = membersCountVal<T>;
    std::array<std::string_view, Cnt> arr;
#if __cplusplus >= 202002L && (!defined(_MSC_VER) || _MSC_VER >= 1930)
    constexpr auto tp = internal::getStaticObjPtrTuple<T>(); // 获取 tuple<成员指针...>
    [&] <std::size_t... Is> (std::index_sequence<Is...>) {
        ((arr[Is] = internal::getMemberName<std::get<Is>(tp)>()), ...);
    } (std::make_index_sequence<Cnt>{});
#else
    internal::staticForTupleSetArr<T>(arr, std::make_index_sequence<membersCountVal<T>>{});
#endif
    return arr;
}

/**
 * @brief 遍历`聚合类`的所有成员, 并且以(`index`, `name`, `val`)的方式传入`visit`中
 * @tparam T `聚合类`类型
 * @tparam Visit 回调函数
 * @param obj 需要被遍历成员的对象实例
 * @param visit 回调函数
 */
template <typename T, typename Visit>
inline constexpr void forEach(T&& obj, Visit&& func) {
    constexpr auto Cnt = membersCountVal<T>;
    constexpr auto membersArr = getMembersNames<T>();
    auto tr = internal::getObjTie(obj);
    [&] <std::size_t... Is> (std::index_sequence<Is...>) {
        ((func(std::index_sequence<Is>{}, membersArr[Is], std::get<Is>(tr))), ...);
    } (std::make_index_sequence<Cnt>{});
}

} // namespace HX::reflection

#endif // !_HX_MEMBER_NAME_H_