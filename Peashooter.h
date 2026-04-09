#pragma once
#include "Plant.h"
#include "config.h"

/**
 * @file Peashooter.h
 * @brief 豌豆射手类定义
 *
 * 豌豆射手是最基础的攻击植物：
 * - 每隔80帧检查是否有僵尸进入危险区域
 * - 如果同列有僵尸且在攻击范围内，发射豌豆子弹
 * - 豌豆向左飞行，击中僵尸造成伤害
 *
 * 设计理由：将攻击逻辑封装在植物内部，通过canShoot()判断是否需要射击
 */

/**
 * @class Peashooter
 * @brief 豌豆射手类
 *
 * 继承自Plant基类，重写了以下方法：
 * - update(): 处理动画更新和射击逻辑
 * - draw(): 绘制豌豆射手动画帧
 * - getType(): 返回PEASHOOTER类型标识
 */
class Peashooter : public Plant {
public:
    /**
     * @brief 更新豌豆射手状态
     * @param dt 时间步长（秒）
     *
     * 主要职责：
     * 1. 更新动画帧（每5帧切换一次）
     * 2. 检查是否可以射击（canShoot）
     * 3. 如果可以且射击冷却结束，调用shoot()发射子弹
     */
    void update(double dt) override;

    /**
     * @brief 绘制豌豆射手
     *
     * 使用当前帧索引从图片数组中获取对应图片并绘制
     */
    void draw() override;

    // 类型标识
    PlantType getType() const override { return PEASHOOTER; }

    // 获取坐标（用于外部访问）
    int getX() const override { return x; }
    int getY() const override { return y; }
    int getRow() const override { return row; }
    int getCol() const override { return col; }

    // 获取动画帧数（豌豆射手有13帧动画）
    int getFrameCount() const override { return 13; }

    /**
     * @brief 构造函数
     * @param row 行号 (0~2)
     * @param col 列号 (0~8)
     *
     * 创建豌豆射手，初始血量100，位置由行列决定
     */
    Peashooter(int row, int col);

    /**
     * @brief 检查是否可以射击
     * @return true 可以射击
     * @return false 不能射击
     *
     * 判断条件：
     * - 同列有存活的僵尸
     * - 僵尸X坐标小于危险区域边界（900-60=840）
     *
     * 设计理由：只在有僵尸靠近时才射击，避免浪费子弹
     */
    bool canShoot() const;

    /**
     * @brief 发射豌豆
     *
     * 在子弹数组中找一个空闲位置，初始化子弹属性：
     * - 位置：植物右侧 (x+50)
     * - 方向：向左飞行
     * - 速度：4像素/帧
     */
    void shoot();

private:
    int shootTimer = 0;    /**< 射击冷却计时 */
    int frameTimer = 0;    /**< 动画帧切换计时 */
    static constexpr int SHOOT_INTERVAL = 80;   /**< 射击间隔（80帧≈1.3秒@60FPS） */
    static constexpr int FRAME_DELAY = 5;        /**< 动画帧切换延迟（5帧） */
};