#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-01-25 16:08:09
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
#ifndef _HX_STATUS_H_
#define _HX_STATUS_H_

#include <string_view>

namespace HX::net {

/**
 * @brief 响应状态码
 */
enum class Status {
    CODE_100 = 100, // Continue
    CODE_101 = 101, // Switching Protocols
    CODE_102 = 102, // Processing
    CODE_200 = 200, // OK
    CODE_201 = 201, // Created
    CODE_202 = 202, // Accepted
    CODE_203 = 203, // Non-Authoritative Information
    CODE_204 = 204, // No Content
    CODE_205 = 205, // Reset Content
    CODE_206 = 206, // Partial Content
    CODE_207 = 207, // Multi-Status
    CODE_226 = 226, // IM Used
    CODE_300 = 300, // Multiple Choices
    CODE_301 = 301, // Moved Permanently
    CODE_302 = 302, // Moved Temporarily
    CODE_303 = 303, // See Other
    CODE_304 = 304, // Not Modified
    CODE_305 = 305, // Use Proxy
    CODE_306 = 306, // Reserved
    CODE_307 = 307, // Temporary Redirect
    CODE_400 = 400, // Bad Request
    CODE_401 = 401, // Unauthorized
    CODE_402 = 402, // Payment Required
    CODE_403 = 403, // Forbidden
    CODE_404 = 404, // Not Found
    CODE_405 = 405, // Method Not Allowed
    CODE_406 = 406, // Not Acceptable
    CODE_407 = 407, // Proxy Authentication Required
    CODE_408 = 408, // Request Timeout
    CODE_409 = 409, // Conflict
    CODE_410 = 410, // Gone
    CODE_411 = 411, // Length Required
    CODE_412 = 412, // Precondition Failed
    CODE_413 = 413, // Request Entity Too Large
    CODE_414 = 414, // Request-URI Too Large
    CODE_415 = 415, // Unsupported Media Type
    CODE_416 = 416, // Requested Range Not Satisfiable
    CODE_417 = 417, // Expectation Failed
    CODE_422 = 422, // Unprocessable Entity
    CODE_423 = 423, // Locked
    CODE_424 = 424, // Failed Dependency
    CODE_425 = 425, // Unordered Collection
    CODE_426 = 426, // Upgrade Required
    CODE_428 = 428, // Precondition Required
    CODE_429 = 429, // Too Many Requests
    CODE_431 = 431, // Request Header Fields Too Large
    CODE_434 = 434, // Requested host unavailable
    CODE_444 = 444, // Close connection without sending headers
    CODE_449 = 449, // Retry With
    CODE_451 = 451, // Unavailable For Legal Reasons
    CODE_500 = 500, // Internal Server Error
    CODE_501 = 501, // Not Implemented
    CODE_502 = 502, // Bad Gateway
    CODE_503 = 503, // Service Unavailable
    CODE_504 = 504, // Gateway Timeout
    CODE_505 = 505, // HTTP Version Not Supported
    CODE_506 = 506, // Variant Also Negotiates
    CODE_507 = 507, // Insufficient Storage
    CODE_508 = 508, // Loop Detected
    CODE_509 = 509, // Bandwidth Limit Exceeded
    CODE_510 = 510, // Not Extended
    CODE_511 = 511, // Network Authentication Required
};

/**
 * @brief 获取响应状态码对应的字符串信息
 * @param statusCode 响应状态码
 * @return constexpr std::string_view 
 */
inline constexpr std::string_view getStatusCodeDataStrView(Status statusCode) {
    using namespace std::string_view_literals;
    switch (statusCode) {
    case Status::CODE_100: return "Continue"sv;
    case Status::CODE_101: return "Switching Protocols"sv;
    case Status::CODE_102: return "Processing"sv;
    case Status::CODE_200: return "OK"sv;
    case Status::CODE_201: return "Created"sv;
    case Status::CODE_202: return "Accepted"sv;
    case Status::CODE_203: return "Non-Authoritative Information"sv;
    case Status::CODE_204: return "No Content"sv;
    case Status::CODE_205: return "Reset Content"sv;
    case Status::CODE_206: return "Partial Content"sv;
    case Status::CODE_207: return "Multi-Status"sv;
    case Status::CODE_226: return "IM Used"sv;
    case Status::CODE_300: return "Multiple Choices"sv;
    case Status::CODE_301: return "Moved Permanently"sv;
    case Status::CODE_302: return "Moved Temporarily"sv;
    case Status::CODE_303: return "See Other"sv;
    case Status::CODE_304: return "Not Modified"sv;
    case Status::CODE_305: return "Use Proxy"sv;
    case Status::CODE_306: return "Reserved"sv;
    case Status::CODE_307: return "Temporary Redirect"sv;
    case Status::CODE_400: return "Bad Request"sv;
    case Status::CODE_401: return "Unauthorized"sv;
    case Status::CODE_402: return "Payment Required"sv;
    case Status::CODE_403: return "Forbidden"sv;
    case Status::CODE_404: return "Not Found"sv;
    case Status::CODE_405: return "Method Not Allowed"sv;
    case Status::CODE_406: return "Not Acceptable"sv;
    case Status::CODE_407: return "Proxy Authentication Required"sv;
    case Status::CODE_408: return "Request Timeout"sv;
    case Status::CODE_409: return "Conflict"sv;
    case Status::CODE_410: return "Gone"sv;
    case Status::CODE_411: return "Length Required"sv;
    case Status::CODE_412: return "Precondition Failed"sv;
    case Status::CODE_413: return "Request Entity Too Large"sv;
    case Status::CODE_414: return "Request-URI Too Large"sv;
    case Status::CODE_415: return "Unsupported Media Type"sv;
    case Status::CODE_416: return "Requested Range Not Satisfiable"sv;
    case Status::CODE_417: return "Expectation Failed"sv;
    case Status::CODE_422: return "Unprocessable Entity"sv;
    case Status::CODE_423: return "Locked"sv;
    case Status::CODE_424: return "Failed Dependency"sv;
    case Status::CODE_425: return "Unordered Collection"sv;
    case Status::CODE_426: return "Upgrade Required"sv;
    case Status::CODE_428: return "Precondition Required"sv;
    case Status::CODE_429: return "Too Many Requests"sv;
    case Status::CODE_431: return "Request Header Fields Too Large"sv;
    case Status::CODE_434: return "Requested host unavailable"sv;
    case Status::CODE_444: return "Close connection without sending headers"sv;
    case Status::CODE_449: return "Retry With"sv;
    case Status::CODE_451: return "Unavailable For Legal Reasons"sv;
    case Status::CODE_500: return "Internal Server Error"sv;
    case Status::CODE_501: return "Not Implemented"sv;
    case Status::CODE_502: return "Bad Gateway"sv;
    case Status::CODE_503: return "Service Unavailable"sv;
    case Status::CODE_504: return "Gateway Timeout"sv;
    case Status::CODE_505: return "HTTP Version Not Supported"sv;
    case Status::CODE_506: return "Variant Also Negotiates"sv;
    case Status::CODE_507: return "Insufficient Storage"sv;
    case Status::CODE_508: return "Loop Detected"sv;
    case Status::CODE_509: return "Bandwidth Limit Exceeded"sv;
    case Status::CODE_510: return "Not Extended"sv;
    case Status::CODE_511: return "Network Authentication Required"sv;
    }
    return ""sv;
}

} // namespace HX::net

#endif // !_HX_STATUS_H_