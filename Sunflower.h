#pragma once
#include "Plant.h"
#include "config.h"

/**
 * @file Sunflower.h
 * @brief 向日葵类定义
 *
 * 向日葵是阳光生产植物：
 * - 每600帧（约10秒@60FPS）生产一个阳光球
 * - 阳光球以贝塞尔曲线动画飞向天空，然后落到地面
 * - 玩家收集阳光用于购买植物
 *
 * 设计理由：将阳光生产逻辑封装在植物内部，通过produceSunshine()创建阳光球
 */

/**
 * @class Sunflower
 * @brief 向日葵类
 *
 * 继承自Plant基类，每隔一定时间生产阳光
 */
class Sunflower : public Plant {
public:
    /**
     * @brief 更新向日葵状态
     * @param dt 时间步长（秒）
     *
     * 主要职责：
     * 1. 更新动画帧（每6帧切换一次）
     * 2. 检查是否需要生产阳光（每600帧生产一次）
     */
    void update(double dt) override;

    /**
     * @brief 绘制向日葵
     *
     * 使用当前帧索引从图片数组中获取对应图片并绘制
     */
    void draw() override;

    // 类型标识
    PlantType getType() const override { return SUNFLOWER; }

    // 获取坐标（用于外部访问）
    int getX() const override { return x; }
    int getY() const override { return y; }
    int getRow() const override { return row; }
    int getCol() const override { return col; }

    // 获取动画帧数（向日葵有18帧动画）
    int getFrameCount() const override { return 18; }

    /**
     * @brief 构造函数
     * @param row 行号 (0~2)
     * @param col 列号 (0~8)
     *
     * 创建向日葵，初始血量100，位置由行列决定
     */
    Sunflower(int row, int col);

    /**
     * @brief 生产阳光
     *
     * 在阳光球数组中找一个空闲位置，创建阳光球：
     * - 使用贝塞尔曲线控制阳光球飞向天空的轨迹
     * - 阳光球最终落在地面，进入SUNSHINE_GROUND状态等待玩家收集
     */
    void produceSunshine();

private:
    int frameTimer = 0;                        /**< 动画帧切换计时 */
    static constexpr int SUNSHINE_INTERVAL = 600; /**< 生产阳光间隔（600帧=10秒@60FPS） */
    static constexpr int FRAME_DELAY = 6;        /**< 动画帧切换延迟（6帧） */
};