#pragma once
#include "Plant.h"
#include "config.h"

/**
 * @file PotatoMine.h
 * @brief 土豆地雷类定义
 *
 * 土豆地雷是一种陷阱植物：
 * - 刚种植时处于HIDDEN（地下）状态，不可被吃
 * - 480帧后进入RISING（升起）状态
 * - 再过240帧后进入READY（就绪）状态，可以爆炸
 * - 当僵尸进入爆炸范围时，立即爆炸，对范围内所有僵尸造成150点伤害
 * - 爆炸后进入EXPLODING状态，120帧后标记为死亡
 *
 * 设计理由：土豆地雷使用状态机管理生命周期，从种植到爆炸有明确的阶段
 */

/**
 * @brief 土豆地雷状态枚举
 *
 * HIDDEN → RISING → READY → EXPLODING
 *
 * - HIDDEN: 地下状态，不可见，不能被吃，需要480帧才能升起
 * - RISING: 升起中，从地下钻出来，需要240帧
 * - READY: 就绪，可以检测僵尸并爆炸
 * - EXPLODING: 爆炸中，显示爆炸效果，120帧后结束
 */
enum class PotatoMineState {
    HIDDEN,
    RISING,
    READY,
    EXPLODING
};

/**
 * @class PotatoMine
 * @brief 土豆地雷类
 *
 * 继承自Plant，是陷阱型植物
 */
class PotatoMine : public Plant {
public:
    /**
     * @brief 更新土豆地雷状态
     * @param dt 时间步长（秒）
     *
     * 根据当前状态执行不同逻辑：
     * - HIDDEN: 计时，480帧后进入RISING
     * - RISING: 计时，同时检测僵尸
     * - READY: 持续检测僵尸
     * - EXPLODING: 爆炸动画，120帧后结束
     */
    void update(double dt) override;

    /**
     * @brief 绘制土豆地雷
     *
     * 根据当前状态绘制不同图片：
     * - HIDDEN: 地下图片
     * - RISING/READY: 地面图片
     * - EXPLODING: 无（使用独立的爆炸效果）
     */
    void draw() override;

    // 类型标识
    PlantType getType() const override { return POTATO_MINE; }

    // 获取坐标
    int getX() const override { return x; }
    int getY() const override { return y; }
    int getRow() const override { return row; }
    int getCol() const override { return col; }

    /**
     * @brief 构造函数
     * @param row 行号
     * @param col 列号
     *
     * 初始状态为HIDDEN，血量100
     */
    PotatoMine(int row, int col);

    // 获取爆炸伤害值
    int getDamage() const { return damage; }

    // 获取当前状态
    PotatoMineState getState() const { return state; }

    // 获取爆炸范围
    static constexpr int getBlastRange() { return BLAST_RANGE; }

    // 获取爆炸持续时间
    static constexpr int getExplodeTime() { return EXPLODE_TIME; }

    /**
     * @brief 触发爆炸
     *
     * 当僵尸进入爆炸范围时调用：
     * 1. 切换到EXPLODING状态
     * 2. 对范围内所有同列僵尸造成150点伤害
     * 3. 创建爆炸效果
     */
    void triggerExplosion();

    // 植物特性
    bool canBeEaten() const override;  // 只有HIDDEN状态可被吃
    void takeDamage(int damage);  // 受到伤害时可能提前爆炸

    // 死亡判定
    bool isDead() const override;
    bool isAlive() const;  // EXPLODING状态视为死亡

private:
    /**
     * @brief 检测范围内是否有僵尸
     *
     * 检查同列僵尸是否在爆炸范围内（以土豆地雷为中心±60像素）
     * 如果有，调用triggerExplosion()
     */
    void checkZombieInRange();

    PotatoMineState state = PotatoMineState::HIDDEN;  /**< 当前状态 */
    int damage = 150;                                  /**< 爆炸伤害值 */
    static constexpr int HIDDEN_TIME = 600;            /**< 隐藏时间（600帧=10秒） */
    static constexpr int RISING_TIME = 60;             /**< 升起时间（60帧=1秒） */
    static constexpr int EXPLODE_TIME = 100;            /**< 爆炸持续时间 */
    static constexpr int BLAST_RANGE = 60;             /**< 爆炸范围（±60像素） */
};