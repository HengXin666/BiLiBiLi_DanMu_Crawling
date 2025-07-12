#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-09 23:23:52
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
#ifndef _HX_AWAITER_H_
#define _HX_AWAITER_H_

#include <coroutine>

namespace HX::coroutine {

template <typename T>
concept Awaiter = requires(T const& t, std::coroutine_handle<> h) {
    { t.await_ready() } -> std::convertible_to<bool>; // trailing-return-type requirement (后置返回类型要求)
    t.await_suspend(h);
    t.await_resume();
};

template <typename T>
concept Awaitable = requires(T&& t) {
    t.operator co_await();
};

template <typename T>
concept AwaitableLike = Awaiter<T> || Awaitable<T>;

template <typename T>
concept CoroutineObject = requires(T&& t) { 
    typename T::promise_type;
    static_cast<std::coroutine_handle<>>(t);
};

template <AwaitableLike T>
using AwaiterReturnValue = decltype([](auto&& t) {
    if constexpr (Awaiter<decltype(t)>) {
        return t.await_resume();
    } else if constexpr (Awaitable<decltype(t)>) {
        return t.operator co_await().await_resume();
    } else {
        static_assert(!sizeof(T), "The type is not Awaiter");
    }
}(std::declval<T>()));

} // namespace HX::coroutine

#endif // !_HX_AWAITER_H_