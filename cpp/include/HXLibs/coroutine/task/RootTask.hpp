#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-04 14:36:43
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
#ifndef _HX_ROOT_TASK_H_
#define _HX_ROOT_TASK_H_

#include <HXLibs/coroutine/promise/Promise.hpp>

namespace HX::coroutine {

/**
 * @brief 根协程, 其绝对不应该被其他协程 co_await, 只能被 resume()
 */
template <typename T = void>
struct [[nodiscard]] RootTask {
    static_assert(std::is_void_v<T>, "Error: Only Supports Void Templates");

    // 协程 co_return 时候, 就进行析构; 不需要返回; 它 RAII 时候, 不会销毁协程!
    using promise_type = Promise<void, StopAwaiter<true>, StopAwaiter<false>>;

    constexpr RootTask(std::coroutine_handle<promise_type> h = nullptr)
        : _handle(h)
    {}

#if 0
    RootTask(RootTask&& that) : _handle(that._handle) {
        that._handle = nullptr;
    }

    RootTask &operator=(RootTask&& that) noexcept {
        std::swap(_handle, that._handle);
        return *this;
    }

    RootTask(RootTask const&) noexcept = delete;
    RootTask& operator=(RootTask const&) noexcept = delete;
#else
    RootTask &operator=(RootTask&&) noexcept = delete;
#endif

    void detach() && {
        static_cast<std::coroutine_handle<>>(std::move(*this)).resume();
    }

    ~RootTask() noexcept {
        _handle = nullptr;  // RAII 不销毁协程句柄
                            // 协程生命周期由它自己控制
    }

    constexpr operator std::coroutine_handle<>() const noexcept {
        return _handle;
    }

private:
    std::coroutine_handle<promise_type> _handle;
};

} // namespace HX::coroutine

#endif // !_HX_ROOT_TASK_H_