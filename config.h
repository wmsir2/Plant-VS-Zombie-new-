#pragma once
//该头文件用于放置宏定义
#define WIN_WIDTH	900	//定义游戏窗口宽度
#define WIN_HEIGHT	600	//定义游戏窗口高度
#define MAP_ROW  3		//定义草坪可种植的行数
#define MAP_COL  9		//定义草坪可种植的列数
#define ballMax  10		//定义阳光最大数量
#define ZOMBIE_MAX  10		//定义僵尸的最多数量
// 定义动画帧总数
#define TOTAL_FRAMES 60

// 定义僵尸啃食植物的动画帧数
#define ZOMBIE_EATING_FRAMES 60

// 定义僵尸行走的动画帧数
#define ZOMBIE_WALKING_FRAMES 60

// 定义僵尸行走的动画帧数
#define ZOMBIE_DEAD_FRAMES 60

enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };		//阳光的状态

enum {WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};	//定义一个枚举

enum { GOING, WIN, FAIL };		//定义游戏状态

static int killCount;	//已经杀掉的僵尸个数
static int zmCount;	//已经出现的僵尸个数
static int gameStatus;
