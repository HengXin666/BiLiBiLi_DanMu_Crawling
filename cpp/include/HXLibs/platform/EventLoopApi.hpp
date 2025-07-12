#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-08 10:03:46
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
#ifndef _HX_EVENT_LOOP_API_H_
#define _HX_EVENT_LOOP_API_H_

/**
 * @brief 跨平台的事件循环API, 如 io_uring / iocp
 */

#if defined(__linux__)
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
    #elif defined(_MSC_VER)
        // 没事誰会msvc编译linux的它?
        #pragma warning(push)
        #pragma warning(disable : 4100 4101)
    #endif
    
    #include <liburing.h> // io_uring

    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic pop
    #elif defined(_MSC_VER)
        #pragma warning(pop)
        #pragma warning(pop)
    #endif
#elif defined(_WIN32)
    /// @todo 下面还需要精简!
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <WinSock2.h>
    #include <MSWSock.h>
    #include <Windows.h>

    #pragma comment(lib, "Ws2_32.lib")
    #pragma comment(lib, "Mswsock.lib")
#else
    #error "Does not support the current operating system."
#endif

#endif // !_HX_EVENT_LOOP_API_H_