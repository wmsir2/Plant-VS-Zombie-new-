#include"Plant.h"
#include"vector2.h"
#include"config.h"
#include"sunshineBall.h"
#include"Map.h"
#include<easyx.h>
#include <iostream>

// 减少植物的血量
void Plant::takeDamage(int damage) {
    if (blood > 0) {
        blood -= damage;
        if (blood <= 0) {
            blood = 0; // 防止负数
        }
    }
}

bool Plant::isAlive() const {
	return blood > 0;
}

int Plant::getBlood() const {
	return blood; // 返回当前植物的血量
}





