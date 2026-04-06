#include"Zombie.h"
#include"Plant.h"
#include"config.h"
void Zombie::attack(Plant* plant) {
	if (plant != nullptr && plant->isAlive()) {
		plant->takeDamage(damage);
	}
}

void Zombie::update() {
    if (used) {
        x -= speed;
        if (x < 56) {
            gameStatus = FAIL; // 如果僵尸到达左侧边界，游戏失败
        }
    }
}

void Zombie::animate() {
    if (dead) {
        frameIndex++;
        if (frameIndex >= 20) { // 僵尸死亡图片播放到最后一张
            used = false; // 僵尸不再使用
            killCount++;
            if (killCount == ZOMBIE_MAX) {
                gameStatus = WIN; // 如果杀死所有僵尸，游戏胜利
            }
        }
    }
    else if (eating) {
        frameIndex = (frameIndex + 1) % 21; // 啃食动画
    }
    else {
        frameIndex = (frameIndex + 1) % 22; // 行走动画
    }
};