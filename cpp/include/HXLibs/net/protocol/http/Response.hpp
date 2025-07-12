#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-20 23:18:48
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
#ifndef _HX_RESPONSE_H_
#define _HX_RESPONSE_H_

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <optional>

#include <HXLibs/net/protocol/http/Http.hpp>
#include <HXLibs/net/protocol/http/Status.hpp>
#include <HXLibs/net/protocol/http/MimeType.hpp>
#include <HXLibs/net/socket/IO.hpp>
#include <HXLibs/coroutine/task/Task.hpp>
#include <HXLibs/container/ArrayBuf.hpp>
#include <HXLibs/utils/StringUtils.hpp>
#include <HXLibs/utils/FileUtils.hpp>
#include <HXLibs/utils/TimeNTTP.hpp>

namespace HX::net {

/**
 * @brief 响应数据
 */
struct ResponseData {
    int status = 0;        // 状态码
    HeaderHashMap headers; // 响应头
    std::string body;      // 响应体
};

/**
 * @brief 响应类(Response)
 */
class Response {
public:
    explicit Response(IO& io)
        : _recvBuf()
        , _statusLine()
        , _responseHeaders()
        , _body()
        , _responseHeadersIt(_responseHeaders.end())
        , _sendBuf()
        , _io{io}
    {
        // @todo 如果在乎客户端的性能, 就封装为模板, 然后提供 bool, 然后 constexpr if 解决
        _sendBuf.reserve(IO::kBufMaxSize);
    }

#if 0
    Response(const Response&) = delete;
    Response& operator=(const Response&) = delete;

    Response(Response&&) = default;
    Response& operator=(Response&& that) { // 神人代码
        delete this;
        new (this) Response(std::move(that));
        return *this;
    }
#else
    Response& operator=(Response&&) noexcept = delete;
#endif
    // ===== ↓客户端使用↓ =====
    /**
     * @brief 解析服务端响应
     * @tparam Seconds 超时时间, 单位: 秒 (s)
     * @return coroutine::Task<bool> 是否解析完毕 
     */
    template <typename Timeout = decltype(utils::operator""_s<'3', '0'>())>
        requires(requires { Timeout::Val; })
    coroutine::Task<bool> parserRes() {
        for (std::size_t n = IO::kBufMaxSize; n; n = _parserRes()) {
            auto res = co_await _io.recvLinkTimeout<Timeout>(
                // 保留原有的数据
                {_recvBuf.data() + _recvBuf.size(),  _recvBuf.data() + _recvBuf.max_size()}
            );
            if (res.index() == 1) [[unlikely]] {
                co_return false;  // 超时
            }
            auto recvN = exception::IoUringErrorHandlingTools::check(
                res.template get<0, exception::ExceptionMode::Nothrow>()
            );
            if (recvN == 0) [[unlikely]] {
                co_return false; // 连接断开
            }
            _recvBuf.addSize(static_cast<std::size_t>(recvN));
        }
        co_return true;
    }

    /**
     * @brief 获取协议版本
     * @return std::string 
     */
    std::string getProtocolVersion() const {
        return _statusLine[ResponseLineDataType::ProtocolVersion];
    }

    /**
     * @brief 获取状态码
     * @return std::string 
     */
    std::string getStatusCode() const {
        return _statusLine[ResponseLineDataType::StatusCode];
    }

    /**
     * @brief 获取状态信息
     * @return std::string 
     */
    std::string getStatusMessage() const {
        return _statusLine[ResponseLineDataType::StatusMessage];
    }

    /**
     * @brief 获取响应头键值对 只读引用
     * @return 响应头键值对 (键均为小写)
     */
    auto const& getHeaders() const {
        return _responseHeaders;
    }

    /**
     * @brief 获取响应头键值对 引用
     * @return 响应头键值对 (键均为小写)
     */
    auto& getHeaders() {
        return _responseHeaders;
    }

    /**
     * @brief 获取响应体
     * @return std::string 
     */
    std::string getBody() const {
        return _body;
    }

    /**
     * @brief 生成响应数据
     * @return ResponseData 
     */
    ResponseData makeResponseData() {
        return {
            std::stoi(_statusLine[StatusCode]),
            std::move(_responseHeaders),
            std::move(_body)
        };
    }

    // ===== ↑客户端使用↑ =====

    // ===== ↓服务端使用の更加人性化API↓ =====

    /**
     * @brief 设置响应码和正文(html)
     * @param status 
     * @param content
     * @return Response& 可链式调用
     */
    Response& setStatusAndContent(Status status, std::string const& content) {
        setResLine(status).setContentType(TEXT).setBodyData(content);
        return *this;
    }

    /**
     * @brief 发送已经设置的响应
     * @return coroutine::Task<> 
     */
    coroutine::Task<> sendRes() {
        createResponseBuffer();
        co_await _io.send(_sendBuf);
    }

    /**
     * @brief 使用分块编码传输文件
     * @param filePath 文件路径
     */
    coroutine::Task<> useChunkedEncodingTransferFile(std::string_view filePath) {
        using namespace std::string_literals;
        auto fileType = getMimeType(
            utils::FileUtils::getExtension(filePath)
        );
        setResLine(Status::CODE_200);
        addHeader("Content-Type"s, fileType);
        addHeader("Transfer-Encoding"s, "chunked"s);
        // 生成响应行和响应头
        _buildResponseLineAndHeaders();
        // 先发送一版, 告知我们是分块编码
        co_await _io.send(_sendBuf);
        
        utils::AsyncFile file{_io};
        co_await file.open(filePath);
        try {
            std::vector<char> buf(utils::FileUtils::kBufMaxSize);
            while (true) {
                // 读取文件
                std::size_t size = static_cast<std::size_t>(co_await file.read(buf));
                _buildToChunkedEncoding({buf.data(), size});
                co_await _io.send(_sendBuf);
                if (size != buf.size()) {
                    // 需要使用 长度为 0 的分块, 来标记当前内容实体传输结束
                    _buildToChunkedEncoding("");
                    co_await _io.send(_sendBuf);
                    break;
                }
            }
        } catch (std::exception const& e) {
            // _io.send 会抛异常
        }
        co_await file.close();
    }

    /**
     * @brief 使用断点续传传输文件
     * @note 内部会智能判断客户端是否需要使用断点续传, 最坏也只是降级为普通传输 (都是分块读和发的)
     * @param rrv 断点续传参数包, 通过 `req.getRangeRequestView()` 获取
     * @param filePath 文件路径
     */
    coroutine::Task<> useRangeTransferFile(RangeRequestView rrv, std::string_view filePath) {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        // 解析请求的范围
        auto& type = rrv.reqType;
        auto fileType = getMimeType(
            utils::FileUtils::getExtension(filePath)
        );
        auto fileSize = utils::FileUtils::getFileSize(filePath);
        auto fileSizeStr = std::to_string(fileSize);
        auto const& headMap = rrv.reqHead;
        if (type == "HEAD"sv) {
            // 返回文件大小
            /*
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 10737418240\r\n"
                "Accept-Ranges: bytes\r\n"
                "\r\n"
            */
            setResLine(Status::CODE_200);
            addHeader("Content-Length"s, fileSizeStr);
            addHeader("Content-Type"s, fileType);
            addHeader("Accept-Ranges"s, "bytes"s);
            _buildResponseLineAndHeaders();
            co_await _io.send(_sendBuf);
        } else if (auto it = headMap.find("range"); it != headMap.end()) {
            // 开始[断点续传]传输, 先发一下头
            /*
                "HTTP/1.1 206 Partial Content\r\n"
                "Content-Range: bytes X-Y/Z\r\n"
                "Content-Length: Y-X+1\r\n"
                "Accept-Ranges: bytes\r\n"
                "\r\n"

                其中Content-Range为: 本次传输的文件的起始位置-结束位置/总大小
                而Content-Length是 [X, Y] 包含X, 包含Y, 所以是 Y - X + 1个字节
            */
            // 解析范围, 如果访问不合法, 应该返回416
            std::string_view rangeVals = it->second;
            // Range: bytes=<range-start>-<range-end>
            auto rangeNumArr = utils::StringUtil::split<std::string>(rangeVals.substr(6), ","sv);
            setResLine(Status::CODE_206);
            addHeader("Accept-Ranges"s, "bytes"s);

            if (rangeNumArr.size() == 1) [[likely]] { // 一般都是请求单个范围
                auto [begin, end] = utils::StringUtil::splitAtFirst(rangeNumArr.back(), "-"sv);
                if (begin.empty()) {
                    begin += '0';
                }
                if (end.empty()) {
                    end = std::to_string(fileSize - 1);
                }
                uint64_t beginPos = std::stoull(begin);
                uint64_t endPos = std::stoull(end);
                if (beginPos > endPos || endPos > fileSize) [[unlikely]] {
                    // 范围不合法: 返回416, 表示请求错误
                    setResLine(Status::CODE_416);
                    _buildResponseLineAndHeaders();
                    co_await _io.send(_sendBuf);
                } else {
                    uint64_t remaining = endPos - beginPos + 1;
                    addHeader("Content-Range", "bytes " + begin + "-" + end + "/" + fileSizeStr);
                    addHeader("Content-Type", fileType);
                    addHeader("Content-Length", std::to_string(remaining));
                    _buildResponseLineAndHeaders();
                    co_await _io.send(_sendBuf); // 先发一个头
                    
                    utils::AsyncFile file{_io};
                    co_await file.open(filePath);
                    try {
                        file.setOffset(beginPos);
                        std::vector<char> buf(std::min(fileSize, utils::FileUtils::kBufMaxSize));
                        // 支持偏移量
                        while (remaining > 0) {
                            // 读取文件
                            std::size_t size = static_cast<std::size_t>(
                                co_await file.read(
                                    buf,
                                    static_cast<uint32_t>(std::min(remaining, buf.size()))
                                )
                            );
                            if (!size) [[unlikely]] {
                                break;
                            }
                            co_await _io.send(buf);
                            remaining -= size;
                        }
                    } catch (std::exception const& e) {
                        // _io.send 会抛异常
                    }
                    co_await file.close();
                }
            } else {
                /*
                    HTTP/1.1 206 Partial Content\r\n
                    Content-Type: multipart/byteranges; boundary=BOUNDARY_STRING\r\n
                    \r\n
                    --BOUNDARY_STRING\r\n
                    Content-Range: bytes X1-Y1/Z\r\n
                    Content-Length: L1\r\n
                    Content-Type: application/octet-stream\r\n
                    \r\n
                    [BINARY DATA PART 1]\r\n
                    --BOUNDARY_STRING\r\n
                    Content-Range: bytes X2-Y2/Z\r\n
                    Content-Length: L2\r\n
                    Content-Type: application/octet-stream\r\n
                    \r\n
                    [BINARY DATA PART 2]\r\n
                    --BOUNDARY_STRING--\r\n
                */
                addHeader("Content-Type", "multipart/byteranges; boundary=BOUNDARY_STRING");
                _buildResponseLineAndHeaders();
                co_await _io.send(_sendBuf); // 先发一个头
                for (auto& ragen : rangeNumArr) {
                    auto [begin, end] = utils::StringUtil::splitAtFirst(ragen, "-");
                    if (begin.empty()) {
                        begin += '0';
                    }
                    if (end.empty()) {
                        end = std::to_string(fileSize - 1);
                    }
                    uint64_t beginPos = std::stoull(begin);
                    uint64_t endPos = std::stoull(end);
                    // 范围不合法: 不会报错, 而是忽略!
                    if (beginPos > endPos || endPos > fileSize) [[unlikely]] {
                        continue;
                    }
                    _sendBuf.clear();

                    uint64_t remaining = endPos - beginPos + 1;
                    utils::StringUtil::append(_sendBuf, "--BOUNDARY_STRING\r\n"sv);
                    utils::StringUtil::append(_sendBuf, "Content-Range: bytes "sv);
                    utils::StringUtil::append(_sendBuf, begin);
                    utils::StringUtil::append(_sendBuf, "-"sv);
                    utils::StringUtil::append(_sendBuf, end);
                    utils::StringUtil::append(_sendBuf, "/"sv);
                    utils::StringUtil::append(_sendBuf, fileSizeStr);
                    utils::StringUtil::append(_sendBuf, CRLF);
                    utils::StringUtil::append(_sendBuf, "Content-Length: "sv);
                    utils::StringUtil::append(_sendBuf, std::to_string(remaining));
                    utils::StringUtil::append(_sendBuf, CRLF);
                    utils::StringUtil::append(_sendBuf, "Content-Type: application/octet-stream\r\n"sv);
                    utils::StringUtil::append(_sendBuf, CRLF);
                    co_await _io.send(_sendBuf); // 先发头

                    utils::AsyncFile file{_io};
                    co_await file.open(filePath);
                    try {
                        file.setOffset(beginPos);
                        std::vector<char> buf(utils::FileUtils::kBufMaxSize);
                        // 支持偏移量
                        while (remaining > 0) {
                            // 读取文件
                            std::size_t size = static_cast<std::size_t>(
                                co_await file.read(
                                    buf,
                                    static_cast<uint32_t>(std::min(remaining, buf.size()))
                                )
                            );
                            if (!size) [[unlikely]] {
                                break;
                            }
                            co_await _io.send(buf);
                            co_await _io.send(CRLF);
                            remaining -= size;
                        }
                    } catch (std::exception const& e) {
                        // _io.send 会抛异常
                    }
                    co_await file.close();
                }
                co_await _io.send("--BOUNDARY_STRING--\r\n"sv);
            }
        } else {
            // 普通的传输文件
            setResLine(Status::CODE_200);
            addHeader("Content-Type"s, fileType);
            addHeader("Content-Length"s, fileSizeStr);
            _buildResponseLineAndHeaders();
            co_await _io.send(_sendBuf); // 先发一个头

            utils::AsyncFile file{_io};
            co_await file.open(filePath);
            try {
                std::vector<char> buf(std::min(fileSize, utils::FileUtils::kBufMaxSize));
                uint64_t remaining = fileSize;
                while (remaining > 0) {
                    std::size_t size = static_cast<std::size_t>(
                        co_await file.read(buf)
                    );
                    if (!size) [[unlikely]] {
                        break;
                    }
                    co_await _io.send(buf);
                    remaining -= size;
                }
            } catch (std::exception const& e) {
                // _io.send 会抛异常
            }
            co_await file.close();
        }
    }

    // ===== ↑服务端使用の更加人性化API↑ =====

    // ===== ↓服务端使用↓ =====
    /**
     * @brief 设置状态行 (协议使用HTTP/1.1)
     * @param statusCode 状态码
     * @param describe 状态码描述: 如果为`""`则会使用该状态码对应默认的描述
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    Response& setResLine(Status statusCode, std::string_view describe = "") {
        using namespace std::string_view_literals;
        _statusLine.clear();
        _statusLine.resize(3);
        _statusLine[ResponseLineDataType::ProtocolVersion] = "HTTP/1.1"sv;
        _statusLine[ResponseLineDataType::StatusCode] = std::to_string(static_cast<int>(statusCode));
        if (!describe.size()) {
            _statusLine[ResponseLineDataType::StatusMessage] = getStatusCodeDataStrView(statusCode);
        } else {
            _statusLine[ResponseLineDataType::StatusMessage] = describe;
        }
        return *this;
    }

    /**
     * @brief 设置响应头部: 设置响应类型, 如果响应体是文本, 你需要指定字符编码(不指定则留空`""`)
     * @param type 响应类型, 如`text/html`
     * @param encoded 字符编码, 如`UTF-8` ~~(如果是图片等就可以不用指定)~~
     * @return [this&] 可以链式调用
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    Response& setContentType(HttpContentType type) {
        using namespace std::string_literals;
        _responseHeaders["Content-Type"s] = getContentTypeStrView(type);
        return *this;
    }

    /**
     * @brief 设置响应体
     * @param data 响应体数据
     * @return [this&] 可以链式调用
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    Response& setBodyData(const std::string& data) {
        _body = data;
        return *this;
    }

    /**
     * @brief 向响应头部添加一个键值对
     * @param key 键
     * @param val 值
     * @return Response&
     * @warning `key`在`map`中是区分大小写的, 故不要使用`大小写不同`的相同的`键`
     */
    template <typename Str>
    Response& addHeader(const std::string& key, Str&& val) {
        _responseHeaders[key] = std::forward<Str>(val);
        return *this;
    }
    // ===== ↑服务端使用↑ =====

    /**
     * @brief 清空的响应, 重置状态
     */
    void clear() noexcept {
        _statusLine.clear();
        _responseHeaders.clear();
        _body.clear();
        _responseHeadersIt = _responseHeaders.end();
        _sendBuf.clear();
        _completeResponseHeader = false;
    }
private:
    /**
     * @brief 响应行数据分类
     */
    enum ResponseLineDataType {
        ProtocolVersion = 0,   // 协议版本
        StatusCode = 1,        // 状态码
        StatusMessage = 2,     // 状态信息
    };
    
    /**
     * @brief 仅用于读取时候写入的缓冲区
     */
    container::ArrayBuf<char, IO::kBufMaxSize> _recvBuf;

    // 注意: 他们的末尾并没有事先包含 \r\n, 具体在to_string才提供
    std::vector<std::string> _statusLine; // 状态行
    HeaderHashMap _responseHeaders;       // 响应头
    std::string _body;                    // 响应体

    // 指向上一次解析的响应头的键值对; 无效时候指向 `.end()`
    decltype(_responseHeaders)::iterator _responseHeadersIt; 

    std::vector<char> _sendBuf;                     // 用于发送数据的缓冲区
    std::optional<std::size_t> _remainingBodyLen;   // 仍需读取的请求体长度
    IO& _io;
    bool _completeResponseHeader = false;           //是否解析完成响应头

    /**
     * @brief [仅服务端] 生成响应行和响应头
     */
    template <bool IsEnd = true>
    void _buildResponseLineAndHeaders() {
#ifndef NDEBUG // Debug
        if (_sendBuf.size()) [[unlikely]] {
            // 不应该有此异常
            throw std::runtime_error{"There shouldn't be this exception"};
        }
#endif
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        _responseHeaders["Connection"s] = "keep-alive"sv; // 长连接
        _responseHeaders["Server"s] = "HXLibs::net"sv;
        
        utils::StringUtil::append(_sendBuf, _statusLine[ResponseLineDataType::ProtocolVersion]);
        utils::StringUtil::append(_sendBuf, " "sv);
        utils::StringUtil::append(_sendBuf, _statusLine[ResponseLineDataType::StatusCode]);
        utils::StringUtil::append(_sendBuf, " "sv);
        utils::StringUtil::append(_sendBuf, _statusLine[ResponseLineDataType::StatusMessage]);
        utils::StringUtil::append(_sendBuf, CRLF);
        for (const auto& [key, val] : _responseHeaders) {
            utils::StringUtil::append(_sendBuf, key);
            utils::StringUtil::append(_sendBuf, HEADER_SEPARATOR_SV);
            utils::StringUtil::append(_sendBuf, val);
            utils::StringUtil::append(_sendBuf, CRLF);
        }
        if constexpr (IsEnd) {
            utils::StringUtil::append(_sendBuf, CRLF);
        }
    }

    /**
     * @brief [仅服务端] 将`buf`转化为`ChunkedEncoding`的Body, 放入`_body`以分片发送
     * @param buf 
     * @warning 内部会清空 `_sendBuf`, 再以`ChunkedEncoding`格式写入 buf 到 `_sendBuf`!
     */
    void _buildToChunkedEncoding(std::string_view buf) {
        _sendBuf.clear();
#ifdef HEXADECIMAL_CONVERSION
        utils::StringUtil::append(_sendBuf, utils::NumericBaseConverter::hexadecimalConversion(buf.size()));
#else
        utils::StringUtil::append(_sendBuf, std::format("{:X}", buf.size())); // 需要十六进制嘞
#endif // !HEXADECIMAL_CONVERSION
        utils::StringUtil::append(_sendBuf, CRLF);
        utils::StringUtil::append(_sendBuf, buf);
        utils::StringUtil::append(_sendBuf, CRLF);
    }

    /**
     * @brief [仅服务端] 生成完整的响应字符串, 用于写入
     * @warning 本方法子适用于`Content-Length`的短消息, 无法使用分块编码
     */
    void createResponseBuffer() {
        using namespace std::string_view_literals;
        _sendBuf.clear();
        _buildResponseLineAndHeaders<false>();
        // 补充这个
        utils::StringUtil::append(_sendBuf, "Content-Length: "sv);
        utils::StringUtil::append(_sendBuf, std::to_string(_body.size()));
        utils::StringUtil::append(_sendBuf, CRLF);

        utils::StringUtil::append(_sendBuf, CRLF);
        utils::StringUtil::append(_sendBuf, std::move(_body));
    }

    /**
     * @brief [[仅客户端]] 解析响应
     * @return 是否需要继续解析;
     *         `== 0`: 不需要;
     *         `>  0`: 需要继续解析`size_t`个字节
     * @warning 假定内容是符合Http协议的
     */
    std::size_t _parserRes() { 
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        std::string_view buf = {_recvBuf.data(), _recvBuf.size()};
        switch ((_completeResponseHeader << 1) | (!_statusLine.empty())) {
            case 0x00: { // 响应行
                if (_statusLine.empty()) { // 响应行还未解析
                    std::size_t pos = buf.find(CRLF);
                    if (pos == std::string_view::npos) [[unlikely]] { // 不可能事件
                        return IO::kBufMaxSize;
                    }

                    // 解析响应行, 注意 不能按照空格直接切分! 因为 HTTP/1.1 404 NOF FOND\r\n
                    _statusLine = utils::StringUtil::split<std::string>(
                        buf.substr(0, pos), " "sv
                    );
                    if (_statusLine.size() < 3) [[unlikely]] {
                        return IO::kBufMaxSize;
                    }
                    if (_statusLine.size() > 3) {
                        for (std::size_t i = 3; i < _statusLine.size(); ++i) {
                            _statusLine[ResponseLineDataType::StatusMessage] += std::move(_statusLine[i]);
                        }
                        _statusLine.resize(3);
                    }
                    buf = buf.substr(pos + 2); // 再前进, 以去掉 "\r\n"
                }
            }
            case 0x01: { // 响应头
                /**
                 * @brief 请求头
                 * 通过`\r\n`分割后, 取最前面的, 先使用最左的`:`以判断是否是需要作为独立的键值对;
                 * -  如果找不到`:`, 并且 非空, 那么它需要接在上一个解析的键值对的值尾
                 * -  否则即请求头解析完毕!
                 */
                while (!_completeResponseHeader) { // 响应头未解析完
                    std::size_t pos = buf.find(CRLF);
                    if (pos == std::string_view::npos) { // 没有读取完
                        _recvBuf.moveToHead(buf);
                        return IO::kBufMaxSize;
                    }
                    std::string_view subKVStr = buf.substr(0, pos);
                    auto p = utils::StringUtil::splitAtFirst(subKVStr, HEADER_SEPARATOR_SV);
                    if (p.first.empty()) {                  // 找不到 ": "
                        if (subKVStr.size()) [[unlikely]] { // 很少会有分片传输响应头的
                            _responseHeadersIt->second.append(subKVStr);
                        } else { // 请求头解析完毕!
                            _completeResponseHeader = true;
                        }
                    } else {
                        // K: V, 其中 V 是区分大小写的, 但是 K 是不区分的
                        utils::StringUtil::toLower(p.first);
                        _responseHeadersIt = _responseHeaders.insert(p).first;
                    }
                    buf = buf.substr(pos + 2); // 再前进, 以去掉 "\r\n"
                }
            }
            case 0x03: { // 响应体
                if (_responseHeaders.contains(CONTENT_LENGTH_SV)) { // 存在content-length模式接收的响应体
                    // 是 空行之后 (\r\n\r\n) 的内容大小(char)
                    if (!_remainingBodyLen.has_value()) {
                        _body = buf;
                        _remainingBodyLen = std::stoull(_responseHeaders.find(CONTENT_LENGTH_SV)->second) 
                                          - _body.size();
                    } else {
                        *_remainingBodyLen -= buf.size();
                        _body.append(buf);
                    }

                    if (*_remainingBodyLen != 0) {
                        _recvBuf.clear();
                        return *_remainingBodyLen;
                    }
                } else if (_responseHeaders.contains(TRANSFER_ENCODING_SV)) { // 存在响应体以`分块传输编码`
                    if (_remainingBodyLen) { // 处理没有读取完的
                        if (buf.size() <= *_remainingBodyLen) { // 还没有读取完毕
                            _body += buf;
                            *_remainingBodyLen -= buf.size();
                            return IO::kBufMaxSize;
                        } else { // 读取完了
                            _body.append(buf, 0, *_remainingBodyLen);
                            buf = buf.substr(std::min(*_remainingBodyLen + 2, buf.size()));
                            _remainingBodyLen.reset();
                        }
                    }
                    while (true) {
                        std::size_t posLen = buf.find(CRLF);
                        if (posLen == std::string_view::npos) { // 没有读完
                            _recvBuf.moveToHead(buf);
                            return IO::kBufMaxSize;
                        }
                        if (!posLen && buf[0] == '\r') [[unlikely]] { // posLen == 0
                            // \r\n 贴脸, 触发原因, std::min(*_remainingBodyLen + 2, buf.size()) 只能 buf.size()
                            buf = buf.substr(posLen + 2);
                            continue;
                        }
                        _remainingBodyLen = std::stol(
                            std::string{buf.substr(0, posLen)}, nullptr, 16
                        ); // 转换为十进制整数
                        if (!*_remainingBodyLen) { // 解析完毕
                            return 0;
                        }
                        buf = buf.substr(posLen + 2);
                        if (buf.size() <= *_remainingBodyLen) { // 没有读完
                            _body += buf;
                            *_remainingBodyLen -= buf.size();
                            return IO::kBufMaxSize;
                        }
                        _body.append(buf.substr(0, *_remainingBodyLen));
                        buf = buf.substr(*_remainingBodyLen + 2);
                    }
                } else if (_responseHeaders.contains("content-range")) {
                    // 断点续传 @todo
                }
                break;
            }
            [[unlikely]] default:
#ifndef NDEBUG
                throw std::runtime_error{"parserResponse UB Error"}
#endif // !NDEBUG
            ;
        }
        return 0; // 解析完毕
#if 0
        // @todo 需要修改!!!
        if (_buf.size()) {
            _buf += buf;
            buf = _buf;
        }

        if (_statusLine.empty()) { // 响应行还未解析
            std::size_t pos = buf.find(CRLF);
            if (pos == std::string_view::npos) [[unlikely]] { // 不可能事件
                return IO::kBufMaxSize;
            }

            // 解析响应行, 注意 不能按照空格直接切分! 因为 HTTP/1.1 404 NOF FOND\r\n
            _statusLine = utils::StringUtil::split<std::string>(buf.substr(0, pos), " "s);
            if (_statusLine.size() < 3)
                return IO::kBufMaxSize;
            if (_statusLine.size() > 3) {
                for (std::size_t i = 4; i < _statusLine.size(); ++i) {
                    _statusLine[ResponseLineDataType::StatusMessage] += std::move(_statusLine[i]);
                }
                _statusLine.resize(3);
            }
            buf = buf.substr(pos + 2); // 再前进, 以去掉 "\r\n"
        }

        /**
        * @brief 请求头
        * 通过`\r\n`分割后, 取最前面的, 先使用最左的`:`以判断是否是需要作为独立的键值对;
        * -  如果找不到`:`, 并且 非空, 那么它需要接在上一个解析的键值对的值尾
        * -  否则即请求头解析完毕!
        */
        while (!_completeResponseHeader) { // 响应头未解析完
            std::size_t pos = buf.find(CRLF);
            if (pos == std::string_view::npos) { // 没有读取完
                _buf = buf;
                return IO::kBufMaxSize;
            }
            std::string_view subStr = buf.substr(0, pos);
            auto p = utils::StringUtil::splitAtFirst(subStr, ": "s);
            if (p.first.empty()) { // 找不到 ": "
                if (subStr.size()) [[unlikely]] { // 很少会有分片传输响应头的
                    _responseHeadersIt->second.append(subStr);
                } else { // 请求头解析完毕!
                    _completeResponseHeader = true;
                }
            } else {
                utils::StringUtil::toLower(p.first);
                _responseHeadersIt = _responseHeaders.insert(p).first;
            }
            buf = buf.substr(pos + 2);
        }

        if (_responseHeaders.count("Content-Length"s)) { // 存在content-length模式接收的响应体
            // 是 空行之后 (\r\n\r\n) 的内容大小(char)
            if (!_remainingBodyLen.has_value()) {
                _body = buf;
                _remainingBodyLen = std::stoull(_responseHeaders["Content-Length"s]) 
                                - _body.size();
            } else {
                *_remainingBodyLen -= buf.size();
                _body.append(buf);
            }

            if (*_remainingBodyLen != 0) {
                _buf.clear();
                return *_remainingBodyLen;
            }
        } else if (_responseHeaders.count("Transfer-Encoding"s)) { // 存在响应体以`分块传输编码`
            if (_remainingBodyLen) { // 处理没有读取完的
                if (buf.size() <= *_remainingBodyLen) { // 还没有读取完毕
                    _body += buf;
                    *_remainingBodyLen -= buf.size();
                    return IO::kBufMaxSize;
                } else { // 读取完了
                    _body.append(buf, 0, *_remainingBodyLen);
                    buf = buf.substr(std::min(*_remainingBodyLen + 2, buf.size()));
                    _remainingBodyLen.reset();
                }
            }
            while (true) {
                std::size_t posLen = buf.find(CRLF);
                if (posLen == std::string_view::npos) { // 没有读完
                    _buf = buf;
                    return IO::kBufMaxSize;
                }
                if (!posLen && buf[0] == '\r') [[unlikely]] { // posLen == 0
                    // \r\n 贴脸, 触发原因, std::min(*_remainingBodyLen + 2, buf.size()) 只能 buf.size()
                    buf = buf.substr(posLen + 2);
                    continue;
                }
                _remainingBodyLen = std::stol(std::string{buf.substr(0, posLen)}, nullptr, 16); // 转换为十进制整数
                if (!*_remainingBodyLen) { // 解析完毕
                    return 0;
                }
                buf = buf.substr(posLen + 2);
                if (buf.size() <= *_remainingBodyLen) { // 没有读完
                    _body += buf;
                    *_remainingBodyLen -= buf.size();
                    return IO::kBufMaxSize;
                }
                _body.append(buf.substr(0, *_remainingBodyLen));
                buf = buf.substr(*_remainingBodyLen + 2);
            }
        }
#endif
    }
};

} // namespace HX::net

#endif // _HX_RESPONSE_H_