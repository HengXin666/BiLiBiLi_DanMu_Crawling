#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-18 22:16:09
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
#ifndef _HX_WEB_SOCKET_H_
#define _HX_WEB_SOCKET_H_

#include <liburing.h>
#include <memory>
#include <chrono>
#include <functional>

#include <HXSTL/coroutine/task/Task.hpp>

namespace HX { namespace web { namespace server {

template <class T>
class IO;

}}} // namespace HX::web::server

namespace HX { namespace web { namespace protocol { namespace websocket {

/**
 * @brief WebSocket包
 */
struct WebSocketPacket {
    /**
     * @brief 操作码 (协议规定的!)
     */
    enum OpCode : uint8_t {
        Text = 1,   // 文本数据
        Binary = 2, // 二进制数据
        Close = 8,  // 关闭
        Ping = 9,
        Pong = 10,
    } _opCode;

    /// @brief 内容
    std::string _content;
};

/**
 * @brief WebSocket
 */
class WebSocket {
    using pointer = std::shared_ptr<WebSocket>;

    /**
     * @brief 尝试升级为 WebSocket
     * @param io
     * @return bool 是否升级成功
     */
    static HX::STL::coroutine::task::Task<bool> httpUpgradeToWebSocket(
        const HX::web::server::IO<void>& io
    );

    /**
     * @brief 读取一个WebSocketPacket
     * @param timeout 超时时间
     * @return std::optional<WebSocketPacket>
     */
    HX::STL::coroutine::task::Task<std::optional<WebSocketPacket>> recvPacket(
        std::chrono::steady_clock::duration timeout
    );

    /**
     * @brief 发送一个WebSocketPacket
     * @param packet WebSocketPacket
     * @param mask 掩码
     */
    HX::STL::coroutine::task::Task<> sendPacket(WebSocketPacket packet, uint32_t mask = 0);

    /**
     * @brief 发送一个Ping包
     */
    HX::STL::coroutine::task::Task<> sendPing();
public:
    WebSocket(WebSocket &&) = default;

    explicit WebSocket(const HX::web::server::IO<void>& io) : _io(io)
    {}

    /**
     * @brief 创建一个WebSocket协议的服务端, 如果可以升级为 WebSocket 则返回 WebSocket指针, 否则是nullptr
     * @return WebSocket指针
     */
    static HX::STL::coroutine::task::Task<pointer> makeServer(
        const HX::web::server::IO<void>& io
    );

    /**
     * @brief 启动 WebSocket
     * @param pingPongTimeout 心跳包超时时间
     * @return 协程任务 (需要`co_await`)
     */
    HX::STL::coroutine::task::Task<> start(
        std::chrono::steady_clock::duration pingPongTimeout = std::chrono::seconds(5)
    );

    /**
     * @brief 设置收到客户端消息时候触发的回调函数
     * @param onMessage 回调函数
     */
    void setOnMessage(
        std::function<HX::STL::coroutine::task::Task<>(const std::string&)> onMessage
    ) {
        _onMessage = std::move(onMessage);
    }

    /**
     * @brief 设置关闭WebSocket时候触发的回调函数
     * @param onClose 回调函数
     */
    void setOnClose(
        std::function<HX::STL::coroutine::task::Task<>()> onClose
    ) {
        _onClose = std::move(onClose);
    }

    /**
     * @brief 设置收到客户端回复的Pong包的时候触发的回调函数
     * @param onPong 回调函数
     */
    void setOnPong(
        std::function<HX::STL::coroutine::task::Task<>(std::chrono::steady_clock::duration)> onPong
    ) {
        _onPong = std::move(onPong);
    }

    /**
     * @brief 发送文本包给客户端
     * @param text 文本
     */
    HX::STL::coroutine::task::Task<> send(const std::string& text);
protected:
    const HX::web::server::IO<void>& _io;
    // === 回调函数 ===
    std::function<HX::STL::coroutine::task::Task<>(const std::string&)> _onMessage;
    std::function<HX::STL::coroutine::task::Task<>()> _onClose;
    std::function<HX::STL::coroutine::task::Task<>(std::chrono::steady_clock::duration)> _onPong;

    /// @brief 最后一次ping的时间点
    std::chrono::steady_clock::time_point _lastPingTime {};

    /// @brief 是否处于Pong看看对方嘎了没有 阶段
    bool _waitingPong = false;

    /// @brief 是否处于半关闭状态
    bool _halfClosed = false;
};

}}}} // HX::web::protocol::websocket

#endif // !_HX_WEB_SOCKET_H_