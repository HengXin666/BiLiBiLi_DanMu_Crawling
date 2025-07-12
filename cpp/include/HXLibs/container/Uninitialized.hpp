#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-09 23:17:25
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
#ifndef _HX_UNINITIALIZED_H_
#define _HX_UNINITIALIZED_H_

#include <utility>

#include <HXLibs/container/NonVoidHelper.hpp>

namespace HX::container {

template <typename T>
struct Uninitialized {
    Uninitialized() noexcept 
        : _available{false}
    {}
    
    Uninitialized(Uninitialized const&) = delete;
    Uninitialized& operator=(Uninitialized const&) = delete;
    
    Uninitialized(Uninitialized&& that) noexcept 
        : _available(that._available)
    {
        if (that._available) {
            set(that.move());
        }
    }
    Uninitialized& operator=(Uninitialized&& that) noexcept {
        del();
        if (that._available) {
            set(that.move());
        }
        return *this;
    }

    bool isAvailable() const noexcept {
        return _available;
    }

    ~Uninitialized() noexcept {
        del();
    }

    T move() noexcept {
        _available = false;
        return std::move(_data);
    }

    template <typename... Ts>
    void set(Ts&&... args) {
        new (std::addressof(_data)) T(std::forward<Ts>(args)...);
        _available = true;
    }
private:
    void del() noexcept {
        if (_available) {
            _data.~T();
            _available = false;
        }
    }

    union {
        T _data;
    };
    bool _available;
};

template <>
struct Uninitialized<void> {
    Uninitialized() noexcept = default;
    
    Uninitialized(Uninitialized const&) = delete;
    Uninitialized& operator=(Uninitialized const&) = delete;

    Uninitialized(Uninitialized&&) noexcept = default;
    Uninitialized& operator=(Uninitialized&&) noexcept = default;
    
    ~Uninitialized() noexcept = default;
    
    bool isAvailable() const noexcept {
        return true;
    }

    auto move() noexcept { return NonVoidHelper<>{}; }
    void set(NonVoidHelper<>) noexcept {}
};

template <typename T>
struct Uninitialized<T const> : public Uninitialized<T> {};

// 如果需要返回引用, 则需要 std::ref() 包裹
template <typename T>
struct Uninitialized<T&> : public Uninitialized<std::reference_wrapper<T>> {};

template <typename T>
struct Uninitialized<T&&> : public Uninitialized<T> {};

} // namespace HX::container

#endif // !_HX_UNINITIALIZED_H_