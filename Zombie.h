#pragma once
#include"Plant.h"
enum ZombieState {
	WALKING,  // 正在行走
	EATING,   // 正在啃食植物
	DEAD      // 已死亡
};
class Zombie {		//僵尸
private:
	int damage;  // 僵尸造成的伤害
public:
	void attack(Plant* plant);
	int x, y;			//僵尸坐标
	int frameIndex;		//僵尸行走动画的序列帧序号
	bool used;			//僵尸是否出现
	int speed;
	int row;			//僵尸的行数
	int col;			//僵尸的列数
	int blood;
	bool dead;
	bool eating;		//正在吃植物
	int eatingTime;		//僵尸啃食计时器
	ZombieState state; // 当前僵尸的状态
	Plant* targetPlant;      // 指向当前目标植物的指针
	Zombie() : used(false), x(0), row(0), y(0), speed(0), blood(100), dead(false), eating(false), frameIndex(0) {}
    void update();
	void animate();
};