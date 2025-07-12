#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-23 18:05:02
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
 * */
#ifndef _HX_ADDRESS_RESOLVER_H_
#define _HX_ADDRESS_RESOLVER_H_

#include <netdb.h>
#include <string>

#include <HXLibs/exception/ErrorHandlingTools.hpp>

namespace HX::net {

/**
 * @brief 地址注册类
 */
class AddressResolver {
public:
    /**
     * @brief 用于保存地址引用
     */
    struct AddressRef {
        struct ::sockaddr* _addr; // 指向 sockaddr 结构体的指针
        ::socklen_t _addrlen;     // sockaddr 结构体的长度
    };

    /**
     * @brief 用于保存地址信息
     */
    struct Address {
        union {
            struct ::sockaddr _addr;                 // sockaddr 结构体
            struct ::sockaddr_storage _addr_storage; // 更大的结构体，适应不同的套接字地址类型
        };
        ::socklen_t _addrlen = sizeof(struct ::sockaddr_storage); // 初始化为 sockaddr_storage 的大小

        /**
         * @brief 类型转换操作符重载, 将 Address 转换为 AddressRef
         */
        operator AddressRef() {
            return {&_addr, _addrlen};
        }
    };

    /**
     * @brief 用于处理 addrinfo 结果
     */
    struct AddressInfo {
        struct ::addrinfo *_curr = nullptr; // 指向当前 addrinfo 结构体的指针

        /**
         * @brief 创建套接字、绑定它并监听连接
         * @return 服务器套接字
         */
        int createSocketAndBind() const {
            int serverFd = createSocket();
            AddressRef serveAddr = getAddress();
            int on = 1;
            setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &on,
                       sizeof(on)); // 设置端口复用(允许一个套接字在 TIME_WAIT
                                    // 状态下重新绑定到之前使用的地址和端口)
            setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &on,
                       sizeof(on)); // 允许多个套接字绑定到相同的地址和端口
            exception::LinuxErrorHandlingTools::checkError(
                "socket error",
                ::bind(serverFd, serveAddr._addr,
                       serveAddr._addrlen)); // 将套接字绑定IP和端口用于监听
            exception::LinuxErrorHandlingTools::checkError(
                "socket error",
                ::listen(
                    serverFd,
                    SOMAXCONN)); // 设置监听: 设定可同时排队的客户端最大连接个数
                                 // 其中`SOMAXCONN`: Linux可监听的最大数量
            return serverFd;
        }

        /**
         * @brief 建立socket套接字
         * @return 服务器套接字
         */
        int createSocket() const {
            /** 注:
             * @brief `::socket` 创建一个套接字
             * 此函数用于创建一个新的套接字。
             * @param domain 协议族/域，如 AF_INET（IPv4）、AF_INET6（IPv6）、AF_UNIX（Unix 域）等
             * @param type 套接字类型，如 SOCK_STREAM（面向连接的流套接字）、SOCK_DGRAM（无连接的数据报套接字）等
             * @param protocol 协议，通常为 0，表示使用默认协议
             * @return 成功时返回套接字描述符，失败时返回 -1
             * 
             * struct addrinfo {
             *     int ai_flags;             // 标志
             *     int ai_family;            // 地址族: AF_INET（IPv4）或 AF_INET6（IPv6）
             *     int ai_socktype;          // 套接字类型: SOCK_STREAM（流）或 SOCK_DGRAM（数据报）
             *     int ai_protocol;          // 协议: 如 IPPROTO_TCP（TCP）或 IPPROTO_UDP（UDP）
             *     size_t ai_addrlen;        // 地址长度
             *     struct sockaddr *ai_addr; // 指向 sockaddr 结构体的指针
             *     char *ai_canonname;       // 规范化的主机名
             *     struct addrinfo *ai_next; // 指向下一个 addrinfo 结构体的指针
             * };
             */
            return exception::LinuxErrorHandlingTools::checkError("socket error",
                ::socket(_curr->ai_family, _curr->ai_socktype, _curr->ai_protocol));
        }

        /**
         * @brief 获取当前 addrinfo 的地址引用
         * @return 当前 addrinfo 的地址引用
         */
        AddressRef getAddress() const {
            return {_curr->ai_addr, _curr->ai_addrlen};
        }

        /**
         * @brief 移动到下一个 addrinfo 结构体
         * @return 是否可以继续移动
         */
        [[nodiscard]] bool nextEntry() {
            _curr = _curr->ai_next;
            return _curr != nullptr;
        }
    };

private:
    // 指向 addrinfo 链表的头部
    struct ::addrinfo* _head = nullptr;

public:
    /**
     * @brief 解析主机名和服务名为 AddressInfo 链表
     * @param name 主机名或地址字符串(IPv4 的点分十进制表示或 IPv6 的十六进制表示)
     * @param service 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
     * @return 用于处理 addrinfo 结果 的结构体
     */
    AddressInfo resolve(std::string_view name, std::string_view service) {
        /**
         * hostname: 主机名或地址字符串 IPv4 的点分十进制表示或 IPv6 的十六进制表示
         * service: 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
         * hints: 可以是空指针, 也可以是指向某个 addrinfo 结构体的指针, 包含对所需地址类型的提示
         * result: 该函数通过 result 指针参数返回一个 addrinfo 结构体链表的指针
         */
        int err = getaddrinfo(name.data(), service.data(), nullptr, &_head);
        if (err) {
            auto ec = std::error_code(err, exception::LinuxErrorHandlingTools::gaiCategory());
            throw std::system_error(ec, name.data() + std::string{":"} + service.data());
        }
        return {_head};
    }
    
    // 默认构造函数
    AddressResolver() = default;

    // 移动构造函数
    AddressResolver(AddressResolver &&that) : _head(that._head) {
        that._head = nullptr;
    }

    /**
     * @brief 析构函数, 释放 addrinfo 链表
     */ 
    ~AddressResolver() noexcept {
        if (_head) {
            ::freeaddrinfo(_head);
        }
    }
};

} // namespace HX::net

#endif // _HX_ADDRESS_RESOLVER_H_
