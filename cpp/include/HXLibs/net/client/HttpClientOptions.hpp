#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-09 21:44:22
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
#ifndef _HX_HTTPCLIENTOPTIONS_H_
#define _HX_HTTPCLIENTOPTIONS_H_

#include <string>

#include <HXLibs/net/protocol/http/Http.hpp>
#include <HXLibs/utils/TimeNTTP.hpp>

namespace HX::net {

template <typename Timeout = decltype(utils::operator""_ms<'5', '0', '0', '0'>())>
    requires(requires { Timeout::Val; })
struct HttpClientOptions {
    // 代理地址
    std::string proxy = {};

    // 超时时间
    Timeout timeout = utils::operator""_ms<'5', '0', '0', '0'>(); // 5000ms
};

} // namespace HX::net

#endif // !_HX_HTTPCLIENTOPTIONS_H_