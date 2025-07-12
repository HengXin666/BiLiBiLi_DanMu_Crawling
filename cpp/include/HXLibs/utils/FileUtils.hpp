#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-31 14:36:36
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
#ifndef _HX_FILE_UTILS_H_
#define _HX_FILE_UTILS_H_

#include <fstream>
#include <string>
#include <string_view>
#include <filesystem>
#include <span>

#include <HXLibs/platform/LocalFdApi.hpp>

#include <HXLibs/coroutine/task/Task.hpp>
#include <HXLibs/coroutine/loop/EventLoop.hpp>

/**
 * @brief @todo !!!本类需要大重构!!!
 */

namespace HX::utils {

using platform::LocalFdType;
using platform::kInvalidLocalFd;

/**
 * @brief 文件操作类
 */
struct FileUtils {
private:
#if IO_URING_DIRECT
    // O_DIRECT 可以用来减少操作系统内存复制的开销 (但需要注意可能的对齐要求)
    static constexpr int kOpenModeDefaultFlags = O_LARGEFILE | O_CLOEXEC | O_DIRECT;
#else
    // O_LARGEFILE 允许文件大小超过 2GB
    // O_CLOEXEC 确保在执行新程序时, 文件描述符不会继承到子进程
    static constexpr int kOpenModeDefaultFlags = O_LARGEFILE | O_CLOEXEC;
#endif
public:
    /// @brief 读取文件buf数组的缓冲区大小
    inline static constexpr std::size_t kBufMaxSize = 1 << 12;

    enum class OpenMode : int {
        Read = O_RDONLY | kOpenModeDefaultFlags,                        // 只读模式 (r)
        Write = O_WRONLY | O_TRUNC | O_CREAT | kOpenModeDefaultFlags,   // 只写模式 (w)
        ReadWrite = O_RDWR | O_CREAT | kOpenModeDefaultFlags,           // 读写模式 (a+)
        Append = O_WRONLY | O_APPEND | O_CREAT | kOpenModeDefaultFlags, // 追加模式 (w+)
        Directory = O_RDONLY | O_DIRECTORY | kOpenModeDefaultFlags,     // 目录
    };

    /**
     * @brief 获取文件拓展名 (如`loli.png`->`.png`)
     * @param name 文件名
     * @return std::string_view 
     */
    inline static std::string_view getExtension(std::string_view name) {
        size_t pos = name.rfind('.');
        if (pos == std::string_view::npos) {
            return {};
        }
        return name.substr(pos);
    }

    /**
     * @brief 获取文件的大小
     * @param filePath 文件路径
     * @return std::uintmax_t 大小 (单位: 字节)
     */
    inline static std::uintmax_t getFileSize(std::string_view filePath) {
        return std::filesystem::file_size(
            std::filesystem::path{filePath}
        );
    }

    /**
     * @brief [同步的]读取文件内容
     * @param path 文件路径
     * @return std::string 文件内容
     */
    static std::string getFileContent(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) [[unlikely]] {
            throw std::system_error{errno, std::generic_category()};
        }
        return std::string{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };
    }

    /**
     * @brief [同步的]向文件写入数据
     * @param path 文件路径
     * @param content 需要写入的数据
     */
    static void putFileContent(const std::string& path, std::string_view content) {
        std::ofstream file(path);
        if (!file.is_open()) [[unlikely]] {
            throw std::system_error{errno, std::generic_category()};
        }
        std::copy(
            content.begin(),
            content.end(),
            std::ostreambuf_iterator<char>(file)
        );
    }

    /**
     * @brief [异步的]读取文件内容
     * @param path 文件路径
     * @param flags 打开方式: OpenMode (枚举 如: OpenMode::Read | OpenMode::Append)
     * @param mode 文件权限模式, 仅在文件创建时有效 (一般写0644)
     * @return std::string 文件内容
     */
    static HX::coroutine::Task<std::string> asyncGetFileContent(
        std::string_view path,
        OpenMode flags = OpenMode::Read,
        mode_t mode = 0644
    ) {
        (void)path;
        (void)flags;
        (void)mode;
#ifdef __HX_TODO__
        int fd = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepOpenat(
                AT_FDCWD, path.c_str(), static_cast<int>(flags), mode
            )
        );
        std::string res;
        std::vector<char> buf;
        buf.reserve(kBufMaxSize);
        std::size_t len = 0;
        uint64_t offset = 0;
        while ((len = static_cast<std::size_t>(
            co_await HX::STL::coroutine::loop::IoUringTask().prepRead(fd, buf, offset)
        )) == buf.size()) {
            res += std::string_view {buf.data(), buf.size()};
            offset += buf.size();
        }
        co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
        co_return (res.append(std::string_view {buf.data(), len}));
#else
        co_return {};
#endif // __HX_TODO__
    }

    /**
     * @brief [异步的]向文件写入内容
     * @param path 文件路径
     * @param content 需要写入的数据
     * @param flags 打开方式: OpenMode (枚举 如: OpenMode::Write | OpenMode::Append)
     * @param mode 文件权限模式, 仅在文件创建时有效 (一般写0644)
     * @return std::string 文件内容
     */
    static HX::coroutine::Task<int> asyncPutFileContent(
        const std::string& path,
        std::string_view content,
        OpenMode flags,
        mode_t mode = 0644
    ) {
        (void)path;
        (void)content;
        (void)flags;
        (void)mode;
#ifdef __HX_TODO__
        int fd = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepOpenat(
                AT_FDCWD, path.c_str(), static_cast<int>(flags), mode
            )
        );
        // 无需设置 offset, 因为内核会根据 open 的 flags 来
        int res = co_await HX::STL::coroutine::loop::IoUringTask().prepWrite(
            fd, content, static_cast<std::uint64_t>(-1)
        );
        co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
        co_return res;
#else
        co_return {};
#endif // __HX_TODO__
    }

    /**
     * @brief 设置套接字为非阻塞
     * @param fd 
     * @return int `fcntl` 的返回值
     */
    static int setNonBlock(int fd) {
        int flags = ::fcntl(fd, F_GETFL, 0);
        if (flags < 0) [[unlikely]] {
            return errno;
        }
        return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
};

/**
 * @brief 异步文件类
 */
class AsyncFile {
public:
    AsyncFile(coroutine::EventLoop& eventLoop)
        : _eventLoop{eventLoop}
        , _offset{}
        , _fd{kInvalidLocalFd}
    {}

    /**
     * @brief 异步打开文件
     * @param path 文件路径
     * @param flags 打开方式: OpenMode (枚举 如: OpenMode::Write | OpenMode::Append)
     * @param mode 文件权限模式, 仅在文件创建时有效 (一般写0644)
     * @return HX::STL::coroutine::task::Task<> 
     */
    coroutine::Task<> open(
        std::string_view path,
        FileUtils::OpenMode flags = FileUtils::OpenMode::ReadWrite,
        mode_t mode = 0644
    ) {
        _fd = exception::IoUringErrorHandlingTools::check(
            co_await _eventLoop.makeAioTask().prepOpenat(
                AT_FDCWD, path.data(), static_cast<int>(flags), mode
            )
        );
    }

    /**
     * @brief 读取文件内容到 buf
     * @param buf [out] 读取到的数据
     * @return int 读取的字节数
     */
    coroutine::Task<int> read(std::span<char> buf) {
        int len = exception::IoUringErrorHandlingTools::check(
            co_await _eventLoop.makeAioTask().prepRead(
                _fd, buf, _offset
            )
        );
        _offset += static_cast<uint64_t>(len);
        co_return len;
    }

    /**
     * @brief 读取文件内容到 buf
     * @param buf [out] 读取到的数据
     * @param size 读取的长度
     * @return int 读取的字节数
     */
    coroutine::Task<int> read(std::span<char> buf, unsigned int size) {
        int len = exception::IoUringErrorHandlingTools::check(
            co_await _eventLoop.makeAioTask().prepRead(
                _fd, buf, size, _offset
            )
        );
        _offset += static_cast<uint64_t>(len);
        co_return len;
    }

    /**
     * @brief 将 buf 写入到文件中
     * @param buf [in] 需要写入的数据
     * @return int 写入的字节数
     */
    coroutine::Task<int> write(std::span<char> buf) {
        co_return exception::IoUringErrorHandlingTools::check(
            co_await _eventLoop.makeAioTask().prepWrite(
                _fd, buf, static_cast<std::uint64_t>(-1)
            )
        );
    }

    /**
     * @brief 关闭文件
     * @return coroutine::Task<> 
     */
    coroutine::Task<> close() {
        exception::IoUringErrorHandlingTools::check(
            co_await _eventLoop.makeAioTask().prepClose(_fd)
        );
        _fd = kInvalidLocalFd;
    }

    /**
     * @brief 设置偏移量
     * @param offset 
     */
    void setOffset(uint64_t offset) {
        _offset = offset;
    }

#ifdef NDEBUG
    ~AsyncFile() noexcept = default;
#else
    ~AsyncFile() noexcept(false) {
        if (_fd != kInvalidLocalFd) [[unlikely]] {
            throw std::runtime_error{"Before that, it is necessary to call close()"};
        }
    }
#endif // !NDEBUG

    AsyncFile& operator=(AsyncFile&&) noexcept = delete;

private:
    coroutine::EventLoop& _eventLoop;
    uint64_t _offset;
    LocalFdType _fd;
};

/**
 * @brief OpenMode 位运算符重载 (目前有意义的只有 Read | Append (r+))
 * @warning 几乎用不到这个! 如果使用了请看看`FileUtils::OpenMode`是什么东西先!
 */
constexpr FileUtils::OpenMode operator|(FileUtils::OpenMode lhs, FileUtils::OpenMode rhs) {
    return static_cast<FileUtils::OpenMode>(
        static_cast<int>(lhs) |
        static_cast<int>(rhs)
        // static_cast<std::underlying_type<FileUtils::OpenMode>::type>(lhs) |
        // static_cast<std::underlying_type<FileUtils::OpenMode>::type>(rhs)
    );
}

} // namespace HX::utils

#endif // _HX_FILE_UTILS_H_