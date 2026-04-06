#pragma once
#include<iostream>
#include"vector2.h"
class sunshineBall {
public:
	int x, y;		//阳光球在飘落过程中的坐标位置(X不变)
	int frameIndex;	//当前显示的图片帧的序号
	int destY;		//飘落的目标位置的y坐标
	bool used;		//判断阳光池的阳光是否正在使用
	int timer;

	float xoff;		//收集阳光,阳光飞往左上角的x坐标
	float yoff;		//收集阳光,阳光飞往左上角的y坐标

	float t;		//贝塞尔曲线的时间点 0-1
	vector2 p1, p2, p3, p4;
	vector2 pCur;	//当前时刻阳光的位置
	float speed;
	int status;
};