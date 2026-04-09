#include "PotatoMine.h"
#include "config.h"
#include <easyx.h>
#include "tool.h"
#include "Zombie.h"

/**
 * @file PotatoMine.cpp
 * @brief 土豆地雷实现文件
 *
 * 包含土豆地雷的状态转换和爆炸逻辑
 */

// 外部引用：全局僵尸数组和爆炸效果数组
extern Zombie zms[10];                                    // 僵尸数组
extern struct ExplosionEffect explosions[10];             // 爆炸效果数组（最多10个）
extern IMAGE imgPotatoUnderground;                        // 地下状态图片
extern IMAGE imgPotatoMine;                               // 地面状态图片

/**
 * @brief 判断土豆地雷是否死亡
 * @return true 死亡，false 存活
 *
 * 死亡条件：
 * - 爆炸动画播放结束（EXPLODING状态且timer > 30）
 * - 血量耗尽（被啃死，不会触发爆炸）
 */
bool PotatoMine::isDead() const {
    if (state == PotatoMineState::EXPLODING && timer > EXPLODE_TIME) {
        return true;
    }
    if (blood <= 0) {
        return true;
    }
    return false;
}

/**
 * @brief 判断土豆地雷是否存活
 * @return true 存活，false 已摧毁
 *
 * 存活条件：不是EXPLODING状态且血量 > 0
 * 设计理由：EXPLODING状态虽然还没被移除，但已经不能被攻击或使用了
 */
bool PotatoMine::isAlive() const {
    if (state == PotatoMineState::EXPLODING) {
        return false;
    }
    return blood > 0;
}

/**
 * @brief 判断土豆地雷是否可被吃
 * @return true 可被吃，false 不可被吃
 *
 * 只有HIDDEN（地下）状态可以被僵尸吃
 * 设计理由：当地雷升出地面后，僵尸就不能吃了，只能触发爆炸
 */
bool PotatoMine::canBeEaten() const {
    return state == PotatoMineState::HIDDEN;
}

/**
 * @brief 土豆地雷受到伤害
 * @param damage 伤害值
 *
 * 处理逻辑：
 * - HIDDEN状态：正常扣血，但不会爆炸
 * - RISING状态：扣血，如果血量<=0则立即爆炸
 * - READY/EXPLODING状态：不吃伤害（免疫）
 *
 * 设计理由：土豆地雷在地下时很脆弱，被啃几下就死
 * 但一旦升起成为READY状态，就只能被炸掉了
 */
void PotatoMine::takeDamage(int damage) {
    if (state == PotatoMineState::HIDDEN) {
        // 地下状态：正常扣血，被啃死不会爆炸
        blood -= damage;
        if (blood <= 0) {
            blood = 0;
            // 血量耗尽，直接标记为死亡（不在此处delete，由调用方处理）
        }
    } else if (state == PotatoMineState::RISING) {
        // 升起状态：扣血，血量清零时爆炸
        blood -= damage;
        if (blood <= 0) {
            blood = 0;
            triggerExplosion();  // 提前爆炸
        }
    }
}

/**
 * @brief 土豆地雷构造函数
 * @param row 行号
 * @param col 列号
 *
 * 初始状态为HIDDEN，血量100
 */
PotatoMine::PotatoMine(int row, int col) : Plant(100, row, col), state(PotatoMineState::HIDDEN) {
}

/**
 * @brief 更新土豆地雷状态
 * @param dt 时间步长
 *
 * 状态机逻辑：
 * - HIDDEN: 180帧后进入RISING
 * - RISING: 240帧后进入READY，期间持续检测僵尸
 * - READY: 持续检测僵尸，有僵尸进入范围就爆炸
 * - EXPLODING: 120帧后标记为死亡
 */
void PotatoMine::update(double dt) {
    switch (state) {
    case PotatoMineState::HIDDEN:
        // 地下状态：等待升起
        timer++;
        if (timer > HIDDEN_TIME) {
            // HIDDEN_TIME帧后进入升起状态
            state = PotatoMineState::RISING;
            timer = 0;
            checkZombieInRange();  // 升起时立即检测
        }
        break;

    case PotatoMineState::RISING:
        // 升起状态：计时，同时检测僵尸
        timer++;
        if (timer > RISING_TIME) {
            // 升起完成，进入就绪状态
            state = PotatoMineState::READY;
            timer = 0;
            checkZombieInRange();  // 完成后立即检测
        } else {
            // 升起过程中也持续检测（僵尸可能在升起时就靠近了）
            checkZombieInRange();
        }
        break;

    case PotatoMineState::READY:
        // 就绪状态：持续检测僵尸
        checkZombieInRange();
        break;

    case PotatoMineState::EXPLODING:
        // 爆炸状态：等待爆炸结束
        timer++;
        if (timer > EXPLODE_TIME) {
            // 爆炸动画结束，标记为死亡
            blood = 0;
        }
        break;
    }
}

/**
 * @brief 绘制土豆地雷
 *
 * 根据状态绘制不同图片：
 * - HIDDEN: 绘制地下图片（土豆在地底下，看不到）
 * - RISING/READY: 绘制地面图片（土豆已经冒出来了）
 * - EXPLODING: 不绘制，使用独立的爆炸效果
 */
void PotatoMine::draw() {
    if (state == PotatoMineState::HIDDEN) {
        // 地下状态：绘制地下图片（玩家可以看到地上有个鼓包）
        if (imgPotatoUnderground.getwidth() > 0) {
            putimagePNG(x, y, &imgPotatoUnderground);
        }
    } else if (state == PotatoMineState::READY || state == PotatoMineState::RISING) {
        // 升起或就绪：绘制地面图片
        if (imgPotatoMine.getwidth() > 0) {
            putimagePNG(x, y, &imgPotatoMine);
        }
    }
}

/**
 * @brief 检测范围内是否有僵尸
 *
 * 遍历同列所有僵尸，检查是否在爆炸范围内
 * 如果僵尸的身体前端（x+80）进入爆炸范围（±60像素），触发爆炸
 *
 * 设计理由：使用僵尸的"嘴"位置（身体前端）来判断是否接触
 * 这样可以确保僵尸确实走到了土豆地雷上面
 */
void PotatoMine::checkZombieInRange() {
    for (int i = 0; i < 10; i++) {
        if (zms[i].used && !zms[i].dead && zms[i].row == row) {
            // 计算僵尸的"嘴"位置（身体前端）
            int zombieMouthX = zms[i].x + 80;
            // 检查是否在爆炸范围内
            if (abs(zombieMouthX - x) <= BLAST_RANGE) {
                triggerExplosion();  // 触发爆炸
                break;              // 爆炸后不再检测
            }
        }
    }
}

/**
 * @brief 触发爆炸
 *
 * 执行以下操作：
 * 1. 切换到EXPLODING状态
 * 2. 重置timer
 * 3. 对范围内所有同列僵尸造成150点伤害
 * 4. 创建爆炸效果
 *
 * 设计理由：爆炸是瞬间完成的，但伤害判定需要考虑范围内所有僵尸
 */
void PotatoMine::triggerExplosion() {
    state = PotatoMineState::EXPLODING;
    timer = 0;

    // 对范围内所有僵尸造成伤害
    for (int k = 0; k < 10; k++) {
        if (zms[k].used && !zms[k].dead && zms[k].row == row) {
            int zmX = zms[k].x + 80;  // 僵尸嘴位置
            // 检查是否在爆炸范围内
            if (abs(zmX - x) <= BLAST_RANGE) {
                zms[k].explodeByPotato(damage);  // 调用僵尸的爆炸死亡方法
            }
        }
    }

    // 查找空闲爆炸效果槽位，创建爆炸动画
    for (int e = 0; e < 10; e++) {
        if (!explosions[e].used) {
            explosions[e].x = x;
            explosions[e].y = y;
            explosions[e].frameIndex = 0;
            explosions[e].used = true;
            break;  // 只创建一个爆炸效果
        }
    }
}