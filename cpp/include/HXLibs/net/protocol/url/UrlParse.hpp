#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-10 14:57:04
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
#ifndef _HX_URL_PARSE_H_
#define _HX_URL_PARSE_H_

#include <string>
#include <string_view>
#include <optional>
#include <cstdint>
#include <regex>
#include <stdexcept>
#include <unordered_map>

namespace HX::net {

struct AccountInfo {
    std::string_view  account;
    std::string_view password;
};

struct UrlParse {
    /**
     * @brief 从 URL 从提取出 Ptah 
     * @param url 
     * @return std::string_view Ptah
     */
    static std::string_view extractPath(std::string_view url) {
        std::size_t protocolFind = url.find("://");
        std::size_t pathFind = url.find("/",
            protocolFind == std::string_view::npos ? 0 : protocolFind + 3);
        return pathFind == std::string_view::npos ? "/" : url.substr(pathFind);
    }

    /**
     * @brief 从 URL 从提取出 域名
     * @param url 
     * @return std::string DomainName
     */
    static std::string extractDomainName(std::string const& url) {
        std::regex urlRegex(R"([a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+\.?)");
        std::smatch urlMatch;
        if (std::regex_search(url, urlMatch, urlRegex)) {
            return urlMatch.str();
        } else {
            throw std::invalid_argument("Invalid URL format: " + url);
        }
    }

    /**
     * @brief 从 URL 提取 Protocol(协议, 如`http:// -> http`)
     * @param url 
     * @return std::string_view Protocol
     * @throw 如果提取失败则会抛出异常
     */
    static std::string_view extractProtocol(std::string_view url) {
        std::size_t protocolFind = url.find("://");
        if (protocolFind == std::string_view::npos) {
            throw std::invalid_argument("Extract Protocol Error: " 
                + std::string{url.data(), url.size()});
        }
        return url.substr(0, protocolFind);
    }

    /**
     * @brief 从 URL 中解析出用户名和密码, 如`hx:666@www.loli.com -> hx, 666`
     * @return pair<用户名, 密码>, 如果解析不到, 则返回 std::nullopt
     */
    static std::optional<AccountInfo> extractUser(std::string_view url) {
        std::size_t findEnd = url.find('@');
        if (findEnd == std::string_view::npos) {
            return std::nullopt;
        }
        std::size_t findMid = url.rfind(':', findEnd - 1);
        std::size_t findStart = url.rfind('/', findMid - 1);
        if (findStart == std::string_view::npos) {
            return std::optional{AccountInfo{
                url.substr(0, findMid), 
                url.substr(findMid + 1, findEnd - findMid - 1)
            }};
        }
        return std::optional{AccountInfo{
            url.substr(findStart + 1, findMid - findStart - 1), 
            url.substr(findMid + 1, findEnd - findMid - 1)
        }};
    }

    /**
     * @brief 从 URL 剔除 Protocol(协议, 如`http:// -> http`)
     * @param url 
     * @return std::string Protocol
     * @throw 如果提取失败则会抛出异常
     */
    static std::string removeProtocol(std::string& url) {
        std::size_t protocolFind = url.find("://");
        if (protocolFind == std::string::npos) {
            throw std::invalid_argument("Extract Protocol Error: " + url);
        }
        std::string res = url.substr(0, protocolFind);
        url = url.substr(protocolFind + 3);
        return res;
    }

    /**
     * @brief 将协议转化为对应的端口, 如果常见的映射没有则假设 protocol 是端口的字符串形式
     * @param protocol 协议, 如`"http" -> 80`
     * @throw 不在常见的协议内
     * @return u_int16_t 
     */
    static uint16_t getProtocolPort(std::string const& protocol) {
        // 协议和端口号的映射
        static const std::unordered_map<std::string, uint16_t> protocolPorts = {
            {"http", 80},
            {"https", 443},
            {"ftp", 21},
            {"ftps", 990},
            {"sftp", 22},
            {"smtp", 25},
            {"pop3", 110},
            {"imap", 143},
            {"ldap", 389},
            {"ldaps", 636},
            {"telnet", 23},
            {"ssh", 22},
            {"dns", 53},
            {"dhcp", 67},
            {"ws", 80},
            {"wss", 443}
        };
        if (auto it = protocolPorts.find(protocol);
            it != protocolPorts.end()
        ) {
            return it->second;
        } else {
            return static_cast<uint16_t>(std::stoi(protocol)); // 未知协议, 则假设它是端口
        }
    }
};

/**
 * @brief 从 URL 中提取出 主机名称(域名或者ip) 和 端口(如果没有直接提供端口则使用协议的默认服务端口, 否则默认http)
 */
struct UrlInfoExtractor {
    UrlInfoExtractor(std::string_view url) {
        parseUrl(url);
    }

    std::string_view getHostname() const {
        return _hostname;
    }

    std::string_view getService() const {
        return _service;
    }

private:
    std::string _hostname {};
    std::string _service {};

    void parseUrl(std::string_view url) {
        std::size_t protocolFind = url.find("://");
        std::size_t userFind = url.find('@', protocolFind == std::string_view::npos ? 0 : protocolFind + 1);
        if (userFind == std::string_view::npos)
            userFind = protocolFind;
        std::size_t portFind = url.find(':', userFind == std::string_view::npos ? 0 : userFind + 1);

        if (protocolFind == std::string_view::npos && portFind == std::string_view::npos) {
            // 没有协议和端口，默认服务为 "http"
            _service = "http";
        } else {
            if (portFind != std::string_view::npos) {
                // 有端口，提取端口号
                std::size_t start = portFind + 1;
                std::size_t end = url.find_first_not_of("0123456789", start);
                _service = url.substr(start, end - start);
            } else if (protocolFind != std::string_view::npos) {
                // 有协议但没有端口
                _service = url.substr(0, protocolFind);
            }
        }
        _hostname = UrlParse::extractDomainName({url.data(), url.size()});
    }
};

} // namespace HX::net

#endif // !_HX_URL_PARSE_H_