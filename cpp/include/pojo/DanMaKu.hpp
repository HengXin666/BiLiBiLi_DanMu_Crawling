#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-12 16:16:09
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
#ifndef _HX_DANMAKU_H_
#define _HX_DANMAKU_H_

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace HX {

struct DanmakuElem {
    int64_t id;               // 弹幕id
    int32_t progress;         // 出现时间（单位 ms）
    int32_t mode;             // 弹幕类型
    int32_t fontsize;         // 字号
    uint32_t color;           // 颜色（RGB）
    std::string midHash;      // 发送者mid hash
    std::string content;      // 弹幕内容
    int64_t ctime;            // 发送时间
    int32_t weight;           // 权重
    std::string action;       // 动作
    int32_t pool;             // 弹幕池（0普通池）
    std::string idStr;        // 弹幕id str
    int32_t attr;             // 属性位（如保护/高赞等）
    std::string animation;    // 动画（暂未使用）
    std::optional<int32_t> colorful; // 大会员渐变色（可选）
};

struct DmSegMobileReply {
    std::vector<DanmakuElem> elems;
    int32_t state = 0;
    // 以下字段可省略
    // DanmakuAIFlag ai_flag;
    // std::vector<DmColorful> colorfulSrc;
};

struct DmWebViewReply {
    std::vector<std::string> special_dms; // repeated string special_dms = 6;
};

} // namespace HX

#endif // !_HX_DANMAKU_H_