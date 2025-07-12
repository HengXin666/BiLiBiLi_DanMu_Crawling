#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-31 21:12:33
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
#ifndef _HX_HTTP_H_
#define _HX_HTTP_H_

#include <string>
#include <string_view>
#include <unordered_map>

namespace HX::net {

namespace internal {

struct TransparentStringHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
};

struct TransparentStringEqual {
    using is_transparent = void;

    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
        return lhs == rhs;
    }
};

} // namespace internal

class Http {};

enum class HttpMethod {
    NIL = 0,
    GET,
    HEAD,
    POST,
    PUT,
    TRACE,
    PATCH,
    CONNECT,
    OPTIONS,
    DEL,
};

inline constexpr auto GET = HttpMethod::GET;
inline constexpr auto POST = HttpMethod::POST;
inline constexpr auto DEL = HttpMethod::DEL;
inline constexpr auto HEAD = HttpMethod::HEAD;
inline constexpr auto PUT = HttpMethod::PUT;
inline constexpr auto CONNECT = HttpMethod::CONNECT;

inline constexpr std::string_view getMethodStringView(HttpMethod const mthd) {
    using namespace std::string_view_literals;
    switch (mthd) {
    case HttpMethod::DEL:     return "DELETE"sv;
    case HttpMethod::GET:     return "GET"sv;
    case HttpMethod::HEAD:    return "HEAD"sv;
    case HttpMethod::POST:    return "POST"sv;
    case HttpMethod::PUT:     return "PUT"sv;
    case HttpMethod::PATCH:   return "PATCH"sv;
    case HttpMethod::CONNECT: return "CONNECT"sv;
    case HttpMethod::OPTIONS: return "OPTIONS"sv;
    case HttpMethod::TRACE:   return "TRACE"sv;
    default:                  return "NIL"sv;
    }
}

inline constexpr std::string_view CRLF{"\r\n", 2};

inline constexpr std::string_view HEADER_SEPARATOR_SV   = ": ";
inline constexpr std::string_view HEADER_END_SV         = "\r\n\r\n";
inline constexpr std::string_view CONTENT_LENGTH_SV     = "content-length";
inline constexpr std::string_view CONTENT_TYPE_SV       = "content-type";
inline constexpr std::string_view TRANSFER_ENCODING_SV  = "transfer-encoding";
inline constexpr std::string_view CONNECTION_SV         = "connection";

/**
 * @brief 实现了 std::string 作为 Key, 但是可以通过std::string_view 进行查找的功能
 */
using HeaderHashMap = std::unordered_map<
    std::string, 
    std::string, 
    internal::TransparentStringHash, 
    internal::TransparentStringEqual
>;

/**
 * @brief 断点续传参数包
 */
struct RangeRequestView {
    std::string_view reqType; // 请求类型
    HeaderHashMap const& reqHead;  // 请求头
};

} // namespace HX::net

#endif // !_HX_HTTP_H_