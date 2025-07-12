#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-09 22:12:56
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
#ifndef _HX_PROMISE_H_
#define _HX_PROMISE_H_

#include <exception>
#include <coroutine>

#include <HXLibs/coroutine/awaiter/StopAwaiter.hpp>
#include <HXLibs/coroutine/awaiter/PreviousAwaiter.hpp>
#include <HXLibs/container/Uninitialized.hpp>

namespace HX::coroutine {

/**
 * @brief 协程控制体
 * @tparam T 返回类型
 * @tparam Init 初始化体 (initial_suspend)
 * @tparam Dele 删除体 (final_suspend)
 */
template <
    typename T, 
    typename Init = StopAwaiter<true>,
    typename Dele = PreviousAwaiter
>
struct Promise {
    using InitStrategy = Init;
    using DeleStrategy = Dele;

    Init initial_suspend() noexcept { return {}; }
    
    auto get_return_object() noexcept {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }
    
    auto final_suspend() noexcept { 
        if constexpr (requires { Dele{_previous}; }) {
            return Dele{_previous}; 
        } else {
            return Dele{};
        }
    }

    void return_value(T const& value) {
        if (_exception) [[unlikely]] {
            std::rethrow_exception(_exception);
        }
        _value.set(value);
    }

    void return_value(T&& value) {
        if (_exception) [[unlikely]] {
            std::rethrow_exception(_exception);
        }
        _value.set(std::move(value));
    }

    auto yield_value(T&& value) {
        _value.set(std::move(value));
        return PreviousAwaiter{_previous};
    }

    auto yield_value(T const& value) {
        _value.set(value);
        return PreviousAwaiter{_previous};
    }

    T result() {
        if (_exception) [[unlikely]] {
            std::rethrow_exception(_exception);
        }
        return _value.move();
    }
    
    void unhandled_exception() noexcept {
        _exception = std::current_exception();
    }

    Promise& operator=(Promise&&) noexcept = delete;

    std::coroutine_handle<> _previous{};
private:
    std::exception_ptr _exception{};
    container::Uninitialized<T> _value;
};

template <typename Init, typename Dele>
struct Promise<void, Init, Dele> {
    using InitStrategy = Init;
    using DeleStrategy = Dele;

    Init initial_suspend() noexcept { return {}; }

    auto get_return_object() noexcept {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    auto final_suspend() noexcept { 
        if constexpr (requires { Dele{_previous}; }) {
            return Dele{_previous}; 
        } else {
            return Dele{};
        }
    }

    void return_void() { }

    void result() {
        if (_exception) [[unlikely]] {
            std::rethrow_exception(_exception);
        }
    }

    void unhandled_exception() noexcept {
        _exception = std::current_exception();
    }

    Promise& operator=(Promise&&) noexcept = delete;

    std::coroutine_handle<> _previous{};
private:
    std::exception_ptr _exception{};
};

} // namespace HX::coroutine

#endif // !_HX_PROMISE_H_