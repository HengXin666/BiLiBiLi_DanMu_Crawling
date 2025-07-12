#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-09 22:54:00
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
#ifndef _HX_EXIT_AWAITER_H_
#define _HX_EXIT_AWAITER_H_

#include <coroutine>

namespace HX::coroutine {

/**
 * @brief 默认的协程控制: 在协程挂起(`co_await`)时会退出整个协程链条
 * @tparam T 协程返回值
 * @tparam P 协程 `promise_type`
 * @tparam IsStop 启动时候是否暂停 (默认为 `true` (暂停))
 */
template <typename T, typename P, bool IsStop = true>
struct ExitAwaiter {
    using promise_type = P;

    explicit ExitAwaiter(std::coroutine_handle<promise_type> coroutine)
        : _coroutine(coroutine)
    {}

    bool await_ready() const noexcept { 
        return !IsStop; 
    }

    /**
     * @brief 挂起当前协程
     * @param coroutine 这个是`co_await`的协程句柄 (而不是 _coroutine)
     * @return std::coroutine_handle<promise_type> 
     */
    std::coroutine_handle<promise_type> await_suspend(
        std::coroutine_handle<> coroutine
    ) const noexcept {
        _coroutine.promise()._previous = coroutine; // 此处记录 co_await 之前的协程, 方便恢复
        return _coroutine;
    }

    /**
     * @brief co_await 继续
     * @return T 
     * @throw 可能会抛出异常, 正因为如此, 才可以像普通函数一样捕获co_await的异常 (本类设计)
     */
    T await_resume() const {
        return _coroutine.promise().result();
    }

    std::coroutine_handle<promise_type> _coroutine;
};

} // namespace HX::coroutine

#endif // !_HX_EXIT_AWAITER_H_