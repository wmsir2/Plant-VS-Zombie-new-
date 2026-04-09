#pragma once
#include <iostream>
#include "config.h"
using namespace std;

/**
 * @brief 植物类型枚举
 *
 * 定义游戏中所有植物的种类：
 * - PEASHOOTER: 豌豆射手，发射豌豆攻击僵尸
 * - SUNFLOWER: 向日葵，生产阳光
 * - POTATO_MINE: 土豆地雷，潜伏后爆炸
 */
enum PlantType { PEASHOOTER = 0, SUNFLOWER = 1, POTATO_MINE = 2 };

/**
 * @brief 爆炸效果结构体
 *
 * 用于土豆地雷爆炸时的视觉效果管理：
 * - 支持同时显示多个爆炸（最多10个）
 * - 每帧动画结束后标记为未使用
 */
struct ExplosionEffect {
    int x, y;           // 爆炸中心坐标
    int frameIndex;     // 当前动画帧
    bool used;          // 是否正在使用
};

/**
 * @brief 植物基类
 *
 * 所有植物的父类，采用模板方法模式：
 * - 定义植物的公共属性（血量、位置、动画）
 * - 提供虚函数接口供子类实现（更新、绘制、获取类型）
 *
 * 设计理由：通过基类指针操作植物数组，实现多态（不同植物有不同行为）
 */
class Plant {
public:
    // 纯虚函数：子类必须实现
    virtual void update(double dt) = 0;      // 更新植物状态（射击、生产阳光等）
    virtual void draw() = 0;                 // 绘制植物
    virtual PlantType getType() const = 0;   // 获取植物类型

    // 获取坐标和网格位置（用于碰撞检测）
    virtual int getX() const = 0;
    virtual int getY() const = 0;
    virtual int getRow() const = 0;
    virtual int getCol() const = 0;

    // 植物可被啃食特性（土豆地雷在地下时不可被吃）
    virtual bool canBeEaten() const { return true; }

    // 死亡判定：血量<=0
    virtual bool isDead() const { return blood <= 0; }

    // 获取动画帧数（不同植物动画长度不同）
    virtual int getFrameCount() const { return 60; }

    /**
     * @brief 植物受伤
     * @param damage 伤害值
     *
     * 当僵尸啃食植物时调用，减少植物血量
     */
    void takeDamage(int damage) {
        if (blood > 0) {
            blood -= damage;
            if (blood <= 0) blood = 0;  // 血量不能为负
        }
    }

    // 存活判定
    bool isAlive() const { return blood > 0; }
    int getBlood() const { return blood; }

    /**
     * @brief 植物构造函数
     * @param initialBlood 初始血量
     * @param row 所在行 (0~2)
     * @param col 所在列 (0~8)
     *
     * 计算植物在屏幕上的实际像素坐标：
     * x = 256 + col * 81（每列宽度81像素）
     * y = 179 + row * 102 + 14（行高102像素，偏移14像素对齐）
     */
    Plant(int initialBlood, int row, int col) {
        frameIndex = 0;
        timer = 0;
        this->row = row;
        this->col = col;
        blood = initialBlood;
        price = 0;
        // 根据网格位置计算屏幕坐标
        x = 256 + col * 81;
        y = 179 + row * 102 + 14;
    }

    // 虚析构函数：确保子类对象正确析构
    virtual ~Plant() = default;

protected:
    int frameIndex;      // 动画帧索引
    int timer;           // 通用计时器（用于射击间隔、生产阳光等）
    int x, y;            // 屏幕坐标
    int row, col;        // 网格行列
    int blood;           // 血量
    int price;           // 价格（暂未使用）
};