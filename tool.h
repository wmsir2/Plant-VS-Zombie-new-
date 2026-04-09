#pragma once
#include <graphics.h>

/**
 * @file tool.h
 * @brief 工具函数声明
 *
 * 包含游戏使用的辅助函数：
 * - putimagePNG: 绘制PNG图片（支持透明背景）
 * - getDelay: 获取时间延迟
 *
 * 设计理由：这些是跨多个模块使用的通用功能
 */

/**
 * @brief 绘制PNG图片（支持透明背景）
 * @param picture_x 图片X坐标
 * @param picture_y 图片Y坐标
 * @param picture 图片指针
 *
 * 与EasyX的putimage不同，这个函数：
 * 1. 支持PNG的Alpha通道透明度
 * 2. 自动处理图片边界裁剪（不会绘制到屏幕外）
 *
 * 使用alpha混合算法：Cp = p*FP + (1-p)*BP
 * 其中p是源像素的Alpha值，FP是前景色，BP是背景色
 */
void putimagePNG(int picture_x, int picture_y, IMAGE* picture);

/**
 * @brief 获取距离上次调用的毫秒数
 * @return 上次调用到这次调用经过的毫秒数
 *
 * 常用于帧率控制：
 * - 第一次调用返回0
 * - 之后的调用返回与上次的时间差
 *
 * 设计理由：用于控制动画播放速度，确保帧率稳定
 */
int getDelay();