#pragma once
#include "Plant.h"

/**
 * @file Zombie.h
 * @brief 僵尸类定义
 *
 * 僵尸是玩家的敌人，从屏幕右侧生成：
 * - 沿小路向左移动
 * - 遇到植物时停下来啃食
 * - 可以被子弹击中受伤或击杀
 * - 被土豆地雷炸死时不播放死亡动画
 *
 * 设计理由：僵尸是游戏的主要威胁，通过状态机管理行走、啃食、死亡等行为
 */

/**
 * @brief 僵尸状态枚举
 *
 * 僵尸有三种状态：
 * - WALKING: 正常行走，向左移动
 * - EATING: 啃食植物，停在原地
 * - DEAD: 死亡，播放死亡动画
 *
 * 状态转换：
 * WALKING → EATING（遇到植物）
 * WALKING → DEAD（被子弹击杀或爆炸击杀）
 * EATING → WALKING（植物死亡后继续前进）
 */
enum ZombieState {
    WALKING,
    EATING,
    DEAD
};

/**
 * @class Zombie
 * @brief 僵尸类
 *
 * 僵尸的主要行为：
 * - update(): 更新位置和动画
 * - attack(): 啃食植物造成伤害
 * - explodeByPotato(): 被土豆地雷炸死
 */
class Zombie {
private:
    int damage;  /**< 啃食伤害（每30帧造成10点伤害） */

public:
    /**
     * @brief 啃食攻击植物
     * @param plant 植物指针
     *
     * 当僵尸处于EATING状态时，每隔一定时间调用此函数
     * 减少植物的血量
     */
    void attack(Plant* plant);

    // 位置和状态
    int x, y;                /**< 僵尸屏幕坐标 */
    int frameIndex;          /**< 当前动画帧索引 */
    bool used;               /**< 是否被使用（用于对象池管理） */
    int speed;               /**< 移动速度（像素/帧） */
    int row;                 /**< 所在行 (0~2) */
    int col;                 /**< 所在列 (0~8，暂未使用) */
    int blood;               /**< 血量 */
    bool dead;               /**< 是否死亡 */
    bool eating;             /**< 是否正在啃食 */
    int eatingTime;          /**< 啃食计时（用于控制伤害频率） */
    bool exploded;           /**< 是否被炸死（被土豆地雷炸死，不播放死亡动画） */
    ZombieState state;       /**< 当前状态 */
    Plant* targetPlant;      /**< 目标植物（正在啃食的植物） */

    /**
     * @brief 默认构造函数
     *
     * 初始化僵尸为未使用状态
     */
    Zombie() : used(false), x(0), row(0), y(0), speed(0), blood(100),
               dead(false), eating(false), frameIndex(0), exploded(false), damage(10) {}

    /**
     * @brief 被土豆地雷炸死
     * @param damage 爆炸伤害值
     *
     * 设置exploded=true，标记为不播放死亡动画
     * 血量减少后如果<=0则立即死亡
     */
    void explodeByPotato(int damage);

    /**
     * @brief 更新僵尸
     *
     * 更新僵尸位置、动画和状态
     */
    void update();

    /**
     * @brief 播放僵尸动画
     *
     * 根据当前状态更新动画帧：
     * - DEAD: 播放死亡动画（20帧后移除）
     * - EATING: 播放啃食动画（21帧循环）
     * - WALKING: 播放走路动画（22帧循环）
     */
    void animate();
};