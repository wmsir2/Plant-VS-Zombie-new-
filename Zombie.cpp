#include "Zombie.h"
#include "Plant.h"
#include "config.h"

/**
 * @file Zombie.cpp
 * @brief 僵尸实现文件
 *
 * 包含僵尸的啃食、爆炸和更新逻辑
 */

/**
 * @brief 啃食攻击植物
 * @param plant 植物指针
 *
 * 当僵尸处于EATING状态时调用，每30帧造成一次伤害
 * 伤害值由damage成员变量定义（默认10点）
 */
void Zombie::attack(Plant* plant) {
    if (plant != nullptr && plant->isAlive()) {
        plant->takeDamage(damage);  // 对植物造成伤害
    }
}

/**
 * @brief 更新僵尸
 *
 * 主要职责：
 * 1. 向左移动（speed像素/帧）
 * 2. 检查是否到达左侧边界（游戏失败条件）
 */
void Zombie::update() {
    if (used) {
        // 向左移动
        x -= speed;

        // 僵尸到达左侧边界，游戏失败
        if (x < 56) {
            gameStatus = FAIL;
        }
    }
}

/**
 * @brief 播放僵尸动画
 *
 * 根据僵尸状态更新动画帧：
 * - 如果已死亡(exploded=true): 被土豆炸死，不播放动画，直接移除
 * - 如果已死亡: 播放死亡动画，20帧后移除
 * - 如果正在吃东西: 播放啃食动画（21帧循环）
 * - 否则: 播放走路动画（22帧循环）
 *
 * 设计理由：
 * - 被炸死的僵尸不需要播放死亡动画，直接移除更自然
 * - 吃东西动画和走路动画帧数不同，需要分别处理
 */
void Zombie::animate() {
    if (dead) {
        // 被炸死的僵尸不播放死亡动画，直接移除
        if (exploded) {
            used = false;
            killCount++;  // 增加击杀计数
            return;
        }

        // 普通死亡：播放死亡动画
        frameIndex++;
        if (frameIndex >= 20) {
            used = false;  // 动画结束，移除僵尸
            killCount++;   // 增加击杀计数
        }
    } else if (eating) {
        // 吃东西动画（21帧循环）
        frameIndex = (frameIndex + 1) % 21;
    } else {
        // 走路动画（22帧循环）
        frameIndex = (frameIndex + 1) % 22;
    }
}

/**
 * @brief 被土豆地雷炸死
 * @param damage 爆炸伤害值
 *
 * 处理被土豆地雷爆炸击杀的情况：
 * - 减少血量
 * - 如果血量<=0，标记为死亡、停止移动、设置exploded=true
 *
 * 设计理由：exploded标记用于区分"被子弹击杀"和"被炸死"
 * 被炸死的僵尸不需要播放死亡动画，更加真实
 */
void Zombie::explodeByPotato(int damage) {
    if (dead) return;  // 已经死了就不要再处理

    // 扣除血量
    blood -= damage;
    if (blood <= 0) {
        blood = 0;
        dead = true;           // 标记为死亡
        eating = false;        // 停止啃食
        speed = 0;             // 停止移动
        frameIndex = 0;        // 重置动画帧
        exploded = true;       // 标记为被炸死，不播放死亡动画
    }
}