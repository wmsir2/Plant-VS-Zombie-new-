#include<stdio.h>
#include<windows.h>
#include<iostream>
#include<string>
#include<graphics.h>  //easyx��ͷ�ļ�
#include<math.h>
#include<chrono>
#include<time.h>
#include"tool.h"	//����putimagePNG��getDelay��������
#include"vector2.h"
#include"Plant.h"
#include"sunshineBall.h"
#include"Zombie.h"
#include"config.h"
#include"Map.h"
#include<mmsystem.h>	//��������
#pragma comment(lib,"winmm.lib")
using namespace std;

//enum {WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};	//����һ��ö��
//enum{GOING,WIN,FAIL};		//������Ϸ״̬
//int killCount;	//�Ѿ�ɱ��Ľ�ʬ����
//int zmCount;	//�Ѿ����ֵĽ�ʬ����
//int gameStatus;

//����ͼƬ  IMAGE��ʾgraphics.hͼ�ο��е�ͼ�����
IMAGE imgBg;	//��ʾ����ͼƬ
IMAGE imgBar;	//��ʾ������
IMAGE imgShovelSlot; // ��ʾ���Ӳ۵�ͼ��
IMAGE imgShovel;  // ��ʾ���ӵ�ͼ��
IMAGE imgCards[ZHI_WU_COUNT];	//������һ����Ϊ imgCards ,����ΪIMAGE������,��СΪ ZHI_WU_COUNT
IMAGE* imgPlant[ZHI_WU_COUNT][20];	//������һ����Ϊ imgZhiWu �Ķ�ά���飬��Ԫ������Ϊ IMAGE*��ָ�� IMAGE ���͵�ָ�룩

int curX, curY;	//��ǰѡ�е�ֲ��,���ƶ������е�λ��
int curZhiWu;	//0:û��ѡ��, 1:ѡ���˵�һ��ֲ��

Map map[MAP_ROW][MAP_COL];	//����Map���� map

//enum{SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};		//����״̬

class sunshineBall balls[10];//����һ������
IMAGE imgSunshineBall[29];	//��ʾ����29֡ͼƬ
int sunshine;

class Zombie zms[10];
IMAGE imgZM[22];
IMAGE imgZMDead[20];		//��ʬ������ͼƬ����
IMAGE imgZMEat[21];		//��ʬ��ֲ���ͼƬ����
IMAGE imgZmStand[11];	//��ʬվ����ͼƬ����

//�ӵ�����������
struct bullet {
public:
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast; //�Ƿ�����ը
	int frameIndex;	//�ӵ���ը��֡���
	int injury;		//�ӵ��˺�
};
struct bullet bullets[30];
IMAGE imgBulletNormal;		//��������µ��ӵ�ͼƬ
IMAGE imgBullBlast[4];		//��ը����µ��ӵ�ͼƬ����

bool fileExist(const char* name) {	//�ж��ļ��Ƿ���ڵĺ���
	FILE* fp = fopen(name, "r");	//����һ��ָ�� FILE ��ָ�� fp����������ֻ��ģʽ"r"����Ϊ name ���ļ�
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

	//���ر���ͼƬ
	//���ַ����޸�Ϊ�����ֽ��ַ�����

	loadimage(&imgBg, "res/白天.jpg");
	loadimage(&imgBar, "res/bar5.png");
	loadimage(&imgShovelSlot, "res/���Ӳ�.png");
	loadimage(&imgShovel, "res/����.png");

	memset(imgPlant, 0, sizeof(imgPlant));	//�� imgZhiWu ��һ���ڴ�����ȫ������Ϊ0
	memset(map, 0, sizeof(map));			//�� map ��һ���ڴ���ȫ������Ϊ0

	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;

	//��ʼ��ֲ�￨��
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//����ֲ�￨�Ƶ��ļ���
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png",i+1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i ,j+1);
			//���ж�����ļ��Ƿ����
			if (fileExist(name)) {
				imgPlant[i][j] = new IMAGE;
				loadimage(imgPlant[i][j], name);
			}
			else {
				break;
			}
		}
	}

	 
	curZhiWu = 0;	//ֲ�￨���Ƿ�����ı�־,Ҳ��ֲ������
	sunshine = 50;	//���ó�ʼ���

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//�����������
	srand(time(NULL));

	//������Ϸ��ͼ�λ�����

	//��������
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;	//�����Ч��
	settextstyle(&f);		//��������
	setbkmode(TRANSPARENT);	//���ñ���͸��
	setcolor(BLACK);		//������ɫ

	//��ʼ����ʬ����
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);		//������Դ�ļ���
		loadimage(&imgZM[i], name);			//����22�Ž�ʬͼƬ
	}

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//��ʼ��ֲ���ӵ���֡ͼƬ����
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	//�Ŵ�ͼƬ
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;		
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
			imgBullBlast[3].getwidth() * k,	
			imgBullBlast[3].getheight() * k, true);
	}

	for (int i = 0; i < 20; i++) {		//��ʼ����ʬ������ͼƬ
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);	//��ʼ���ļ���
		loadimage(&imgZMDead[i], name);
	}

	for (int i = 0; i < 21; i++) {		//��ʼ����ʬ��ֲ���ͼƬ
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);	//��ʼ���ļ���
		loadimage(&imgZMEat[i], name);
	}

	for (int i = 0; i < 11; i++) {		//��ʼ����ʬվ����ͼƬ
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);	//��ʼ���ļ���
		loadimage(&imgZmStand[i], name);
	}
}

void drawZombie() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			IMAGE* img = nullptr;
			if (zms[i].dead) img = &imgZMDead[zms[i].frameIndex];    // ʹ�� & ��ȡԪ�ص�ַ
			else if (zms[i].eating) img = &imgZMEat[zms[i].frameIndex];
			else img = &imgZM[zms[i].frameIndex];

			putimagePNG(
				zms[i].x,
				zms[i].y - img->getheight(),
				img
			);
		}
	}
}



void drawBullets() {		//��Ⱦֲ���ӵ�
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletsMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
}

void drawSunshines() {
	//��Ⱦ���
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];	//��ʾ����ĳһ֡ͼƬ
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);	//��ָ����ʽ���ַ�����ӡ��������
	if (sunshine < 100) {
		outtextxy(282, 67, scoreText);
	}
	else {
		outtextxy(276, 67, scoreText);		//��ָ��λ�������ǰ���ֵ
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

				// ֻ�������е�ֲ��
				Plant* plant = map[i][j].plant;
				if (plant != nullptr) {
					putimagePNG(map[i][j].plant->x, map[i][j].plant->y, imgPlant[PlantType][index]); // ��Ⱦֲ��
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

void updateWindow() {		//ˢ�´��ں���
	BeginBatchDraw();	//��ʼ����
	cleardevice();     // �������
	putimage(0, 0, &imgBg);
	//putimage(250, 0, &imgBar);
	putimagePNG(250, 0, &imgBar);

	drawCards();
	drawShovel();
	drawPlant();
	//��Ⱦ�϶������е�ֲ��
	if (curZhiWu > 0) {
		IMAGE* img = imgPlant[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getwidth() / 2, img);
	}

	drawSunshines();



	//��Ⱦ��ʬ
	drawZombie();

	//��Ⱦֲ���ӵ�
	drawBullets();

	EndBatchDraw();	//����˫����
}

void collectSunshine(ExMessage* msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();	//�������Ŀ�
	int h = imgSunshineBall[0].getheight();	//�������ĸ�
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].pCur.x;
			int y = balls[i].pCur.y;
			if (msg->x > x && msg->x < x + w &&
				msg->y > y && msg->y < y + h) {

				balls[i].status = SUNSHINE_COLLECT;
				//sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);	//����ռ�������Ч
				//PlaySound("res/sunshine.wav", NULL,SND_FILENAME | SND_ASYNC);
				//��������ƫ����
				//float destY = 0;	//Ŀ������
				//float destX = 262;
				//float angle = atan((y - destY) / (x - destX));
				//balls[i].xoff = 4 * cos(angle);
				//balls[i].yoff = 4 * sin(angle);
				balls[i].p1 = balls[i].pCur;	//p1Ϊ���
				balls[i].p4 = vector2(262, 0);	//p4Ϊ�յ�
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i].p4);	//�����
				float off = 8;
				balls[i].speed = 1.0 / (distance /off);
				break;
			}
		}
	}
}

void userClick() {
	ExMessage msg;
	static int status = 0;		//�ж��Ƿ�ѡ��ɹ�
	int isPlantArea = 0; // ���������굱ǰ�Ƿ���ֲ�￨������
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {	//�������
			if (msg.message == WM_LBUTTONDOWN && !isPlantArea && status == 0) { //���������µ�λ�ò���ֲ�￨�������ȴ����ռ����
				collectSunshine(&msg);
			}
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {	//�ж��Ƿ�ѡ��ֲ�￨��
				int index = (msg.x - 338) / 65;	//��ʾѡ���������ֲ�￨��
				if (index == 0 && sunshine >= 100) {
					status = 1;						//��ʾѡ��ɹ�
					curZhiWu = index + 1;			//����ֲ��
					isPlantArea = 1; // �����ֲ�￨������
				}
				else if (index == 1 && sunshine >= 50) {
					status = 1;						//��ʾѡ��ɹ�
					curZhiWu = index + 1;			//����ֲ��
					isPlantArea = 1; // �����ֲ�￨������
				}
			}
			else if (curZhiWu > 0 && (msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489)) {	//�������ֲ�����Ƿ����򽫷���ֲ��
				curZhiWu = 0; 
				status = 0;
			}
			else if(msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489) {
				isPlantArea = 0;		//���ڿ�������
				//collectSunshine(&msg);
			}

		}
		else if (msg.message == WM_MOUSEMOVE) {	//����ƶ�
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {	//����ɿ�
			if (msg.x > 338 && msg.x < 338 + 65 * curZhiWu && msg.y < 96) {	//�������ֲ�￨�Ƶ�ʱ��������ֲ�￨�����ɿ�����Ϊ��������ֲ�￨��
				int index = (msg.x - 338) / 65;	//��ʾѡ���������ֲ�￨��
				status = 1;						//��ʾѡ��ɹ�
				curZhiWu = index + 1;			//����ֲ��
				isPlantArea = 1; // �����ֲ�￨������
			}
			else if (curZhiWu > 0 && (msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489)) {		//����ڷǷ������ɿ���������ѡ��
				curZhiWu = 0;
				status = 0;
				isPlantArea = 0;
			}
			else if (msg.x > 256 && msg.x < 1000 && msg.y > 179 && msg.y < 489) {	// ����ڵ�ͼ����Ч�������ɿ���꣬�����ֲ��
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				if (map[row][col].plant == nullptr && status == 1) {		//������Է���ֲ�������ֲ��
					//����ֲ�����
					//���ú�۳����
					if (curZhiWu == 1) {
						if (sunshine >= 100) {
							sunshine -= 100;
							map[row][col].plant = new Peashooter(row, col); // �����㶹����
						}
					}
					else if (curZhiWu == 2) {
						if (sunshine >= 50) {
							sunshine -= 50;
							map[row][col].plant = new Sunflower(row, col); // �������տ�
						}
					}
					// ���ú�����ѡ��״̬
					if (map[row][col].plant != nullptr) {
						map[row][col].plant->type = curZhiWu;
						map[row][col].plant->frameIndex = 0;
						map[row][col].plant->shootTime = 0;

						map[row][col].plant->x = 256 + col * 81;
						map[row][col].plant->y = 179 + row * 102 + 14;

						curZhiWu = 0;
						status = 0;
					}
				}
				else if (map[row][col].plant != nullptr && map[row][col].plant->type != 0) { // �������ֲ������ڸø�����������ѡ��
					curZhiWu = 0;
					status = 0;
				}

			}
		}
	}
}

void createSunshine() {
	static int count = 0;
	static int fre = 200;	//��һ�����������Ҫ��֡��
	count++;
	if (count >= fre) {		//�ڶ��ο�ʼÿ fre ֡����һ�����
		fre = 300 + rand() % 200;	//300-500֡
		count = 0;

		//��������ȡһ������ʹ�õ�
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;		//������ʱ������Ϊ0
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 -112 + rand() % (900 - 320 + 112), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}

	for (int i = 0; i < MAP_ROW; i++) {
		for (int j = 0; j < MAP_COL; j++) {
			if (map[i][j].plant != nullptr && map[i][j].plant->type == XIANG_RI_KUI + 1) {
				map[i][j].plant->produceSun();
			}
		}
	}


	//���տ��������
	//for (int i = 0; i < MAP_ROW; i++) {
	//	for (int j = 0; j < MAP_COL; j++) {
	//		if (map[i][j].plant!=nullptr&&map[i][j].plant->type == XIANG_RI_KUI + 1) {
	//			map[i][j].plant->timer++;
	//			if (map[i][j].plant->timer > 600) {		//���տ�������������
	//				map[i][j].plant->timer = 0;

	//				int k;
	//				for (k = 0; k < ballMax&&balls[k].used; k++);
	//				if (k >= ballMax) return;

	//				balls[k].used = true;
	//				balls[k].p1 = vector2(map[i][j].plant->x, map[i][j].plant->y);
	//				int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);	//����������³����
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
		if (balls[i].used) {	//�����⴦��ʹ��״̬
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;	//��������֡
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
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 300) {		//��ⲥ����˸����  ��ʵ��

					if (balls[i].timer > 400) {		//����ڵ�������ʧ��ʱ��
						balls[i].used = false;
						balls[i].timer = 0;
					}
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) {
					sun->used = false;
					sunshine += 25;		//���+25
					sun->t = 0;
				}
			}
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
	if (zmCount >= ZOMBIE_MAX) {
		return;
	}

	static int zmFre = 200;  // ��ʬ���ֵ�Ƶ��Ϊ200֡
	static int count = 0;    // ��ʾѭ�����õĴ���
	count++;

	if (count > zmFre) {
		count = 0;
		zmFre = 1000 + rand() % 500; // 1000-1500 �ڶ��ο�ʼ�����ʱ����ֽ�ʬ

		for (int i = 0; i < ZOMBIE_MAX; i++) { // ������ʬ����
			if (!zms[i].used) { // �ҵ�δ��ʹ�õĽ�ʬ
				zms[i].used = true;  // ���ҵ��Ľ�ʬ��Ϊʹ��״̬
				zms[i].x = WIN_WIDTH; // ��ʬ���ֵ�λ��Ϊ��Ϸ���ڵĿ��
				zms[i].row = rand() % MAP_ROW; // �������
				zms[i].y = 172 + (1 + zms[i].row) * 100;
				zms[i].speed = 1;
				zms[i].blood = 100;
				zms[i].dead = false;
				zmCount++;
				cout << "�ڵ�" << zms[i].row << "�гɹ�������һ����ʬ" << endl;
				break; // �ҵ���ʬ���˳�
			}
		}
	}
}

void updateZombie() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
	count++;
	if (count > 4) {		//��ʬ�ƶ��ٶȼ���5��
		count = 0;
		//���½�ʬ��λ��
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 56) {
					//printf("GAME OVER\n");
					//MessageBox(NULL, "over", "over", 0);		//���Ż�
					//exit(0);		//���Ż�
					gameStatus = FAIL;
				}
			}
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 > 5) {		//��ʬ���߶�������5��
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {		//ʵ�ֽ�ʬ���߶���
			if (zms[i].used) {
				if (zms[i].dead) {			//��ʬ����
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {	//��ʬ����ͼƬ���ŵ����һ��
						zms[i].used = false;
						killCount++;
						if (killCount == ZOMBIE_MAX) {
							gameStatus = WIN;
						}
					}
				}
				else if (zms[i].eating) {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else {		//��������²��Ž�ʬ���߶���
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
	int dangerX = WIN_WIDTH - 60;	//����ֲ�￪ʼ�����ӵ��ľ���
	for (int i = 0; i < zmCount; i++) {	//�������н�ʬ
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1;
		}
	}

	for (int i = 0; i < MAP_ROW; i++) {
		for (int j = 0; j < MAP_COL; j++) {
			if (map[i][j].plant!=nullptr&&map[i][j].plant->type == WAN_DOU + 1 && lines[i]) {	//������㶹����
				//static int count2 = 0;
				//count2++;
				map[i][j].plant->shootTime++;		//ÿ��ֲ���Լ��ķ�����
				if (map[i][j].plant->shootTime > 80) {	//�����ӵ��ķ�����
					map[i][j].plant->shootTime = 0;

					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);//�ҵ�δ��ʹ�õ��ӵ�
						if (k < bulletMax) {	//�ҵ���δ��ʹ�õ��ӵ�
							bullets[k].used = true;
							bullets[k].row = i;
							bullets[k].speed = 4;

							//��ʼ���ӵ���ը
							bullets[k].blast = false;
							bullets[k].frameIndex = 0;

							int zwX = 256 + j * 81;	//ֲ��x����
							int zwY = 179 + i * 102 + 14;
							bullets[k].x = zwX + imgPlant[map[i][j].plant->type - 1][0]->getwidth() - 10;	//�ӵ�����
							bullets[k].y = zwY + 5;
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
