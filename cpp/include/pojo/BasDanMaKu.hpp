#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-07-12 18:16:05
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
#ifndef _HX_BAS_DANMAKU_H_
# define _HX_BAS_DANMAKU_H_

# include <string>
# include <vector>

namespace HX::bas {

// 分段弹幕包信息?
struct DmSegConfig {
    int64_t pageSize; // 分段时间?
    int64_t total;    // 最大分页数?
};

//
struct DanmakuFlagConfig {
    int32_t recFlag;     //
    std::string recText; //
    int32_t recSwitch;   //
};

// 互动弹幕条目
struct CommandDm {
    int64_t id;          // 弹幕dmid
    int64_t oid;         // 视频cid
    int64_t mid;         // 发送者mid
    std::string command; // 弹幕指令
    std::string content; // 弹幕文字
    int32_t progress;    // 弹幕出现时间
    std::string ctime;   //
    std::string mtime;   //
    std::string extra;   // 弹幕负载数据
    std::string idStr;   // 弹幕dmid（字串形式）
};

// 弹幕个人配置
struct DanmuWebPlayerConfig {
    bool dmSwitch;          // 弹幕开关
    bool aiSwitch;          // 智能云屏蔽
    int32_t aiLevel;        // 智能云屏蔽级别
    bool blocktop;          // 屏蔽类型-顶部
    bool blockscroll;       // 屏蔽类型-滚动
    bool blockbottom;       // 屏蔽类型-底部
    bool blockcolor;        // 屏蔽类型-彩色
    bool blockspecial;      // 屏蔽类型-特殊
    bool preventshade;      // 防挡弹幕（底部15%）
    bool dmask;             // 智能防挡弹幕（人像蒙版）
    float opacity;          // 弹幕不透明度
    int32_t dmarea;         // 弹幕显示区域
    float speedplus;        // 弹幕速度
    float fontsize;         // 字体大小
    bool screensync;        // 跟随屏幕缩放比例
    bool speedsync;         // 根据播放倍速调整速度
    std::string fontfamily; // 字体类型?
    bool bold;              // 粗体?
    int32_t fontborder;     // 描边类型
    std::string drawType;   // 渲染类型?
};

struct DmWebViewReply {
    int32_t state;                       // 弹幕开放状态
    std::string text;                    //
    std::string textSide;                //
    DmSegConfig dmSge;                   // 分段弹幕包信息?
    DanmakuFlagConfig flag;              //
    std::vector<std::string> specialDms; // BAS（代码）弹幕专包url
    bool checkBox;                       //
    int64_t count;                       // 实际弹幕总数
    std::vector<CommandDm> commandDms;   // 互动弹幕条目
    DanmuWebPlayerConfig dmSetting;      // 弹幕个人配置
};

} // namespace HX

#endif // !_HX_BAS_DANMAKU_H_
