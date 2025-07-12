#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-01-26 20:09:33
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
#ifndef _HX_PATH_VARIABLE_H_
#define _HX_PATH_VARIABLE_H_

#include <string_view>
#include <span>

namespace HX { namespace web { namespace protocol { namespace http {

class Request;

/**
 * @brief 路径变量, 如`/home/{id}`的`id`, 
 * @brief 存放的是解析后的结果字符串视图(指向的是Request的请求行)
 */
struct PathVariable {
    // 单个路径变量的结果数组
    std::span<std::string_view> wildcarDataArr;

    // 通配符的结果
    std::string_view UWPData;

    PathVariable(Request& req) noexcept;
    PathVariable(Request& req, std::span<std::string_view> wda) noexcept;
    PathVariable(Request& req, std::string_view _UWPData) noexcept;
    PathVariable(Request& req, std::span<std::string_view> wda, std::string_view _UWPData) noexcept;

    ~PathVariable() noexcept;
private:
    Request& _req;
};

}}}} // namespace HX::web::protocol::http

#endif // !_HX_PATH_VARIABLE_H_