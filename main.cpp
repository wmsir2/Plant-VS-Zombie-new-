#include<stdio.h>
#include<windows.h>
#include<iostream>
#include<string>
#include<graphics.h>  // EasyX 图形库头文件
#include<math.h>
#include<chrono>
#include<time.h>
#include"tool.h"	// 声明 putimagePNG、getDelay 等函数
#include"vector2.h"
#include"Plant.h"
#include"sunshineBall.h"
#include"Zombie.h"
#include"config.h"
#include"Map.h"
#include<mmsystem.h>	// 多媒体播放
#pragma comment(lib,"winmm.lib")
using namespace std;

//enum {WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};	// 植物类型枚举（豌豆射手、向日葵、植物总数量）
//enum{GOING,WIN,FAIL};		// 游戏状态（进行中 / 胜利 / 失败）
//int killCount;	// 已击杀的僵尸数量
//int zmCount;	// 已生成的僵尸数量
//int gameStatus;  // 当前游戏状态

//图片资源
IMAGE imgBg;	// 游戏背景图片
IMAGE imgBar;	// 顶部功能栏图片
IMAGE imgShovelSlot; // 铲子槽位图片（UI框）
IMAGE imgShovel;  // 铲子图标图片
IMAGE imgCards[ZHI_WU_COUNT];	// 植物卡片图片数组（卡槽中的植物卡片）

// 植物动画帧二维数组
// 第一维：植物类型
// 第二维：动画帧序号（最多20帧）
// 存储 IMAGE* 指针，便于动态加载不同数量的动画帧
IMAGE* imgPlant[ZHI_WU_COUNT][20];	

int curX, curY;	// 当前选中的植物跟随鼠标移动时的位置（鼠标坐标）
int curZhiWu;	// 当前选择的植物类型 0：未选择植物 1~n：已选择对应植物

Map map[MAP_ROW][MAP_COL];	//游戏地图二维数组（每个格子存放植物、状态等信息）

//enum{SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};		// 阳光状态枚举

class sunshineBall balls[10];// 阳光球对象数组（同时存在的阳光数量上限）
IMAGE imgSunshineBall[29];	// 阳光动画图片（共29帧）
int sunshine;  // 当前玩家拥有的阳光数量（资源值）

class Zombie zms[10];   // 僵尸对象数组（场上最多10个僵尸）
IMAGE imgZM[22];        // 僵尸行走动画（22帧）
IMAGE imgZMDead[20];	// 僵尸死亡动画帧
IMAGE imgZMEat[21];		// 僵尸啃食植物动画帧
IMAGE imgZmStand[11];	// 僵尸站立等待动画帧

// 子弹数据结构
struct bullet {
public:
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast; // 是否发生爆炸
	int frameIndex;	// 子弹爆炸动画帧索引
	int injury;		// 子弹伤害值
};
struct bullet bullets[30];
IMAGE imgBulletNormal;		// 普通状态下的子弹图片
IMAGE imgBullBlast[4];		// 爆炸状态下的子弹图片数组

bool fileExist(const char* name) {	
	FILE* fp = fopen(name, "r");	// 定义一个指向 FILE 的指针 fp，以只读模式 "r" 打开名为 name 的文件
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}
void gameInit() {	//游戏初始化函数

	//初始化图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT,1);

	//加载背景图片

	// 加载背景图片
	// 中文路径需要修改为多字节字符串格式

	loadimage(&imgBg, "res/白天.jpg");
	loadimage(&imgBar, "res/bar5.png");
	loadimage(&imgShovelSlot, "res/铲子槽.png");
	loadimage(&imgShovel, "res/铲子.png");

	memset(imgPlant, 0, sizeof(imgPlant));	// 将 imgPlant 所占内存全部初始化为 0
	memset(map, 0, sizeof(map));			// 将地图数据全部初始化为 0

	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;

	// 初始化植物卡片
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		// 拼接植物卡片文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png",i+1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i ,j+1);
			// 判断图片文件是否存在
			if (fileExist(name)) {
				imgPlant[i][j] = new IMAGE;
				loadimage(imgPlant[i][j], name);
			}
			else {
				break;
			}
		}
	}

	 
	curZhiWu = 0;	// 当前是否选中植物卡片的标志，同时表示植物编号
	sunshine = 50;	// 设置初始阳光值


	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	// 初始化随机数种子
	srand(time(NULL));

	// 设置游戏文字显示位置

	// 设置字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;	// 抗锯齿效果
	settextstyle(&f);		// 设置字体
	setbkmode(TRANSPARENT);	// 设置背景透明
	setcolor(BLACK);		// 设置文字颜色

	// 初始化僵尸数据
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);		// 拼接资源文件名
		loadimage(&imgZM[i], name);			// 加载 22 帧僵尸动画
	}

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	// 初始化子弹爆炸动画帧图片数组
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	// 缩放爆炸图片
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;		
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
			imgBullBlast[3].getwidth() * k,	
			imgBullBlast[3].getheight() * k, true);
	}

	for (int i = 0; i < 20; i++) {		// 初始化僵尸死亡动画
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);	
		loadimage(&imgZMDead[i], name);
	}

	for (int i = 0; i < 21; i++) {		// 初始化僵尸吃植物动画
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);	
		loadimage(&imgZMEat[i], name);
	}

	for (int i = 0; i < 11; i++) {		// 初始化僵尸站立动画
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);	
		loadimage(&imgZmStand[i], name);
	}
}

void drawZombie() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			IMAGE* img = nullptr;
			// 根据僵尸状态选择对应动画帧
			if (zms[i].dead) img = &imgZMDead[zms[i].frameIndex];    // 使用 & 取元素地址
			else if (zms[i].eating) img = &imgZMEat[zms[i].frameIndex];
			else img = &imgZM[zms[i].frameIndex];
			// 绘制僵尸（底部对齐地面）
			putimagePNG(
				zms[i].x,
				zms[i].y - img->getheight(),
				img
			);
		}
	}
}



void drawBullets() {		// 渲染植物子弹
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletsMax; i++) {
		if (bullets[i].used) {
			// 子弹爆炸动画
			if (bullets[i].blast) {
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			// 普通飞行子弹
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
}

// 绘制阳光
void drawSunshines() {
	// 渲染阳光
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			// 显示当前动画帧图片
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];	
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);	// 按指定格式将阳光数值转换为字符串
	if (sunshine < 100) {
		outtextxy(282, 67, scoreText);
	}
	else {
		outtextxy(276, 67, scoreText);		// 在指定位置显示当前阳光数值
	}
}

void drawCards() {
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 338 + i * 65;
		int y = 5;
		putimage(x, y, &imgCards[i]);
	}
}


void drawPlant() {
	for (int i = 0; i < MAP_ROW; i++) {
		for (int j = 0; j < MAP_COL; j++) {
			if (map[i][j].plant !=nullptr&&map[i][j].plant->type > 0) {
				int PlantType = map[i][j].plant->type - 1;
				int index = map[i][j].plant->frameIndex;

				// 只绘制存在的植物
				Plant* plant = map[i][j].plant;
				if (plant != nullptr) {
					putimagePNG(map[i][j].plant->x, map[i][j].plant->y, imgPlant[PlantType][index]); // 渲染植物
				}
			}
		}
	}
}

void drawShovel() {
	int x = 838;
	int y = 5;
	putimagePNG(x, y, &imgShovelSlot);
	putimagePNG(x, y, &imgShovel);
}

void updateWindow() {		// 刷新窗口函数
	BeginBatchDraw();	// 开始批量绘制（双缓冲开始）
	cleardevice();     // 清空绘图窗口
	putimage(0, 0, &imgBg);
	//putimage(250, 0, &imgBar);
	putimagePNG(250, 0, &imgBar);

	drawCards();
	drawShovel();
	drawPlant();
	// 渲染拖动选中的植物
	if (curZhiWu > 0) {
		IMAGE* img = imgPlant[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getwidth() / 2, img);
	}

	drawSunshines();

	// 渲染僵尸
	drawZombie();

	// 渲染植物子弹
	drawBullets();

	EndBatchDraw();	// 结束批量绘制（双缓冲结束）
}

void collectSunshine(ExMessage* msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();	// 获取阳光的宽度
	int h = imgSunshineBall[0].getheight();	// 获取阳光的高度
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].pCur.x;
			int y = balls[i].pCur.y;
			// 判断鼠标是否点击到阳光
			if (msg->x > x && msg->x < x + w &&
				msg->y > y && msg->y < y + h) {

				balls[i].status = SUNSHINE_COLLECT;
				//sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);	// 播放收集阳光音效
				//PlaySound("res/sunshine.wav", NULL,SND_FILENAME | SND_ASYNC);
				// 计算阳光飞向太阳槽的偏移量（旧方案，已注释）
				//float destY = 0;	// 目标位置Y
				//float destX = 262;
				//float angle = atan((y - destY) / (x - destX));
				//balls[i].xoff = 4 * cos(angle);
				//balls[i].yoff = 4 * sin(angle);
				balls[i].p1 = balls[i].pCur;	// p1 为起点
				balls[i].p4 = vector2(262, 0);	// p4 为终点（太阳槽位置）
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i].p4);	// 计算距离
				float off = 8;
				balls[i].speed = 1.0 / (distance /off);
				break;
			}
		}
	}
}

void userClick() {
	ExMessage msg;
	static int status = 0;		// 判断是否选择成功
	int isPlantArea = 0; // 判断鼠标当前位置是否在植物卡片区域
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {	// 鼠标左键按下
			if (msg.message == WM_LBUTTONDOWN && !isPlantArea && status == 0) { // 如果点击位置不在植物卡区域，并且未选择植物，则优先检测收集阳光
				collectSunshine(&msg);
			}
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {	// 判断是否点击植物卡片
				int index = (msg.x - 338) / 65;	// 计算点击的是第几个植物卡
				if (index == 0 && sunshine >= 100) {
					status = 1;						// 表示选择成功
					curZhiWu = index + 1;			// 设置当前植物
					isPlantArea = 1; // 位于植物卡区域
				}
				else if (index == 1 && sunshine >= 50) {
					status = 1;						
					curZhiWu = index + 1;			
					isPlantArea = 1; 
				}
			}
			else if (curZhiWu > 0 && (msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489)) {	
				curZhiWu = 0; 
				status = 0;
			}
			else if(msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489) {
				isPlantArea = 0;		// 位于空白区域
				//collectSunshine(&msg);
			}

		}
		else if (msg.message == WM_MOUSEMOVE) {	// 鼠标移动
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {	// 鼠标左键释放
			if (msg.x > 338 && msg.x < 338 + 65 * curZhiWu && msg.y < 96) {	// 在卡片区域释放鼠标，重新选择植物
				int index = (msg.x - 338) / 65;	// 计算选择的植物卡
				status = 1;						// 选择成功
				curZhiWu = index + 1;			
				isPlantArea = 1; 
			}
			// 在非法区域释放，取消选择
			else if (curZhiWu > 0 && (msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489)) {		//����ڷǷ������ɿ���������ѡ��
				curZhiWu = 0;
				status = 0;
				isPlantArea = 0;
			}
			// 在地图有效区域释放鼠标 —— 执行种植
			else if (msg.x > 256 && msg.x < 1000 && msg.y > 179 && msg.y < 489) {	
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				// 如果当前位置没有植物并且已经选择植物
				if (map[row][col].plant == nullptr && status == 1) {		
					// 根据植物类型创建植物对象
					if (curZhiWu == 1) {
						if (sunshine >= 100) {
							sunshine -= 100;
							map[row][col].plant = new Peashooter(row, col);// 创建豌豆射手
						}
					}
					else if (curZhiWu == 2) {
						if (sunshine >= 50) {
							sunshine -= 50;
							map[row][col].plant = new Sunflower(row, col); // 创建向日葵
						}
					}
					// 初始化植物属性
					if (map[row][col].plant != nullptr) {
						map[row][col].plant->type = curZhiWu;
						map[row][col].plant->frameIndex = 0;
						map[row][col].plant->shootTime = 0;

						map[row][col].plant->x = 256 + col * 81;
						map[row][col].plant->y = 179 + row * 102 + 14;
						// 重置选择状态
						curZhiWu = 0;
						status = 0;
					}
				}
				else if (map[row][col].plant != nullptr && map[row][col].plant->type != 0) { // 如果格子已有植物，取消当前选择
					curZhiWu = 0;
					status = 0;
				}

			}
		}
	}
}

void createSunshine() {
	static int count = 0;
	static int fre = 200;	// 第一次生成阳光需要的帧数
	count++;
	if (count >= fre) {		// 第二次开始每 fre 帧生成一个阳光
		fre = 300 + rand() % 200;	// 300~500帧随机生成
		count = 0;

		// 找一个未使用的阳光对象
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;		// 动画计时器归零
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		// 起始位置（天空随机位置）
		balls[i].p1 = vector2(260 -112 + rand() % (900 - 320 + 112), 60);
		// 终点位置（落到草坪某一行）
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		// 控制下落速度（t 的增长速度）
		balls[i].speed = 1.0 / (distance / off);
	}
	// 向日葵生产阳光
	for (int i = 0; i < MAP_ROW; i++) {
		for (int j = 0; j < MAP_COL; j++) {
			if (map[i][j].plant != nullptr && map[i][j].plant->type == XIANG_RI_KUI + 1) {
				map[i][j].plant->produceSun();
			}
		}
	}


	//向日葵生成阳光
	//for (int i = 0; i < MAP_ROW; i++) {
	//	for (int j = 0; j < MAP_COL; j++) {
	//         // 如果该格子存在植物，并且是向日葵
	//		if (map[i][j].plant!=nullptr&&map[i][j].plant->type == XIANG_RI_KUI + 1) {
	//			map[i][j].plant->timer++;
	//			if (map[i][j].plant->timer > 600) {		
	//				map[i][j].plant->timer = 0;

	//				int k;
	//				for (k = 0; k < ballMax&&balls[k].used; k++);
	//				if (k >= ballMax) return;

	//				balls[k].used = true;
	//				balls[k].p1 = vector2(map[i][j].plant->x, map[i][j].plant->y);
	//				int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);	// 左右随机偏移距离（随机向左或向右）
	//				balls[k].p4 = vector2(map[i][j].plant->x + w,
	//					map[i][j].plant->y + imgPlant[XIANG_RI_KUI][0]->getheight() -
	//					imgSunshineBall->getheight());
	//				balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 200);
	//				balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 200);
	//				balls[k].status = SUNSHINE_PRODUCT;
	//				balls[k].speed = 0.02;
	//				balls[k].t = 0;
	//			}
	//		}
	//	}
	//}
}

void updateSunshine() {
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {	// 只处理正在使用的阳光对象
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;	// 更新动画帧（阳光旋转动画）
			// 1. 天空掉落阳光
			if (balls[i].status == SUNSHINE_DOWN) {
				class sunshineBall* sun = &balls[i];
				sun->t += sun->speed * 0.6;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
					sun->t = 0;
					sun->timer = 0;
				}
			}
			// 2. 地面停留状态
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 300) {		// 开始闪烁（提示即将消失）

					if (balls[i].timer > 400) {		// 地面存在超时 → 消失
						balls[i].used = false;
						balls[i].timer = 0;
					}
				}
			}
			// 3. 被玩家收集（飞向UI）
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) {
					sun->used = false;
					sunshine += 25;		// 阳光 +25
					sun->t = 0;
				}
			}
			// 4. 向日葵生产阳光（贝塞尔曲线）
			else if(balls[i].status == SUNSHINE_PRODUCT) {
				class sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) {
					sun->status = SUNSHINE_GROUND;
					sun->t = 0;
					sun->timer = 0;
				}
			}
			
		}
	}
}

void createZombie() {
	// 僵尸数量达到上限，停止生成
	if (zmCount >= ZOMBIE_MAX) {
		return;
	}

	static int zmFre = 200;  // 僵尸生成频率（帧）
	static int count = 0;    // 帧计数器
	count++;
	// 到达生成时间
	if (count > zmFre) {
		count = 0;
		zmFre = 1000 + rand() % 500; // 下一次生成时间（1000~1500帧随机）

		for (int i = 0; i < ZOMBIE_MAX; i++) { // 在僵尸池中寻找未使用的位置
			if (!zms[i].used) { // 找到空闲僵尸
				zms[i].used = true;  
				zms[i].x = WIN_WIDTH; // 从屏幕右侧生成
				zms[i].row = rand() % MAP_ROW; // 随机行
				zms[i].y = 172 + (1 + zms[i].row) * 100;
				// 基础属性
				zms[i].speed = 1;
				zms[i].blood = 100;
				zms[i].dead = false;
				zmCount++;
				cout << "在第 "  << zms[i].row << " 行成功生成一个僵尸" << endl;
				break; 
			}
		}
	}
}

void updateZombie() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
	count++;
	if (count > 4) {		// 僵尸移动速度控制（每5帧移动一次）
		count = 0;
		// 更新僵尸位置
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				// 僵尸进入房子，游戏失败
				if (zms[i].x < 56) {
					//printf("GAME OVER\n");
					//MessageBox(NULL, "over", "over", 0);		// 游戏结束提示
					//exit(0);		// 退出游戏
					gameStatus = FAIL;
				}
			}
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 > 5) {		// 僵尸动画播放速度控制（每5帧刷新一次）
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {		// 更新僵尸动画帧
			if (zms[i].used) {
				if (zms[i].dead) {			// 僵尸死亡动画
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {	// 死亡动画播放到最后一帧
						zms[i].used = false;
						killCount++;
						// 所有僵尸被消灭，游戏胜利
						if (killCount == ZOMBIE_MAX) {
							gameStatus = WIN;
						}
					}
				}
				else if (zms[i].eating) {
					// 僵尸啃食植物动画
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else {		
					// 正常行走动画
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}

}

void shoot() {
	int lines[3] = { 0 };
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - 60;	// 当僵尸进入该范围内，植物开始发射子弹的距离
	for (int i = 0; i < zmCount; i++) {	// 遍历所有僵尸
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1; // 标记该行有僵尸
		}
	}
	// 遍历地图
	for (int i = 0; i < MAP_ROW; i++) {
		for (int j = 0; j < MAP_COL; j++) {
			if (map[i][j].plant!=nullptr&&map[i][j].plant->type == WAN_DOU + 1 && lines[i]) {	// 如果当前位置有植物，并且是豌豆射手，并且该行有僵尸
				//static int count2 = 0;
				//count2++;
				map[i][j].plant->shootTime++;		// 每个植物维护自己的发射计时器
				if (map[i][j].plant->shootTime > 80) {	// 达到发射间隔
					map[i][j].plant->shootTime = 0;

					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);// 找到一个未使用的子弹
						if (k < bulletMax) {	// 找到未使用子弹
							bullets[k].used = true;
							bullets[k].row = i;
							bullets[k].speed = 4;

							// 初始化子弹爆炸状态
							bullets[k].blast = false;
							bullets[k].frameIndex = 0;
							// 计算植物位置
							int zwX = 256 + j * 81;	
							int zwY = 179 + i * 102 + 14;
							// 设置子弹初始位置
							bullets[k].x = zwX + imgPlant[map[i][j].plant->type - 1][0]->getwidth() - 10;	//�ӵ�����
							bullets[k].y = zwY + 5;
							// 当前行已发射子弹，避免重复触发
							lines[i] = 0;
						}
				}
			}
		}
	}
}

void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {	//����ӵ��ɳ���Χ
				bullets[i].used = false;
			}
			if (bullets[i].blast) {		//����ӵ��뽩ʬ������ײ
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;
				}
			}
		}
	}
}

void checkBullet_to_ZM() {		//����ӵ���ײ��ʬ
	int bCount = sizeof(bullets) / sizeof(bullets[0]);	//ֲ���ӵ�������
	int zCount = sizeof(zms) / sizeof(zms[0]);		//��ʬ������
	for (int i = 0; i < bCount; i++) {
		if (bullets[i].used == false || bullets[i].blast)continue;	//�������δʹ�õ��ӵ����ѱ�ը���ӵ�
		for (int k = 0; k < zCount; k++) {
			if (zms[k].used == false)continue;	//�������δ���ֵĽ�ʬ
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2) {		//��ʬ��ֲ���ӵ���ײ
				zms[k].blood -= 10;		//��ʬ��Ѫ
				bullets[i].blast = true;
				bullets[i].speed = 0;

				if (zms[k].blood <= 0) {	//��⽩ʬѪ���Ƿ�Ϊ0
					zms[k].dead = true;		//����ʬ����Ϊ����״̬
					zms[k].speed = 0;		//����ʬ���ٶ�����Ϊ0
					zms[k].frameIndex = 0;
				}
				break;		//��ֲ���ӵ���ײ�ɹ�������������ʬ�����ж�
			}
		}
	}
}

void eatPlant() {
	// �����ʬ���ڿ�ʳ
	for (int i = 0; i < ZOMBIE_MAX && zms[i].used; i++) {	//�������н�ʬ,�ҵ�δ��ʹ�õĽ�ʬ
		zms[i].eatingTime++; // ���ӽ�ʬ�Ľ�ʳ��ʱ��
		if (zms[i].eatingTime >= 30) { // ÿ30֡��һ��Ѫ��
			Plant* plant = map[zms[i].row][zms[i].col].plant; // ��ȡֲ�����
			if (plant != nullptr) {
				plant->takeDamage(10); // ÿ�ο�10��Ѫ��
				zms[i].eatingTime = 0; // ���ý�ʳ��ʱ��

				// ���ֲ���Ƿ��Ѿ�����
				if (!plant->isAlive()) {
					map[zms[i].row][zms[i].col].plant->type = 0; // ֲ���������Ƴ�
					map[zms[i].row][zms[i].col].plant->catched = false; // ����ֲ��ı���ʳ״̬

					 //�������н�ʬ�������������ڿ�ʳ��ֲ��Ľ�ʬ״̬
					//for (int j = 0; j < ZOMBIE_MAX && zms[i].used; j++) {
					//	if (zms[j].state == EATING && zms[j].row == map[zms[j].row]) {
					//		// �жϽ�ʬ�������Ƿ���ֲ��λ����
					//		int zombieX = zms[j].x + 80; // ��ʬ�������߽�
					//		int zhiWuX = 256 + k * 81; // ����ֲ���X����
					//		int x1 = zhiWuX + 10; // ֲ��������߽�
					//		int x2 = zhiWuX + 60; // ֲ������Ҳ�߽�
					//		if (zombieX > x1 && zombieX < x2) {
					//			zms[j].state = WALKING; // ��Ϊ����״̬
					//			zms[j].speed = 1; // �ָ���ʬ���ƶ��ٶ�
					//			zms[j].eatingTime = 0; // ���ÿ�ʳ��ʱ��
					//		}
					//	}
					//}
				}
			}
		}
	}
}

void checkZombie_to_Plant() {
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zCount; i++) { // �������н�ʬ
		int row = zms[i].row;
		if (zms[i].state == DEAD) continue;

		for (int k = 0; k < MAP_COL; k++) { // ����9��
			if (map[row][k].plant != nullptr && map[row][k].plant->type == 0) continue; // ����Ƿ����ֲ��

			int zhiWuX = 256 + k * 81; // ����ֲ���X����
			int x1 = zhiWuX + 10; // ֲ��������߽�
			int x2 = zhiWuX + 60; // ֲ������Ҳ�߽�
			int x3 = zms[i].x + 80; // ��ʬ�������߽�

			// �жϽ�ʬ��ֲ���Ƿ�����ײ
			if (x3 > x1 && x3 < x2) {
				if (zms[i].state == EATING) { // �����ʬ���ڿ�ʳ
					zms[i].eatingTime++; // ���ӽ�ʬ�Ľ�ʳ��ʱ��
					if (zms[i].eatingTime >= 30) { // ÿ30֡��һ��Ѫ��
						Plant* plant = map[row][k].plant; // ��ȡֲ�����
						if (plant != nullptr) {
							plant->takeDamage(10); // ÿ�ο�10��Ѫ��
							zms[i].eatingTime = 0; // ���ý�ʳ��ʱ��

							// ���ֲ���Ƿ��Ѿ�����
							if (!plant->isAlive()) {
								map[row][k].plant->type = 0; // ֲ���������Ƴ�
								map[row][k].plant->catched = false; // ����ֲ��ı���ʳ״̬

								// �������н�ʬ�������������ڿ�ʳ��ֲ��Ľ�ʬ״̬
								for (int j = 0; j < zCount; j++) {
									if (zms[j].state == EATING && zms[j].row == row) {
										// �жϽ�ʬ�������Ƿ���ֲ��λ����
										int zombieX = zms[j].x + 80; // ��ʬ�������߽�
										if (zombieX > x1 && zombieX < x2) {
											zms[j].state = WALKING; // ��Ϊ����״̬
											zms[j].speed = 1; // �ָ���ʬ���ƶ��ٶ�
											zms[j].eatingTime = 0; // ���ÿ�ʳ��ʱ��
										}
									}
								}
							}
						}
					}
				}
				else {
					// ��ʬ��ʼ��ʳ
					zms[i].state = EATING;
					zms[i].speed = 0; // ��ʬֹͣ�ƶ�
					zms[i].eatingTime = 0; // ��ʼ����ʳ��ʱ��
				}
			}
		}
	}
}



void collisionCheek() {
	checkBullet_to_ZM();	//����ӵ���ײ��ʬ
	checkZombie_to_Plant();	//��⽩ʬ��ֲ�����ײ���
}

void updatePlant() {
	static int updateCounter = 0; // ��Ӽ�����
	int updateInterval = 5; // ���Ƹ���Ƶ�ʣ�ֵԽ�󶯻�Խ��,5������5��

	if (++updateCounter < updateInterval) {
		return; // ������δ������ʱ��������֡
	}
	updateCounter = 0; // ���ü�����
	for (int i = 0; i < MAP_ROW; i++) {
		for (int j = 0; j < MAP_COL; j++) {
			if (map[i][j].plant != nullptr&&map[i][j].plant->type > 0) {
				map[i][j].plant->frameIndex += 1; // ÿ�ε���ֻ����1
				int zhiWuType = map[i][j].plant->type - 1;
				int index = map[i][j].plant->frameIndex;
				if (index >= 60) {
					map[i][j].plant->frameIndex = 0; // �ص���һ֡��ʵ��ѭ������
				}
				if (imgPlant[zhiWuType][index] == NULL) {
					map[i][j].plant->frameIndex = 0;
				}
			}
		}
	}
}

void updateGame(double dt) {
	updatePlant();// ����ֲ��״̬
	createSunshine();//�������
	updateSunshine();//��������״̬

	createZombie();		//������ʬ
	updateZombie();		//���½�ʬ��״̬

	shoot();		//�����ӵ�
	updateBullets();	//�����ӵ�

	collisionCheek();	//ʵ��ֲ���ӵ��ͽ�ʬ����ײ���
}

void startUI() {
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;

	while (1) {
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);

		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN &&
				msg.x > 474 && msg.x < 474 + 300 &&
				msg.y > 75  && msg.y < 75 + 140) {
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP && flag) {
				EndBatchDraw();
				break;
			}
		}

		EndBatchDraw();
	}
}

// ��ȡ��ǰʱ�䣨�룩  
double getTime() {
	return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void viewScence() {
	int xMin = WIN_WIDTH - imgBg.getwidth();
	vector2 points[9] = {
		{550,80},{530,160},{630,170},{530,200},{515,270},
		{565,370},{605,340},{705,280},{690,340}
	};
	int index[9];
	for (int i = 0; i < 9; i++) {
		index[i] = rand() % 11;     // ��ʬ��վ��֡���
	}

	const int frameChangeRate = 3;  // ���ƽ�ʬ֡�л��ٶ�
	const int sleepTime = 10;        // ˯��ʱ�䣬������������
	int frameCounter = 0;            // ���ƽ�ʬ֡�л��ļ�ʱ��

	// ����ͳһ��֡�л�����
	auto updateZombies = [&](int xOffset) {
		frameCounter++;
		BeginBatchDraw();
		putimage(xOffset, 0, &imgBg);

		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xMin + xOffset, points[k].y, &imgZmStand[index[k]]);
			if (frameCounter >= frameChangeRate) {
				index[k] = (index[k] + 1) % 11; // ���½�ʬ֡
			}
		}

		if (frameCounter >= frameChangeRate) {
			frameCounter = 0; // ���ü�����
		}

		EndBatchDraw();
		Sleep(sleepTime);
		};

	// ��һ�׶Σ��ƶ���Ϸ��������
	for (int x = 0; x >= xMin; x -= 2) {
		updateZombies(x);
	}

	// ͣ��1S����
	for (int i = 0; i < 100; i++) {
		updateZombies(xMin);
	}

	// �ڶ��׶Σ��ƶ���Ϸ��������
	for (int x = xMin; x <= 0; x += 2) {
		updateZombies(x);
	}
}


void barsDown() {
	int height = imgBar.getheight();
	for (int y = -height; y <= 0; y++) {
		BeginBatchDraw();

		putimage(0, 0, &imgBg);
		putimagePNG(250, y, &imgBar);
		putimage(350, y, &imgShovel);
		for (int i = 0; i < ZHI_WU_COUNT; i++) {
			int x = 338 + i * 65;
			putimage(x, 6 + y, &imgCards[i]);
		}
		EndBatchDraw();
		Sleep(5);
	}
}

bool checkOver() {
	int ret = false;
	if (gameStatus == WIN) {
		loadimage(0,"res/win.png");
		Sleep(5000);
		ret = true;
	}
	else if (gameStatus == FAIL) {
		loadimage(0,"res/fail.png");		
		Sleep(5000);
		ret = true;
	}
	return ret;
}

int main() {
	gameInit();
	startUI();  // ����û�����
	viewScence();	//�����Ϸ����(͵����ʬ)
	barsDown();		//�������½�

	const double TIMESTEP = 1.0 / 60.0; // 60 FPS��ʱ�䲽������λ��  
	double accumulator = 0.0;          // �ۻ���ʱ��  
	double lastTime = getTime();        // �ϴθ��µ�ʱ��  
	bool gameIsRunning = true;          // ��Ϸ����״̬  

	// ��Ϸ��ѭ��  
	while (gameIsRunning) {
		double currentTime = getTime(); // ��ǰʱ��  
		double deltaTime = currentTime - lastTime; // ������ʱ��  
		lastTime = currentTime;

		// ��ֹĳ֡����ʱ�����  
		if (deltaTime > 0.25) { // ������������ÿ֡�����ʱ��Ϊ250����  
			deltaTime = 0.25;
		}

		accumulator += deltaTime; // �ۼӵ���ʱ����  

		// ��������������ʱ�䲽��  
		while (accumulator >= TIMESTEP) {
			updateGame(TIMESTEP); // ʹ�ù̶�ʱ�䲽��������Ϸ  
			accumulator -= TIMESTEP;
		}
		// ��ֵ���㣨�����Ҫ�Ļ���������Ի���accumulator��ʣ��ֵ���У�  
		double alpha = accumulator / TIMESTEP;

		// ��Ⱦ��Ϸ  
		updateWindow(); // ע�⣺����û��ʹ��alpha���в�ֵ����ΪrenderGame������Ҫ���������֧�ֲ�ֵ  

		// �����û�����  
		userClick();

		// ����Ƿ�Ӧ���˳���Ϸ  
		if (checkOver()) {
			break;
		}
	}


	return 0;
}
