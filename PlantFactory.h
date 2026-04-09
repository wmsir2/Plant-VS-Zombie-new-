#pragma once
#include "Plant.h"
#include "Peashooter.h"
#include "Sunflower.h"
#include "PotatoMine.h"

/**
 * @file PlantFactory.h
 * @brief 植物工厂类定义
 *
 * 植物工厂用于统一创建植物对象：
 * - 隐藏具体植物的创建细节
 * - 集中管理植物的实例化
 * - 提供多种创建接口（按类型或按序号）
 *
 * 设计理由：工厂模式解耦了植物创建和使用的关系
 * 当需要添加新植物时，只需修改工厂，不需要修改使用植物的代码
 */

/**
 * @class PlantFactory
 * @brief 植物工厂类
 *
 * 提供静态方法用于创建不同类型的植物对象
 */
class PlantFactory {
public:
    /**
     * @brief 根据PlantType创建植物
     * @param type 植物类型（PEASHOOTER/SUNFLOWER/POTATO_MINE）
     * @param row 行号 (0~2)
     * @param col 列号 (0~8)
     * @return 植物指针，创建失败返回nullptr
     *
     * 使用方法：
     * Plant* p = PlantFactory::createPlant(PEASHOOTER, row, col);
     */
    static Plant* createPlant(PlantType type, int row, int col) {
        switch (type) {
        case PEASHOOTER:
            return new Peashooter(row, col);
        case SUNFLOWER:
            return new Sunflower(row, col);
        case POTATO_MINE:
            return new PotatoMine(row, col);
        default:
            return nullptr;  // 未知类型
        }
    }

    /**
     * @brief 根据旧序号创建植物（兼容旧代码）
     * @param legacyType 植物序号 (1=豌豆射手, 2=向日葵, 3=土豆地雷)
     * @param row 行号 (0~2)
     * @param col 列号 (0~8)
     * @return 植物指针，创建失败返回nullptr
     *
     * 使用方法：
     * Plant* p = PlantFactory::createPlant(1, row, col);  // 创建豌豆射手
     *
     * 注意：序号从1开始，与PlantType枚举的0开始不同
     * 这是因为UI中用1、2、3来表示植物
     */
    static Plant* createPlant(int legacyType, int row, int col) {
        switch (legacyType) {
        case 1:
            return new Peashooter(row, col);
        case 2:
            return new Sunflower(row, col);
        case 3:
            return new PotatoMine(row, col);
        default:
            return nullptr;  // 未知序号
        }
    }
};