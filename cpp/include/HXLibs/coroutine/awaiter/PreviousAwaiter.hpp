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
#ifndef _HX_PREVIOUS_AWAITER_H_
#define _HX_PREVIOUS_AWAITER_H_

#include <coroutine>

namespace HX::coroutine {

/**
 * @brief 协程模式: 运行之前的协程
 */
struct PreviousAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> /*coroutine*/) const noexcept {
        if (_previous)
            return _previous;
        else
            return std::noop_coroutine();
    }

    void await_resume() const noexcept {}

    std::coroutine_handle<> _previous; // 之前的协程
};

} // namespace HX::coroutine

#endif // !_HX_PREVIOUS_AWAITER_H_