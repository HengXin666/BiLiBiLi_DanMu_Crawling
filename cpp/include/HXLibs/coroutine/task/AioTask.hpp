#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-07 22:24:56
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
#ifndef _HX_AIO_TASK_H_
#define _HX_AIO_TASK_H_

#include <span>

#include <HXLibs/platform/EventLoopApi.hpp>

#include <HXLibs/coroutine/awaiter/WhenAny.hpp>

namespace HX::coroutine {

#if defined(__linux__)

namespace internal {

struct IoUring;

} // namespace internal

struct AioTask {
    AioTask(::io_uring_sqe* sqe) noexcept
        : _sqe{sqe}
    {
        ::io_uring_sqe_set_data(_sqe, this);
    }

#if 0 // 注意: 不能存在`移动`, 否则 IoUring::makeAioTask 返回就是 构造的新对象; 屏蔽了移动, 反而是编译器优化!
    AioTask& operator=(AioTask const&) noexcept = delete;
    AioTask(AioTask const&) noexcept = delete;

    AioTask(AioTask&&) noexcept = default;
    AioTask& operator=(AioTask&&) noexcept = default;
#else
    AioTask& operator=(AioTask&&) noexcept = delete;
#endif

    struct AioAwaiter {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            _task->_previous = coroutine;
            _task->_res = -ENOSYS;
        }
        constexpr int await_resume() const noexcept {
            return _task->_res;
        }
        AioTask* _task;
    };

    AioAwaiter operator co_await() {
        return {this};
    }

private:
    friend internal::IoUring;

    union {
        int _res;
        ::io_uring_sqe* _sqe;
    };
    std::coroutine_handle<> _previous;

public:
    /**
     * @brief 异步打开文件
     * @param dirfd 目录文件描述符, 它表示相对路径的基目录; `AT_FDCWD`, 则表示相对于当前工作目录
     * @param path 文件路径
     * @param flags 指定文件打开的方式, 比如 `O_RDONLY`
     * @param mode 文件权限模式, 仅在文件创建时有效 (一般写`0644`)
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepOpenat(
        int dirfd, 
        char const *path, 
        int flags,
        mode_t mode
    ) && {
        ::io_uring_prep_openat(_sqe, dirfd, path, flags, mode);
        return std::move(*this);
    }

    /**
     * @brief 异步创建一个套接字
     * @param domain 指定 socket 的协议族 (AF_INET(ipv4)/AF_INET6(ipv6)/AF_UNIX/AF_LOCAL(本地))
     * @param type 套接字类型 SOCK_STREAM(tcp)/SOCK_DGRAM(udp)/SOCK_RAW(原始)
     * @param protocol 使用的协议, 通常为 0 (默认协议), 或者指定具体协议(如 IPPROTO_TCP、IPPROTO_UDP 等)
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepSocket(
        int domain, 
        int type, 
        int protocol,
        unsigned int flags
    ) && {
        ::io_uring_prep_socket(_sqe, domain, type, protocol, flags);
        return std::move(*this);
    }

    /**
     * @brief 异步建立连接
     * @param fd 服务端套接字
     * @param addr [out] 客户端信息
     * @param addrlen [out] 客户端信息长度指针
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepAccept(
        int fd, 
        struct ::sockaddr *addr, 
        ::socklen_t *addrlen,
        int flags
    ) && {
        ::io_uring_prep_accept(_sqe, fd, addr, addrlen, flags);
        return std::move(*this);
    }

    /**
     * @brief 异步的向服务端创建连接
     * @param fd 客户端套接字
     * @param addr [out] 服务端信息
     * @param addrlen 服务端信息长度指针
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepConnect(
        int fd, 
        const struct sockaddr *addr,
        socklen_t addrlen
    ) && {
        ::io_uring_prep_connect(_sqe, fd, addr, addrlen);
        return std::move(*this);
    }

    /**
     * @brief 异步读取文件
     * @param fd 文件描述符
     * @param buf [out] 读取到的数据
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRead(
        int fd,
        std::span<char> buf,
        std::uint64_t offset
    ) && {
        ::io_uring_prep_read(_sqe, fd, buf.data(), static_cast<unsigned int>(buf.size()), offset);
        return std::move(*this);
    }

    /**
     * @brief 异步读取文件
     * @param fd 文件描述符
     * @param buf [out] 读取到的数据
     * @param size 读取的长度
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRead(
        int fd,
        std::span<char> buf,
        unsigned int size,
        std::uint64_t offset
    ) && {
        ::io_uring_prep_read(_sqe, fd, buf.data(), size, offset);
        return std::move(*this);
    }

    /**
     * @brief 异步写入文件
     * @param fd 文件描述符
     * @param buf [in] 写入的数据
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepWrite(
        int fd, 
        std::span<char const> buf,
        std::uint64_t offset
    ) && {
        ::io_uring_prep_write(_sqe, fd, buf.data(), static_cast<unsigned int>(buf.size()), offset);
        return std::move(*this);
    }

    /**
     * @brief 异步读取网络套接字文件
     * @param fd 文件描述符
     * @param buf [out] 读取到的数据
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRecv(
        int fd,
        std::span<char> buf,
        int flags
    ) && {
        ::io_uring_prep_recv(_sqe, fd, buf.data(), buf.size(), flags);
        // printf("recv %p\n", (void *)this);
        return std::move(*this);
    }

    /**
     * @brief 异步写入网络套接字文件
     * @param fd 文件描述符
     * @param buf [in] 写入的数据
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepSend(
        int fd, 
        std::span<char const> buf, 
        int flags
    ) && {
        ::io_uring_prep_send(_sqe, fd, buf.data(), buf.size(), flags);
        return std::move(*this);
    }

    /**
     * @brief 异步关闭文件
     * @param fd 文件描述符
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepClose(int fd) && {
        ::io_uring_prep_close(_sqe, fd);
        return std::move(*this);
    }

    /**
     * @brief 监测一个fd的pool事件
     * @param fd 需要监测的fd
     * @param pollMask 需要监测的poll事件 (如:`POLLIN`)
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepPollAdd(
        int fd, 
        unsigned int pollMask
    ) && {
        ::io_uring_prep_poll_add(_sqe, fd, pollMask);
        return std::move(*this);
    }

    /**
     * @brief 创建未链接的超时操作
     * @param ts 超时时间
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepLinkTimeout(
        struct __kernel_timespec* ts,
        unsigned int flags
    ) && {
        ::io_uring_prep_link_timeout(_sqe, ts, flags);
        return std::move(*this);
    }

    /**
     * @brief 链接超时操作
     * @param task 任务
     * @param timeoutTask `prepLinkTimeout`的返回值
     * @return auto 
     */
    [[nodiscard]] inline static auto linkTimeout(
        AioTask&& task, AioTask&& timeoutTask
    ) {
        task._sqe->flags |= IOSQE_IO_LINK;
        return whenAny(std::move(task), std::move(timeoutTask));
    }

    ~AioTask() noexcept {}
};

#elif defined(_WIN32)

#else
    #error "Does not support the current operating system."
#endif

} // namespace HX::coroutine

#endif // !_HX_AIO_TASK_H_