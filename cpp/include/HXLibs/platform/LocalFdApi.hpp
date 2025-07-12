#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-09 10:44:39
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
#ifndef _HX_LOCAL_FD_API_H_
#define _HX_LOCAL_FD_API_H_

/**
 * @brief 跨平台 本地 fd 类型定义
 * @note 只定义 local_fd_t, 不引入额外命名、宏或接口
 * @note 不要问我为什么, 你问一下win的架构师?
 */

#if defined(__linux__)
    #include <unistd.h>

    namespace HX::platform {
        using LocalFdType = int;
        inline constexpr LocalFdType kInvalidLocalFd = -1;
    } // namespace HX::platform
#elif defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
    namespace HX::platform {
        using LocalFdType = ::HANDLE;
        inline constexpr LocalFdType kInvalidLocalFd = ::INVALID_HANDLE_VALUE;
    } // namespace HX::platform
#else
    #error "Unsupported operating system"
#endif

#endif // !_HX_LOCAL_FD_API_H_