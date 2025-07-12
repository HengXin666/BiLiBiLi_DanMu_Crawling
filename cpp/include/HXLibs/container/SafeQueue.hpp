#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-10 09:52:01
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
#ifndef _HX_SAFE_QUEUE_H_
#define _HX_SAFE_QUEUE_H_

#include <queue>
#include <mutex>
#include <shared_mutex>

namespace HX::container {

/**
 * @brief 线程安全的队列
 * @tparam T 
 */
template <typename T>
struct SafeQueue {
    SafeQueue()
        : _queue{}
        , _mtx{}
    {}

    SafeQueue(SafeQueue const&) = delete;
    SafeQueue& operator=(SafeQueue const&) = delete;

    decltype(auto) front() const {
        std::shared_lock _{_mtx};
        return _queue.front();
    }

    void pop() {
        std::unique_lock _{_mtx};
        _queue.pop();
    }

    T frontAndPop() {
        std::unique_lock _{_mtx};
        T res {std::move(_queue.front())};
        _queue.pop();
        return res;
    }

    template <typename... Args>
    decltype(auto) emplace(Args&&... args) {
        std::unique_lock _{_mtx};
        return _queue.emplace(std::forward<Args>(args)...);
    }

    std::size_t size() const {
        std::shared_lock _{_mtx};
        return _queue.size();
    }

    bool empty() const {
        std::shared_lock _{_mtx};
        return _queue.empty();
    }
private:
    std::queue<T> _queue;
    mutable std::shared_mutex _mtx;
};

} // namespace HX::container

#endif // !_HX_SAFE_QUEUE_H_