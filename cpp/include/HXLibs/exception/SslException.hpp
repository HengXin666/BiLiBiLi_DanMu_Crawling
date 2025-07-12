#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-03-13 15:29:42
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
#ifndef _HX_SSL_EXCEPTION_H_
#define _HX_SSL_EXCEPTION_H_

#include <exception>
#include <string>

namespace HX::exception {

class SslException : public std::exception {
public:
    explicit SslException(const std::string& msg)
        : _msg("[SSL Error]: " + msg)
    {}

    const char* what() const noexcept override {
        return _msg.c_str();
    }
private:
    std::string _msg;
};

} // namespace HX::STL::exception

#endif // !_HX_SSL_EXCEPTION_H_