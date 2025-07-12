#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-19 14:09:37
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
#ifndef _HX_TIMERLOOP_H_
#define _HX_TIMERLOOP_H_

#include <chrono>
#include <map>
#include <coroutine>

namespace HX::coroutine {

/**
 * @brief 定时器循环
 */
struct TimerLoop {
    using TimerTree = std::multimap<std::chrono::system_clock::time_point, std::coroutine_handle<>>;

    TimerLoop& operator=(TimerLoop&&) = delete;

    std::optional<std::chrono::system_clock::duration> run() {
        while (_timerTree.size()) {
            auto nowTime = std::chrono::system_clock::now();
            auto it = _timerTree.begin();
            if (it->first <= nowTime) {
                it->second.resume();
                _timerTree.erase(it);
            } else {
                return it->first - nowTime;
            }
        }
        return {};
    }

    TimerTree::iterator addTimer(
        std::chrono::system_clock::time_point expireTime,
        std::coroutine_handle<> coroutine
    ) {
        return _timerTree.insert({expireTime, coroutine});
    }
    struct [[nodiscard]] TimerAwaiter {
        TimerAwaiter(TimerLoop* timerLoop)
            : _timerLoop{timerLoop}
            , _expireTime{}
            , _it{}
        {}

        TimerAwaiter(TimerAwaiter const&) = delete;
        TimerAwaiter& operator=(TimerAwaiter const&) noexcept = delete;

        TimerAwaiter(TimerAwaiter&&) = default;
        TimerAwaiter& operator=(TimerAwaiter&&) noexcept = default;

        bool await_ready() const noexcept {
            return false;
        }
        auto await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            _it = _timerLoop->addTimer(_expireTime, coroutine);
        }
        void await_resume() const noexcept {
            _it.reset(); // 如果继续, 说明是从 TimerTree::run() 来的
                         // 之后外界会执行 _timerTree.erase(_it)
                         // 因此内部要执行 _it = {}, 防止多次 erase
        }
        TimerAwaiter&& setExpireTime(std::chrono::system_clock::time_point expireTime) && noexcept {
            _expireTime = expireTime;
            return std::move(*this);
        }
        ~TimerAwaiter() noexcept {
            if (_it) {
                _timerLoop->_timerTree.erase(*_it);
            }
        }
    private:
        TimerLoop* _timerLoop;
        std::chrono::system_clock::time_point _expireTime;  // 过期时间
        mutable std::optional<TimerTree::iterator> _it;     // 红黑树迭代器
    };
private:
    struct [[nodiscard]] TimerAwaiterBuilder {
        TimerAwaiterBuilder(TimerLoop* timerLoop)
            : _timerLoop{timerLoop}
        {}

        TimerAwaiterBuilder& operator=(TimerAwaiterBuilder&&) noexcept = delete;

        /**
         * @brief 暂停一段时间
         * @param duration 比如 3s
         */
        TimerAwaiter sleepFor(std::chrono::system_clock::duration duration) && {
            return TimerAwaiter{_timerLoop}.setExpireTime(
                std::chrono::system_clock::now() + duration);
        }
        /**
         * @brief 暂停指定时间点
         * @param timerLoop 计时器循环对象
         * @param expireTime 时间点, 如 2024-8-4 22:12:23
         */
        TimerAwaiter sleepUntil(std::chrono::system_clock::time_point expireTime) && {
            return TimerAwaiter{_timerLoop}.setExpireTime(expireTime);
        }
        TimerLoop* _timerLoop;
    };
public:
    /**
     * @brief 创建一个定时器工厂, 需要用户指定定时的时间
     * @param timerLoop 
     * @return TimerAwaiterBuilder 
     */
    static TimerAwaiterBuilder makeTimer(TimerLoop& timerLoop) {
        return {&timerLoop};
    }
private:
    TimerTree _timerTree;
};

} // namespace HX::coroutine

#endif // !_HX_TIMERLOOP_H_