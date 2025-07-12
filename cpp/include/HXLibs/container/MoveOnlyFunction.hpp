#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-10 10:50:39
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
#ifndef _HX_MOVE_ONLY_FUNCTION_H_
#define _HX_MOVE_ONLY_FUNCTION_H_

#include <utility>

namespace HX::container {

/**
 * @brief 一个类型擦除的, 仅支持std::move的 Function 基类
 */
struct MoveOnlyFunction {
    MoveOnlyFunction() = default;
    MoveOnlyFunction(MoveOnlyFunction&&) noexcept = default;

    MoveOnlyFunction& operator=(MoveOnlyFunction const&) = delete;
    MoveOnlyFunction& operator=(MoveOnlyFunction&&) noexcept = default;
    
    virtual void call() {}
    virtual ~MoveOnlyFunction() noexcept = default;
};

/**
 * @brief 仅支持std::move的 Function 模板子类
 * @tparam Lambda 
 */
template <typename Lambda>
struct MoveOnlyFunctionAny : public MoveOnlyFunction {
    MoveOnlyFunctionAny(Lambda&& cb)
        : _cb{std::move(cb)}
    {}
    
    MoveOnlyFunctionAny(MoveOnlyFunctionAny&&) noexcept = default;

    MoveOnlyFunctionAny& operator=(MoveOnlyFunctionAny const&) = delete;
    MoveOnlyFunctionAny& operator=(MoveOnlyFunctionAny&&) noexcept = default;

    void call() override {
        _cb();
    }
    
    ~MoveOnlyFunctionAny() noexcept override = default;
private:
    Lambda _cb;
};

} // namespace HX::container

#endif // !_HX_MOVE_ONLY_FUNCTION_H_