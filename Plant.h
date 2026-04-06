#pragma once
#include<iostream>
#include"config.h"
using namespace std;
class Plant {
public:
	virtual void produceSun() {} // 添加虚函数声明

	Plant() : blood(100), catched(false), frameIndex(0), timer(0), deadTime(0),PlantName("空") {}

	// 带参数的构造函数，允许设置血量、行和列
	Plant(int initialBlood, int row, int col)
		: blood(initialBlood), catched(false), frameIndex(0), timer(0), deadTime(0), row(row), col(col) {}
	bool isAlive() const;

	// 减少植物的血量
	void takeDamage(int damage);

	int getBlood() const;// 返回当前植物的血量

	string PlantName;	//植物名称
	int type;		//0:没有植物 1:第一种植物
	int frameIndex;	//序列帧的序号
	bool catched;		//是否被僵尸捕获
	int deadTime;
	int timer;
	int x, y;			//植物的坐标
	int row;
	int col;			//植物所在的行和列
	int shootTime;		//植物子弹发射间隔
	int price;			//植物种植价格
	int blood;			//植物血量

};

class Sunflower : public Plant {
public:
	Sunflower(int row, int col) : Plant(100,row,col) {	//构造函数 设置血量 行 列
		type = 2;
		PlantName = "Sunflower";  // 设置植物名称
		cout << "成功种植：" << PlantName << " 植物的行为:" << row << " 列为:" << col << endl;

	}
};

class Peashooter : public Plant {
public:
	Peashooter(int row, int col) : Plant(100,row,col) {  // 构造函数 设置血量 行 列
		type = 1;  // 假设1代表豌豆射手类型
		PlantName = "Peashooter";  // 设置植物名称
		cout << "成功种植：" << PlantName << " 植物的行为:" << row << " 列为:" << col << endl;

	}

	void shoot();
};
