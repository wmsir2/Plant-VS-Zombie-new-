#include "Peashooter.h"
#include "config.h"
#include <easyx.h>
#include "tool.h"
#include "Zombie.h"

/**
 * @file Peashooter.cpp
 * @brief 豌豆射手实现文件
 *
 * 包含豌豆射手的更新逻辑、绘制和射击机制
 */

// 外部引用：全局僵尸数组（在main.cpp中定义）
// 用于判断是否有僵尸需要攻击
extern Zombie zms[10];

/**
 * @struct bullet
 * @brief 子弹结构体
 *
 * 子弹从豌豆射手发射，向左飞行击中僵尸：
 * - x, y: 子弹位置
 * - row: 子弹所在行（用于判断击中哪个僵尸）
 * - speed: 移动速度
 * - blast: 是否处于爆炸动画状态
 * - frameIndex: 爆炸动画帧
 */
struct bullet {
    int x, y;          // 子弹坐标
    int row;           // 子弹所在行（用于碰撞检测）
    bool used;         // 是否被使用
    int speed;         // 移动速度（像素/帧）
    bool blast;        // 是否在爆炸状态
    int frameIndex;    // 爆炸动画帧
    int injury;        // 伤害值
};

// 外部引用：子弹数组和植物图片数组
extern struct bullet bullets[30];                      // 子弹数组（最多30颗）
extern IMAGE* imgPlant[ZHI_WU_COUNT][20];              // 植物动画帧图片

/**
 * @brief 豌豆射手构造函数
 * @param row 行号
 * @param col 列号
 *
 * 初始化豌豆射手：
 * - 设置初始血量（100）
 * - 重置射击和动画计时器
 */
Peashooter::Peashooter(int row, int col) : Plant(100, row, col), shootTimer(0), frameTimer(0) {
}

/**
 * @brief 更新豌豆射手
 * @param dt 时间步长
 *
 * 更新逻辑：
 * 1. 每5帧切换一次动画帧，实现动画播放效果
 * 2. 调用canShoot()检查是否需要射击
 * 3. 如果需要且射击冷却已满，发射子弹并重置冷却计时器
 */
void Peashooter::update(double dt) {
    // 更新动画帧
    frameTimer++;
    if (frameTimer >= FRAME_DELAY) {
        frameIndex = (frameIndex + 1) % getFrameCount();  // 循环播放
        frameTimer = 0;
    }

    // 检查是否需要射击
    if (canShoot()) {
        shootTimer++;
        if (shootTimer >= SHOOT_INTERVAL) {
            shoot();       // 发射子弹
            shootTimer = 0; // 重置冷却
        }
    } else {
        // 没有僵尸时，重置射击计时器（实现"看到僵尸立刻射击"的效果）
        shootTimer = 0;
    }
}

/**
 * @brief 绘制豌豆射手
 *
 * 从图片数组中获取当前帧的图片并绘制到屏幕
 * 使用putimagePNG确保透明背景正确显示
 */
void Peashooter::draw() {
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
 * @brief 检查是否可以射击
 * @return true 可以射击，false 不能射击
 *
 * 判断逻辑：
 * 遍历所有僵尸，如果满足以下条件返回true：
 * - 僵尸存活且未死亡
 * - 僵尸与豌豆射手在同一行
 * - 僵尸X坐标小于危险区域边界（说明僵尸已经进入攻击范围）
 *
 * 设计理由：只有当僵尸进入"危险区"时才射击，这个区域是屏幕内靠近左侧的部分
 */
bool Peashooter::canShoot() const {
    int dangerX = 900 - 60;  // 危险区域边界：屏幕右侧留60像素作为"安全区"

    // 遍历所有僵尸
    for (int i = 0; i < 10; i++) {
        if (zms[i].used && !zms[i].dead && zms[i].row == row) {
            // 检查僵尸是否进入危险区域
            if (zms[i].x < dangerX) {
                return true;  // 发现僵尸，可以射击
            }
        }
    }
    return false;  // 没有僵尸，继续等待
}

/**
 * @brief 发射豌豆
 *
 * 在子弹数组中找一个空闲位置，创建一个新子弹：
 * - row: 与豌豆射手相同（只在同行有威胁时才会调用shoot，所以同行）
 * - speed: 4像素/帧，向左飞行
 * - x: 植物右侧（x + 50，让子弹从豌豆出口位置发出）
 * - y: 略微偏移（y + 5，对齐豌豆出口高度）
 */
void Peashooter::shoot() {
    int bulletMax = 30;  // 子弹数组大小限制

    // 找一个空闲的子弹槽位
    int k;
    for (k = 0; k < bulletMax && bullets[k].used; k++);
    if (k >= bulletMax) return;  // 子弹数组满了，无法发射

    // 初始化子弹属性
    bullets[k].used = true;
    bullets[k].row = row;
    bullets[k].speed = 4;      // 向左飞行速度
    bullets[k].blast = false;  // 普通子弹，非爆炸状态
    bullets[k].frameIndex = 0;
    bullets[k].x = x + 50;      // 从豌豆射手右侧发出
    bullets[k].y = y + 5;       // 垂直方向偏移，对齐豌豆出口
}