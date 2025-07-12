#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-09 23:16:42
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
#ifndef _HX_NON_VOID_HELPER_H_
#define _HX_NON_VOID_HELPER_H_

namespace HX::container {

template <typename T = void>
struct NonVoidHelper {
    using Type = T;
};

template <>
struct NonVoidHelper<void> {
    using Type = NonVoidHelper;

    constexpr explicit NonVoidHelper() noexcept = default;

#if 0
    constexpr NonVoidHelper(NonVoidHelper const&) noexcept = default;
    constexpr NonVoidHelper(NonVoidHelper&&) noexcept = default;

    constexpr NonVoidHelper& operator=(NonVoidHelper const&) noexcept = default;
    constexpr NonVoidHelper& operator=(NonVoidHelper&&) noexcept = default;
#endif

    constexpr bool operator==(NonVoidHelper const&) const noexcept {
        return true;
    }
};

template <typename T>
using NonVoidType = NonVoidHelper<T>::Type;

} // namespace HX::container

#endif // !_HX_NON_VOID_HELPER_H_