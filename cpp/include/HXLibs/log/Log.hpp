#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-07 13:18:50
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
#ifndef _HX_LOG_H_
#define _HX_LOG_H_

#include <cstdio>

#include <HXLibs/log/serialize/FormatString.hpp>

namespace HX::log {

namespace internal {

struct Log {
    template <typename... Ts>
    void debug(Ts const&... ts) const {
#ifndef NDEBUG
        printf("\033[1;35m[DEBUG]\033[0m \033[38;5;244m"); // 紫色标签 + 灰内容
        ((printf("%s ", formatString(ts).c_str())), ...);
        printf("\033[0m\n");
#else
        (static_cast<void>(ts), ...);
#endif
    }

    template <typename... Ts>
    void info(Ts const&... ts) const {
        printf("\033[1;32m[INFO]\033[0m \033[32m"); // 亮绿标签 + 正常绿内容
        ((printf("%s ", formatString(ts).c_str())), ...);
        printf("\033[0m\n");
    }

    template <typename... Ts>
    void warning(Ts const&... ts) const {
        printf("\033[1;33m[WARNING]\033[0m \033[33m"); // 亮黄标签 + 正黄内容
        ((printf("%s ", formatString(ts).c_str())), ...);
        printf("\033[0m\n");
    }

    template <typename... Ts>
    void error(Ts const&... ts) const {
        printf("\033[1;31m[ERROR]\033[0m \033[31m"); // 亮红标签 + 正红内容
        ((printf("%s ", formatString(ts).c_str())), ...);
        printf("\033[0m\n");
    }
};

} // namespace internal

inline internal::Log hxLog;

} // namespace HX::log

#endif // !_HX_LOG_H_