#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-10 22:37:12
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
#ifndef _HX_UNINITIALIZED_NON_VOID_VARIANT_H_
#define _HX_UNINITIALIZED_NON_VOID_VARIANT_H_

#include <utility>
#include <stdexcept>

#include <HXLibs/container/NonVoidHelper.hpp>
#include <HXLibs/container/Uninitialized.hpp>
#include <HXLibs/exception/ExceptionMode.hpp>

namespace HX::container {

template <typename... Ts>
struct UninitializedNonVoidVariant;

namespace internal {

template <std::size_t Idx, typename T>
struct UninitializedNonVoidVariantHead {
    NonVoidType<T> _data;
};

template <std::size_t Idx, typename... Ts>
struct UninitializedNonVoidVariantImpl;

template <std::size_t Idx, typename T>
struct UninitializedNonVoidVariantImpl<Idx, T> {
    using UVariant = UninitializedNonVoidVariantImpl;

    union {
        UninitializedNonVoidVariantHead<Idx, T> _head;
    };

    template <std::size_t Index>
    inline static constexpr NonVoidType<T>& get(UVariant& v) noexcept {
        return v._head._data;
    }

    template <std::size_t Index>
    inline static constexpr const NonVoidType<T>& get(UVariant const& v) noexcept {
        return v._head._data;
    }

    template <std::size_t Index>
    inline static constexpr auto&& get(UVariant&& v) noexcept {
        return std::move(v._head._data);
    }

    ~UninitializedNonVoidVariantImpl() noexcept {
        if constexpr (std::is_destructible_v<decltype(_head)>) {
            _head.~UninitializedNonVoidVariantHead<Idx, T>();
        }
    }
};

template <std::size_t Idx, typename T, typename... Ts>
struct UninitializedNonVoidVariantImpl<Idx, T, Ts...> {
    using UVariant = UninitializedNonVoidVariantImpl;

    union {
        UninitializedNonVoidVariantHead<Idx, T> _head;
        UninitializedNonVoidVariantImpl<Idx + 1, Ts...> _body;
    };

    template <std::size_t Index>
    inline static constexpr auto& get(UVariant& v) noexcept {
        if constexpr (Index == Idx) {
            return v._head._data;
        } else {
            return UninitializedNonVoidVariantImpl<Idx + 1, Ts...>::template get<Index>(v._body);
        }
    }

    template <std::size_t Index>
    inline static constexpr auto& get(UVariant const& v) noexcept {
        if constexpr (Index == Idx) {
            return v._head._data;
        } else {
            return UninitializedNonVoidVariantImpl<Idx + 1, Ts...>::template get<Index>(v._body);
        }
    }

    template <std::size_t Index>
    inline static constexpr auto&& get(UVariant&& v) noexcept {
        if constexpr (Index == Idx) {
            return std::move(v._head._data);
        } else {
            return std::move(
                UninitializedNonVoidVariantImpl<Idx + 1, Ts...>::template get<Index>(std::move(v._body))
            );
        }
    }

    ~UninitializedNonVoidVariantImpl() noexcept {
        if constexpr (std::is_destructible_v<decltype(_head)>) {
            _head.~UninitializedNonVoidVariantHead<Idx, T>();
        }
        if constexpr (std::is_destructible_v<decltype(_body)>) {
            _body.~UninitializedNonVoidVariantImpl<Idx + 1, Ts...>();
        }
    }
};

template <typename T, typename... Ts>
struct UninitializedNonVoidVariantIndex;

template <typename T, typename... Ts>
struct UninitializedNonVoidVariantIndex<T, UninitializedNonVoidVariant<Ts...>> {
    template <typename U, typename... Us>
    struct _FindIndex;
    
    template <typename U, typename... Us>
    struct _FindIndex {
        inline static constexpr std::size_t val 
            = std::is_same_v<T, NonVoidType<U>> 
                ? 0 
                : _FindIndex<Us...>::val + 1;
    };

    template <typename U>
    struct _FindIndex<U> {
        inline static constexpr std::size_t val 
            = !std::is_same_v<T, NonVoidType<U>>;
    };

    inline static constexpr std::size_t val
        = _FindIndex<Ts...>::val;
};

template <std::size_t Idx, typename... Ts>
struct UninitializedNonVoidVariantIndexToType;

template <std::size_t Idx, typename T, typename... Ts>
    requires (Idx <= sizeof...(Ts))
struct UninitializedNonVoidVariantIndexToType<Idx, UninitializedNonVoidVariant<T, Ts...>> {
    template <std::size_t _I, typename U, typename... Us>
    struct IndexToType {
        using Type 
            = std::conditional_t<
                Idx == _I,
                NonVoidType<U>,
                typename IndexToType<_I + 1, Us...>::Type
            >;
    };

    template <std::size_t _I, typename U>
    struct IndexToType<_I, U> {
        using Type = NonVoidType<U>;
    };

    using Type = IndexToType<0, T, Ts...>::Type;
};

template <std::size_t Idx, typename Lambda, typename... Ts>
constexpr decltype(auto) visitHelper(UninitializedNonVoidVariant<Ts...>& uv, Lambda& lambda) noexcept {
    return lambda(uv.template get<Idx, exception::ExceptionMode::Nothrow>());
}

} // namespace internal

inline constexpr std::size_t UninitializedNonVoidVariantNpos
    = static_cast<std::size_t>(-1);

// 如果不存在, 则返回 UVariant::N
template <typename T, typename UVariant>
inline constexpr std::size_t UninitializedNonVoidVariantIndexVal
    = internal::UninitializedNonVoidVariantIndex<T, UVariant>::val;

// 通过 Idx, 返回 UVariant 对应位置的类型
template <std::size_t Idx, typename... Ts>
using UninitializedNonVoidVariantIndexToType
    = internal::UninitializedNonVoidVariantIndexToType<
        Idx, UninitializedNonVoidVariant<Ts...>>::Type;

/**
 * @brief 一个支持任意类型的 Variant, 内部类型可以重复, 支持 void
 * @tparam ...Ts
 */
template <typename... Ts>
struct UninitializedNonVoidVariant {
    inline static constexpr std::size_t N = sizeof...(Ts);

    constexpr UninitializedNonVoidVariant() noexcept
        : _idx{UninitializedNonVoidVariantNpos}
    {}

    template <typename T>
        // requires (std::is_constructible_v<NonVoidHelper<T>, NonVoidHelper<Ts>...>) // @todo
    explicit UninitializedNonVoidVariant(T&& t) noexcept(std::is_nothrow_constructible_v<T, T&&>)
        : _idx{UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>}
    {
        /*
    核心思路: 判断 T 可以构造为 Ts..., 并且只能匹配成功一个
        1) 编译期处理: T 可以构造为 Ts... 则是 true, 放到一个数组中
        2) 一个编译期函数: for 看 是否出现 多个 true (如果是, 则不行)
        3) 同时函数返回对应的索引 (不合法就返回 sizeof...(Ts))
        4) 持有索引就可以找到准确的类型 U, 然后 T 构造为 U 了
        */
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        new (std::addressof(get<Idx, exception::ExceptionMode::Nothrow>())) T(std::forward<T>(t));
    }

    UninitializedNonVoidVariant(UninitializedNonVoidVariant const& that) 
        : UninitializedNonVoidVariant{}
    {
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            return;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        that.get<Idx, exception::ExceptionMode::Nothrow>()
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
    }

    UninitializedNonVoidVariant(UninitializedNonVoidVariant&& that) 
        : UninitializedNonVoidVariant{}
    {
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            return;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        std::move(std::move(that).template get<Idx, exception::ExceptionMode::Nothrow>())
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
    }

    constexpr std::size_t index() const noexcept {
        return _idx;
    }

    template <std::size_t Idx, exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
        requires (Idx < N)
    constexpr auto& get() & noexcept(EMode == exception::ExceptionMode::Nothrow) {
        if constexpr (EMode == exception::ExceptionMode::Throw) {
            if (_idx != Idx) [[unlikely]] {
                throw std::runtime_error("get: wrong index for variant");
            }
        }
        return internal::UninitializedNonVoidVariantImpl<0, Ts...>::template get<Idx>(_data);
    }

    template <std::size_t Idx, exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
        requires (Idx < N)
    constexpr auto& get() const & noexcept(EMode == exception::ExceptionMode::Nothrow) {
        if constexpr (EMode == exception::ExceptionMode::Throw) {
            if (_idx != Idx) [[unlikely]] {
                throw std::runtime_error("get: wrong index for variant");
            }
        }
        return internal::UninitializedNonVoidVariantImpl<0, Ts...>::template get<Idx>(_data);
    }

    template <std::size_t Idx, exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
        requires (Idx < N)
    constexpr auto&& get() && noexcept(EMode == exception::ExceptionMode::Nothrow) {
        if constexpr (EMode == exception::ExceptionMode::Throw) {
            if (_idx != Idx) [[unlikely]] {
                throw std::runtime_error("get: wrong index for variant");
            }
        }
        return std::move(
            internal::UninitializedNonVoidVariantImpl<0, Ts...>::template get<Idx>(std::move(_data))
        );
    }

    /**
     * @brief 获取类型为 T 的变量; 如果多个类型, 仅会提取最靠前的那个
     * @tparam T 
     */
    template <typename T, exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
        requires (UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant> < N)
    constexpr auto& get() & noexcept(EMode == exception::ExceptionMode::Nothrow) {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        return get<Idx, EMode>();
    }

    template <typename T, exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
        requires (UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant> < N)
    constexpr const auto& get() const & noexcept(EMode == exception::ExceptionMode::Nothrow) {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        return get<Idx, EMode>();
    }

    template <typename T, exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
        requires (UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant> < N)
    constexpr auto&& get() && noexcept(EMode == exception::ExceptionMode::Nothrow) {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        return std::move(std::move(*this).template get<Idx, EMode>());
    }

    template <typename T, typename... Args, typename NonVT = NonVoidType<T>,
        std::size_t Idx = UninitializedNonVoidVariantIndexVal<NonVT, UninitializedNonVoidVariant>>
            requires (Idx < N)
    NonVT& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<NonVT, Args...>) {
        del(std::make_index_sequence<N>{});
        _idx = Idx;
        new (std::addressof(get<Idx, exception::ExceptionMode::Nothrow>())) NonVT(std::forward<Args>(args)...);
        return get<Idx, exception::ExceptionMode::Nothrow>();
    }

    template <std::size_t Idx, typename... Args,
        typename T = UninitializedNonVoidVariantIndexToType<Idx, Ts...>>
            requires (Idx < N)
    T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        del(std::make_index_sequence<N>{});
        _idx = Idx;
        new (std::addressof(get<Idx, exception::ExceptionMode::Nothrow>())) T(std::forward<Args>(args)...);
        return get<Idx, exception::ExceptionMode::Nothrow>();
    }

    template <typename T>
        requires (!std::is_same_v<std::decay_t<T>, UninitializedNonVoidVariant>)
    UninitializedNonVoidVariant& operator=(T&& t) noexcept {
        emplace<T>(std::forward<T>(t));
        return *this;
    }

    UninitializedNonVoidVariant& operator=(UninitializedNonVoidVariant const& that) noexcept {
        if (std::addressof(that) == this) {
            return *this;
        }
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            reset();
            return *this;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        that.get<Idx, exception::ExceptionMode::Nothrow>()
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
        return *this;
    }

    constexpr UninitializedNonVoidVariant& operator=(UninitializedNonVoidVariant&& that) noexcept {
        if (std::addressof(that) == this) {
            return *this;
        }
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            reset();
            return *this;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        std::move(std::move(that).template 
                            get<Idx, exception::ExceptionMode::Nothrow>())
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
        return *this;
    }

    constexpr bool operator==(UninitializedNonVoidVariant const& that) noexcept {
        if (_idx == that._idx) {
            if (_idx == UninitializedNonVoidVariantNpos) {
                return true;
            }
            bool res = false;
            [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
                (((_idx == Idx) && [&] {
                    res = (get<Idx>() == that.get<Idx>());
                    return true;
                }()) || ...);
            } (std::make_index_sequence<N>{});
            return res;
        }
        return false;
    }

    constexpr bool operator!=(UninitializedNonVoidVariant const& that) noexcept {
        return !(*this == that);
    }

    constexpr void swap(UninitializedNonVoidVariant& that) noexcept {
        if (this == &that) [[unlikely]] {
            return;
        }
        if (auto idx = index(); idx == that.index()) {
            [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
                ((idx == Idx && [&] {
                    using std::swap; // 二段式 ADL, 优先匹配全局的, 再匹配 std 的
                    swap(get<Idx, exception::ExceptionMode::Nothrow>(),
                              that.get<Idx, exception::ExceptionMode::Nothrow>());
                    return true;
                }()) || ...);
            }(std::make_index_sequence<N>{});
            return;
        }
        UninitializedNonVoidVariant tmp;
        tmp = std::move(that);
        that = std::move(*this);
        *this = std::move(tmp);
    }

    void reset() noexcept {
        del(std::make_index_sequence<N>{});
    }
    
    ~UninitializedNonVoidVariant() noexcept {
        del(std::make_index_sequence<N>{});
    }

private:
    template <std::size_t... Idx>
    void del(std::index_sequence<Idx...>) noexcept {
        using DelFuncPtr = void (*)(UninitializedNonVoidVariant&);
        static constexpr DelFuncPtr delFuncs[N] {
            [](UninitializedNonVoidVariant& u) {
#if defined(_MSC_VER)
                // 实际上里面几乎就是套了一层模版, 然后 ->~T
                // 原来是 MSVC 不能 ->~NonVoidType<T> 啊!
                std::destroy_at(std::addressof(u.get<Idx, exception::ExceptionMode::Nothrow>()));
#elif defined(__GNUC__) || defined(__clang__)
                u.get<Idx, exception::ExceptionMode::Nothrow>().~NonVoidType<Ts>(); 
#else
                // 暂时不支持当前编译器
                #error "Currently not supported by the compiler"
#endif
            }...
        };
        if (_idx != UninitializedNonVoidVariantNpos) {
            delFuncs[_idx](*this);
            // @todo 日后可以使用宏或者再是模版, 支持短路式的析构, 而不是跳表~
            _idx = UninitializedNonVoidVariantNpos;
        }
    }

    std::size_t _idx;

    union {
        internal::UninitializedNonVoidVariantImpl<0, Ts...> _data;
    };
};

template <std::size_t Idx, typename... Ts, 
    exception::ExceptionMode EMode = exception::ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx < UninitializedNonVoidVariant<Ts...>::N)>>
constexpr auto& get(
    UninitializedNonVoidVariant<Ts...>& v
) noexcept(EMode == exception::ExceptionMode::Nothrow) {
    return v.template get<Idx, EMode>();
}

template <std::size_t Idx, typename... Ts, 
    exception::ExceptionMode EMode = exception::ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx < UninitializedNonVoidVariant<Ts...>::N)>>
constexpr const auto& get(
    UninitializedNonVoidVariant<Ts...> const& v
) noexcept(EMode == exception::ExceptionMode::Nothrow) {
    return v.template get<Idx, EMode>();
}

template <std::size_t Idx, typename... Ts, 
    exception::ExceptionMode EMode = exception::ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx < UninitializedNonVoidVariant<Ts...>::N)>>
constexpr auto&& get(
    UninitializedNonVoidVariant<Ts...>&& v
) noexcept(EMode == exception::ExceptionMode::Nothrow) {
    return std::move(std::move(v).template get<Idx, EMode>());
}

template <typename T, typename... Ts,
    exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
constexpr auto& get(
    UninitializedNonVoidVariant<Ts...>& v
) noexcept(EMode == exception::ExceptionMode::Nothrow) {
    return v.template get<T, EMode>();
}

template <typename T, typename... Ts,
    exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
constexpr const auto& get(
    UninitializedNonVoidVariant<Ts...> const& v
) noexcept(EMode == exception::ExceptionMode::Nothrow) {
    return v.template get<T, EMode>();
}

template <typename T, typename... Ts,
    exception::ExceptionMode EMode = exception::ExceptionMode::Throw>
constexpr auto&& get(
    UninitializedNonVoidVariant<Ts...>&& v
) noexcept(EMode == exception::ExceptionMode::Nothrow) {
    return std::move(std::move(v).template get<T, EMode>());
}

template <typename Lambda, typename... Ts, 
    typename Res = decltype(std::declval<Lambda>()(
        get<0>(std::declval<UninitializedNonVoidVariant<Ts...>>())))>
constexpr Res visit(UninitializedNonVoidVariant<Ts...>& uv, Lambda&& lambda) {
    if (uv.index() == UninitializedNonVoidVariantNpos) [[unlikely]] {
        throw std::runtime_error("get: wrong index for variant");
    }
    if constexpr (std::is_void_v<Res>) {
        // void 返回值
        auto fun = [&] <std::size_t Idx> (std::index_sequence<Idx>) {
            lambda(uv.template get<Idx, exception::ExceptionMode::Nothrow>());
            return true;
        };
        [&] <std::size_t... Idx>(std::index_sequence<Idx...>) {
            ((uv.index() == Idx && fun(std::index_sequence<Idx>{})) || ...);
        }(std::make_index_sequence<sizeof...(Ts)>{});
        return;
    } else if constexpr (requires {
        new (nullptr) Res(std::forward(std::declval<Res&&>()));
    }) {
        // 返回 带参数的, 支持移动 or 拷贝构造
        Uninitialized<Res> res;
        auto fun = [&] <std::size_t Idx> (std::index_sequence<Idx>) {
            res.set(lambda(uv.template get<Idx, exception::ExceptionMode::Nothrow>()));
            return true;
        };
        [&] <std::size_t... Idx>(std::index_sequence<Idx...>) {
            ((uv.index() == Idx && fun(std::index_sequence<Idx>{})) || ...);
        }(std::make_index_sequence<sizeof...(Ts)>{});
        return res.move();
    } else {
        // 返回 如 Lambda 这类 不支持 移动 or 拷贝构造的
        // 您可以使用 std::function 来返回 ...

#if 1   // 原因见: https://github.com/HengXin666/HXTest/blob/main/src/06-std-analyse/test/08-Lambda/01_Lambda.cpp
        static_assert(sizeof...(Ts) < 0, 
            "visit requires the visitor to have the same "
            "return type for all alternatives of a variant");
#else
        return [&] <std::size_t... Idx>(std::index_sequence<Idx...>) -> Res {
            using FuncPtr = Res (*)(UninitializedNonVoidVariant<Ts...>&, Lambda&);
            static FuncPtr table[] = {
                [] (UninitializedNonVoidVariant<Ts...>& uv, Lambda& ld) -> Res {
                    if constexpr () {
                        return ld(uv.template get<Idx, exception::ExceptionMode::Nothrow>());
                    } else {
                        // [[unlikely]]
                        return std::declval<Res>();
                    }
                }...
            };
            return table[uv.index()](uv, lambda);
        }(std::make_index_sequence<sizeof...(Ts)>{});
#endif
    }
}

} // namespace HX::container

#endif // !_HX_UNINITIALIZED_NON_VOID_VARIANT_H_