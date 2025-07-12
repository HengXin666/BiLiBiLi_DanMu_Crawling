#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-20 17:04:53
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
#ifndef _HX_REQUEST_H_
#define _HX_REQUEST_H_

#include <vector>
#include <optional>
#include <stdexcept>

#include <HXLibs/container/ArrayBuf.hpp>
#include <HXLibs/net/protocol/http/Http.hpp>
#include <HXLibs/net/protocol/http/PathVariable.hpp>
#include <HXLibs/net/socket/IO.hpp>
#include <HXLibs/utils/FileUtils.hpp>
#include <HXLibs/utils/StringUtils.hpp>
#include <HXLibs/utils/ContainerConcepts.hpp>
#include <HXLibs/exception/ErrorHandlingTools.hpp>

#include <HXLibs/log/Log.hpp> // debug

namespace HX::net {

class Router;

/**
 * @brief 请求类(Request)
 */
class Request {
public:
    explicit Request(IO& io) 
        : _recvBuf()
        , _requestLine()
        , _requestHeaders()
        , _requestHeadersIt(_requestHeaders.end())
        , _body()
        , _remainingBodyLen(std::nullopt)
        , _io{io}
    {}

#if 0
    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;

    Request(Request&&) = default;
    Request& operator=(Request&&) = default;
#else
    Request& operator=(Request&&) noexcept = delete;
#endif

    // ===== ↓客户端使用↓ =====
    /**
     * @brief 发送请求, 并且解析
     * @return coroutine::Task<> 
     * @throw 超时
     */
    template <typename Timeout>
        requires(requires { Timeout::Val; })
    coroutine::Task<> sendHttpReq() {
        using namespace std::string_literals;
        // 发送请求行
        std::vector<char> buf;
        buf.reserve(IO::kBufMaxSize); // 预留空间
        utils::StringUtil::append(buf, _requestLine[RequestLineDataType::RequestType]);
        utils::StringUtil::append(buf, " "s);
        utils::StringUtil::append(buf, _requestLine[RequestLineDataType::RequestPath]);
        utils::StringUtil::append(buf, " "s);
        utils::StringUtil::append(buf, _requestLine[RequestLineDataType::ProtocolVersion]);
        utils::StringUtil::append(buf, CRLF);
        // 发送请求头
        for (const auto& [key, val] : _requestHeaders) {
            utils::StringUtil::append(buf, key);
            utils::StringUtil::append(buf, HEADER_SEPARATOR_SV);
            utils::StringUtil::append(buf, val);
            utils::StringUtil::append(buf, CRLF);
        }
        if (_body.empty()) {
            utils::StringUtil::append(buf, CRLF);
            checkTimeout(co_await _io.sendLinkTimeout<Timeout>(buf));
        } else {
            // 发送请求体
            utils::StringUtil::append(buf, CONTENT_LENGTH_SV);
            utils::StringUtil::append(buf, std::to_string(_body.size()));
            utils::StringUtil::append(buf, HEADER_END_SV);
            checkTimeout(co_await _io.sendLinkTimeout<Timeout>(buf));
            checkTimeout(co_await _io.sendLinkTimeout<Timeout>(_body));
        }
        co_return;
    }

    /**
     * @brief 设置请求行 (协议使用HTTP/1.1)
     * @param method 请求方法 (如 "GET")
     * @param url url (如 "www.baidu.com")
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    template <HttpMethod Method>
    Request& setReqLine(std::string_view url) {
        using namespace std::string_literals;
        _requestLine.resize(3);
        _requestLine[RequestLineDataType::RequestType] = getMethodStringView(Method);
        _requestLine[RequestLineDataType::RequestPath] = url;
        _requestLine[RequestLineDataType::ProtocolVersion] = "HTTP/1.1"s;
        return *this;
    }

    /**
     * @brief 向请求头添加一些键值对
     * @param heads 键值对
     * @return Request& 
     */
    Request& addHeaders(const std::vector<std::pair<std::string, std::string>>& heads) {
        _requestHeaders.insert(heads.begin(), heads.end());
        return *this;
    }

    /**
     * @brief 向请求头添加一些键值对
     * @param heads 键值对
     * @return Request& 
     */
    Request& addHeaders(const std::unordered_map<std::string, std::string>& heads) {
        _requestHeaders.insert(heads.begin(), heads.end());
        return *this;
    }

    /**
     * @brief 向请求头添加一些键值对
     * @param heads 键值对
     * @return Request& 
     */
    Request& addHeaders(const HeaderHashMap& heads) {
        _requestHeaders.insert(heads.begin(), heads.end());
        return *this;
    }

    /**
     * @brief 设置请求体信息
     * @param data 信息
     * @return Request& 
     */
    Request& setBody(const std::string& data) {
        _body = data;
        return *this;
    }

    /**
     * @brief 设置请求体信息 [[std::move优化]]
     * @param data 信息
     * @return Request& 
     */
    Request& setBody(std::string&& data) {
        _body = std::move(data);
        return *this;
    }

    /**
     * @brief 设置请求体信息
     * @param data 信息
     * @return Request& 
     */
    Request& setBody(std::string_view data) {
        _body = data;
        return *this;
    }

    /**
     * @brief 向请求头添加一个键值对
     * @param key 键
     * @param val 值
     * @return Request&
     * @warning `key`在`map`中是区分大小写的, 故不要使用`大小写不同`的相同的`键`
     */
    template <utils::StringType Str>
    Request& addHeaders(const std::string& key, Str&& val) {
        _requestHeaders[key] = std::forward<Str>(val);
        return *this;
    }

    /**
     * @brief 向请求头添加一个键值对
     * @param key 键
     * @param val 值
     * @return Request&
     * @warning `key`在`map`中是区分大小写的, 故不要使用`大小写不同`的相同的`键`
     */
    template <typename Char, std::size_t N>
    Request& addHeaders(const std::string& key, const Char (&val)[N]) {
        _requestHeaders[key] = std::string{val, N};
        return *this;
    }

    /**
     * @brief 尝试向请求头添加一个键值对, 如果存在则不插入
     * @param key 键
     * @param val 值
     * @return Request&
     * @warning `key`在`map`中是区分大小写的, 故不要使用`大小写不同`的相同的`键`
     */
    template <utils::StringType Str>
    Request& tryAddHeaders(const std::string& key, Str&& val) {
        _requestHeaders.try_emplace(key, std::forward<Str>(val));
        return *this;
    }
    // ===== ↑客户端使用↑ =====

    // ===== ↓服务端使用↓ =====
    /**
     * @brief 解析请求
     * @return coroutine::Task<bool> 断开连接则为false, 解析成功为true
     */
    template <typename Timeout>
        requires(requires { Timeout::Val; })
    coroutine::Task<bool> parserReq() {
        for (std::size_t n = IO::kBufMaxSize; n; n = _parserReq()) {
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
     * @brief 获取请求头键值对的引用
     * @return const std::unordered_map<std::string, std::string>& 
     */
    const auto& getHeaders() const noexcept {
        return _requestHeaders;
    }

    /**
     * @brief 解析查询参数 (解析如: `?name=loli&awa=ok&hitori`)
     * @return 返回解析到的字符串键值对哈希表
     * @warning 如果解析到不是键值对的, 即通过`&`分割后没有`=`的, 默认其全部为Key, 但Val = ""
     */
    std::unordered_map<std::string, std::string> getParseQueryParameters() const {
        auto path = getReqPath();
        std::size_t pos = path.find('?'); // 没必要反向查找
        if (pos == std::string::npos)
            return {};
        // 如果有#这种, 要删除: 无需处理, 这个只是存在于客户端, 不会传输到服务端(?)至少path没有
        auto parameter = path.substr(pos + 1);
        auto kvArr = HX::utils::StringUtil::split<std::string>(parameter, "&");
        std::unordered_map<std::string, std::string> res;
        for (const auto& it : kvArr) {
            auto&& kvPair = HX::utils::StringUtil::splitAtFirst(it, "=");
            if (kvPair.first == "")
                res.insert_or_assign(it, "");
            else
                res.insert(std::move(kvPair));
        }
        return res;
    }

    /**
     * @brief 获取请求类型
     * @return 请求类型 (如: "GET", "POST"...)
     */
    std::string_view getReqType() const noexcept {
        return _requestLine[RequestLineDataType::RequestType];
    }

    /**
     * @brief 获取请求体 ( 临时设计的 )
     * @return 如果没有请求体, 则返回`""`
     */
    std::string getReqBody() const noexcept {
        return _body;
    }

    /**
     * @brief 获取请求体
     * @return 如果没有请求体, 则返回`""`
     */
    std::string getReqBody() noexcept {
        return std::move(_body);
    }

    /**
     * @brief 获取请求PATH
     * @return 请求PATH (如: "/", "/home?loli=watasi"...)
     */
    std::string_view getReqPath() const noexcept {
        return _requestLine[RequestLineDataType::RequestPath];
    }

    /**
     * @brief 获取请求的纯PATH部分
     * @return 请求PATH (如: "/", "/home?loli=watasi"的"/home"部分)
     */
    std::string getPureReqPath() const noexcept {
        auto path = getReqPath();
        std::size_t pos = path.find('?');
        if (pos != std::string_view::npos) {
            path = path.substr(0, pos);
        }
        return {path.data(), path.size()};
    }

    /**
     * @brief 获取请求协议版本
     * @return 请求协议版本 (如: "HTTP/1.1", "HTTP/2.0"...)
     */
    std::string_view getProtocolVersion() const noexcept {
        return _requestLine[RequestLineDataType::ProtocolVersion];
    }

    /**
     * @brief 获取第`index`个路径参数的内容
     * @param index 路径参数索引, 如`/home/{name}/id`, `index = 0` => {name}
     * @throw std::runtime_error 如果路径参数未初始化则抛出 (端点函数必须为`{val}`格式)
     * @throw std::out_of_range 如果 `index` 超出可用路径参数范围时抛出
     * @note 调用前需要确保路径参数已正确初始化
     * @return std::string_view 
     */
    std::string_view getPathParam(std::size_t index) const {
        if (_wildcarDataArr.empty()) [[unlikely]] {
            throw std::runtime_error("No path parameters available to parse.");
        }
        return _wildcarDataArr[index];
    }

    /**
     * @brief 获取通配符路径参数的内容
     * @throw std::runtime_error 如果路径参数未初始化则抛出 (端点函数必须为`{val}`格式)
     * @return std::string_view 
     */
    std::string_view getUniversalWildcardPath() const {
        if (_urlWildcardData.empty()) [[unlikely]] {
            throw std::runtime_error("No path parameters available to parse.");
        }
        return _urlWildcardData;
    }

    RangeRequestView getRangeRequestView() const {
        return {getReqType(), _requestHeaders};
    }
    // ===== ↑服务端使用↑ =====

    /**
     * @brief 清空已有的请求内容, 并且初始化标准
     */
    void clear() noexcept {
        _requestLine.clear();
        _requestHeaders.clear();
        _requestHeadersIt = _requestHeaders.end();
        // _buf.clear();
        _recvBuf.clear();
        _body.clear();
        _completeRequestHeader = false;
        _remainingBodyLen.reset();
    }
private:
    /**
     * @brief 请求行数据分类
     */
    enum RequestLineDataType {
        RequestType = 0,        // 请求类型
        RequestPath = 1,        // 请求路径
        ProtocolVersion = 2,    // 协议版本
    };

    /**
     * @brief 仅用于读取时候写入的缓冲区
     */
    container::ArrayBuf<char, IO::kBufMaxSize> _recvBuf;

    std::vector<std::string> _requestLine;  // 请求行
    HeaderHashMap _requestHeaders;          // 请求头

    // 上一次解析的请求头
    decltype(_requestHeaders)::iterator _requestHeadersIt;

    // 请求体
    std::string _body;

    // @brief 仍需读取的请求体长度
    std::optional<std::size_t> _remainingBodyLen;

    /**
     * @brief 路径变量, 如`/home/{id}`的`id`, 
     * 存放的是解析后的结果字符串视图(指向的是Request的请求行)
     */
    // 单个路径变量的结果数组
    std::span<std::string_view> _wildcarDataArr;

    // 通配符的结果
    std::string_view _urlWildcardData;

    /**
     * @brief 是否解析完成请求头
     */
    bool _completeRequestHeader = false;

    IO& _io;

    friend class Router;

    template <typename Timeout>
        requires(requires { Timeout::Val; })
    friend class HttpClient;

    /**
     * @brief 解析请求
     * @return 是否需要继续解析;
     *         `== 0`: 不需要;
     *         `>  0`: 需要继续解析`size_t`个字节
     * @warning 假定内容是符合Http协议的
     */
    std::size_t _parserReq() {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        std::string_view buf{_recvBuf.data(), _recvBuf.size()};
        switch ((_completeRequestHeader << 1) | (!_requestLine.empty())) {
            case 0x00: { // 什么也没有解析, 开始解析请求行
                std::size_t pos = buf.find(CRLF);
                if (pos == std::string_view::npos) [[unlikely]] { // 不可能事件
                    return IO::kBufMaxSize;
                }
                std::string_view reqLine = buf.substr(0, pos);

                // 解析请求行
                _requestLine = utils::StringUtil::split<std::string>(
                    reqLine, " "sv
                );

                if (_requestLine.size() < 3) [[unlikely]] {
                    return IO::kBufMaxSize;
                } 
                // else if (_requestLine.size() > 3) [[unlikely]] {
                // @todo 理论上不会出现
                // }

                buf = buf.substr(pos + 2); // 再前进, 以去掉 "\r\n"
            }
            case 0x01: { // 解析完请求行, 开始解析请求头
                /**
                 * @brief 请求头
                 * 通过`\r\n`分割后, 取最前面的, 先使用最左的`:`以判断是否是需要作为独立的键值对;
                 * -  如果找不到`:`, 并且 非空, 那么它需要接在上一个解析的键值对的值尾
                 * -  否则即请求头解析完毕!
                 */
                while (!_completeRequestHeader) { // 请求头未解析完
                    std::size_t pos = buf.find(CRLF);
                    if (pos == std::string_view::npos) { // 没有读取完
                        _recvBuf.moveToHead(buf);
                        return IO::kBufMaxSize;
                    }
                    std::string_view subKVStr = buf.substr(0, pos);
                    auto p = utils::StringUtil::splitAtFirst(subKVStr, HEADER_SEPARATOR_SV);
                    if (p.first.empty()) [[unlikely]] {     // 找不到 ": "
                        if (subKVStr.size()) [[unlikely]] { // 很少会有分片传输请求头的
                            _requestHeadersIt->second.append(subKVStr);
                        } else { // 请求头解析完毕!
                            _completeRequestHeader = true;
                        }
                    } else {
                        // K: V, 其中 V 是区分大小写的, 但是 K 是不区分的
                        utils::StringUtil::toLower(p.first);
                        _requestHeadersIt = _requestHeaders.insert(p).first;
                    }
                    buf = buf.substr(pos + 2); // 再前进, 以去掉 "\r\n"
                }
            }
            case 0x03: { // 解析完请求头, 开始请求体解析
                if (_requestHeaders.contains(CONTENT_LENGTH_SV)) { // 存在content-length模式接收的响应体
                    // 是 空行之后 (\r\n\r\n) 的内容大小(char)
                    if (!_remainingBodyLen.has_value()) {
                        _body = buf;
                        _remainingBodyLen 
                            = std::stoull(_requestHeaders.find(CONTENT_LENGTH_SV)->second) 
                            - _body.size();
                    } else {
                        *_remainingBodyLen -= buf.size();
                        _body.append(buf);
                    }

                    if (*_remainingBodyLen != 0) {
                        _recvBuf.clear();
                        return *_remainingBodyLen;
                    }
                } else if (_requestHeaders.contains(TRANSFER_ENCODING_SV)) { // 存在请求体以`分块传输编码`
                    /**
                     * @todo 目前只支持 chunked 编码, 不支持压缩的 (2024-9-6 09:36:25) 
                     * */
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
                        _remainingBodyLen = std::stol(std::string {buf.substr(0, posLen)}, nullptr, 16); // 转换为十进制整数
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
                // @todo 断点续传协议
                break;
            }
            [[unlikely]] default:
#ifndef NDEBUG
                throw std::runtime_error{"parserRequest UB Error"}
#endif // NDEBUG
            ;
        }
        return 0;
    }
};

} // namespace HX::net

#endif // _HX_REQUEST_H_
