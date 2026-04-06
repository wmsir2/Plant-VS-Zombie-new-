#pragma once
#include"config.h"
class Map {
public:
	int type;		//0:没有植物 1:第一种植物
	float frameIndex;	//序列帧的序号
	bool catched;		//是否被僵尸捕获
	int deadTime;
	int timer;
	int x, y;			//植物的坐标
	int shootTime;		//植物子弹发射间隔
	int price;			//植物种植价格
	int blood;			//植物血量
	Plant* plant;   // 指向植物对象的指针
};