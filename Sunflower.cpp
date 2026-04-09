#include "Sunflower.h"
#include "config.h"
#include <easyx.h>
#include <stdlib.h>
#include "tool.h"
#include "vector2.h"
#include "sunshineBall.h"

/**
 * @file Sunflower.cpp
 * @brief 向日葵实现文件
 *
 * 包含向日葵的更新逻辑和生产阳光机制
 */

// 外部引用：阳光球数组和植物图片数组
extern SunshineBall balls[10];                 // 阳光球数组（最多10个）
extern IMAGE* imgPlant[ZHI_WU_COUNT][20];       // 植物动画帧图片

/**
 * @brief 向日葵构造函数
 * @param row 行号
 * @param col 列号
 *
 * 初始化向日葵：
 * - 设置初始血量（100）
 * - 重置动画计时器
 */
Sunflower::Sunflower(int row, int col) : Plant(100, row, col), frameTimer(0) {
}

/**
 * @brief 更新向日葵
 * @param dt 时间步长
 *
 * 更新逻辑：
 * 1. 每6帧切换一次动画帧，实现动画播放效果
 * 2. 每600帧（约10秒）调用produceSunshine()生产一个阳光球
 */
void Sunflower::update(double dt) {
    // 更新动画帧
    frameTimer++;
    if (frameTimer >= FRAME_DELAY) {
        frameIndex = (frameIndex + 1) % getFrameCount();
        frameTimer = 0;
    }

    // 检查是否需要生产阳光
    timer++;  // timer继承自Plant类，用于计时
    if (timer >= SUNSHINE_INTERVAL) {
        produceSunshine();  // 生产阳光
        timer = 0;          // 重置计时器
    }
}

/**
 * @brief 绘制向日葵
 *
 * 从图片数组中获取当前帧的图片并绘制到屏幕
 */
void Sunflower::draw() {
    int plantType = getType();
    int maxFrame = getFrameCount();

    // 边界检查，防止数组越界
    if (frameIndex < 0) frameIndex = 0;
    if (frameIndex >= maxFrame) frameIndex = maxFrame - 1;

    IMAGE* img = imgPlant[plantType][frameIndex];
    if (img != nullptr) {
        putimagePNG(x, y, img);
    }
}

/**
 * @brief 生产阳光
 *
 * 在阳光球数组中找一个空闲位置，创建一个新的阳光球：
 * - 初始位置：向日葵的位置
 * - 使用贝塞尔曲线控制飞行轨迹
 * - 阳光球从地面飞向天空，然后落向随机位置
 *
 * 贝塞尔曲线控制点说明：
 * - controlP1: 起点（向日葵位置）
 * - controlP2/P3: 控制点1/2（向上偏移200像素，形成弧线）
 * - controlP4: 终点（地面随机位置）
 *
 * 设计理由：向日葵产生的阳光需要从植物位置飘向空中再落下，
 * 使用贝塞尔曲线可以实现自然的抛物线效果
 */
void Sunflower::produceSunshine() {
    // 找一个空闲的阳光球槽位
    int k;
    for (k = 0; k < ballMax && balls[k].isUsed; k++);
    if (k >= ballMax) return;  // 阳光球数组满了

    // 初始化阳光球属性
    balls[k].isUsed = true;
    balls[k].controlP1 = vector2(x, y);  // 起点：向日葵位置

    // 计算终点位置（在向日葵左右随机偏移100~150像素）
    int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);
    balls[k].controlP4 = vector2(x + w, y + 50);  // 终点：稍微偏移

    // 设置贝塞尔曲线的两个控制点（向上弯曲）
    // 控制点2：在起点右侧30%处，向上200像素
    balls[k].controlP2 = vector2(balls[k].controlP1.x + w * 0.3f, balls[k].controlP1.y - 200);
    // 控制点3：在起点右侧70%处，向上200像素
    balls[k].controlP3 = vector2(balls[k].controlP1.x + w * 0.7f, balls[k].controlP1.y - 200);

    // 设置动画参数
    balls[k].state = 3;   // SUNSHINE_PRODUCT：生产状态
    balls[k].speed = 0.02f;  // 动画速度
    balls[k].timeT = 0;      // 初始时间参数
}