#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-01-16 19:30:27
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
#ifndef _HX_JSON_WRITE_H_
#define _HX_JSON_WRITE_H_

#include <HXLibs/log/serialize/ToString.hpp>

namespace HX::reflection {

template <typename Obj, typename Stream>
inline void toJson(Obj&& obj, Stream& s) {
    log::internal::ToString<Obj>::toString(std::forward<Obj>(obj), s);
}

} // namespace HX::reflection

#endif // !_HX_JSON_WRITE_H_