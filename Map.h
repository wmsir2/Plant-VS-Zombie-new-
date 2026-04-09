#pragma once
#include "Plant.h"

/**
 * @file Map.h
 * @brief 游戏地图数据结构
 *
 * 地图采用二维数组实现，每个单元格存储：
 * - 是否有植物
 * - 植物类型和状态
 * - 动画帧等
 *
 * 设计理由：将地图信息集中管理，便于批量更新和渲染
 */

/**
 * @brief 地图单元格结构
 *
 * 代表游戏中可种植植物的一块土地
 * 每个单元格可以拥有一株植物
 */
class MapCell {
public:
    int type;              /**< 单元格类型: 0=空地, 1+=植物类型编号 */
    float frameIndex;       /**< 动画帧索引（用于某些特效） */
    bool catched;           /**< 是否被僵尸咬过（暂未使用） */
    int deadTime;           /**< 死亡计时（暂未使用） */
    int timer;              /**< 通用计时器（用于植物状态切换等） */
    int x, y;             /**< 植物坐标（与Plant类中的x,y同步） */
    int shootTime;          /**< 射击计时（暂未使用） */
    int price;              /**< 价格（暂未使用） */
    int blood;              /**< 血量（暂未使用） */
    Plant* plant;           /**< 指向该位置植物的指针，nullptr表示空地 */
};