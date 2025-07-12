#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-08 09:54:58
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
#ifndef _HX_SOCKET_FD_API_H_
#define _HX_SOCKET_FD_API_H_

/**
 * @brief 跨平台 socket fd 类型定义
 * @note 只定义 socket_fd_t, 不引入额外命名、宏或接口
 */

#if defined(__linux__)
    namespace HX::platform {
        using SocketFdType = int;
        inline constexpr SocketFdType kInvalidSocket = -1;
    } // namespace HX::platform
#elif defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <WinSock2.h>
    // #include <Windows.h>
    namespace HX::platform {
        using SocketFdType = ::SOCKET;
        inline constexpr SocketFdType kInvalidSocket = ::INVALID_SOCKET;
    } // namespace HX::platform
#else
    #error "Unsupported operating system"
#endif


#endif // !_HX_SOCKET_FD_API_H_