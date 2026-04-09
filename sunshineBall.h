#pragma once
#include "config.h"
#include "vector2.h"

/**
 * @file sunshineBall.h
 * @brief 阳光球类定义
 *
 * 阳光球是游戏中的资源单位：
 * - 由向日葵生产或从天空掉落
 * - 玩家点击收集后飞向UI栏的阳光池
 * - 使用贝塞尔曲线实现自然流畅的飞行轨迹
 *
 * 设计理由：阳光球的状态机管理其生命周期
 * 通过不同的state区分下落、等待、收集、飞向UI等阶段
 */

/**
 * @class SunshineBall
 * @brief 阳光球类
 *
 * 负责管理阳光的位置、动画和状态
 */
class SunshineBall {
public:
    int posX;            /**< X坐标（暂未使用，已被currentPos替代） */
    int posY;            /**< Y坐标（暂未使用，已被currentPos替代） */
    int frame;           /**< 当前动画帧（0~28，共29帧） */
    int targetY;         /**< 目标Y坐标（暂未使用） */
    bool isUsed;         /**< 是否被使用（用于对象池管理） */
    int timer;           /**< 等待收集计时（超过300帧开始计时消失） */

    float xOffset;       /**< X偏移（暂未使用） */
    float yOffset;       /**< Y偏移（暂未使用） */

    float timeT;         /**< 贝塞尔曲线时间参数（0~1） */

    // 贝塞尔曲线的四个控制点
    struct vector2 controlP1;  /**< 曲线起点（初始位置） */
    struct vector2 controlP2; /**< 控制点2（向上偏移，形成弧线） */
    struct vector2 controlP3; /**< 控制点3（向上偏移，形成弧线） */
    struct vector2 controlP4; /**< 曲线终点（目标位置） */

    struct vector2 currentPos; /**< 当前位置（实时计算） */
    float speed;               /**< 飞行速度（时间参数增量） */
    int state;                /**< 当前状态（见config.h的枚举） */
};