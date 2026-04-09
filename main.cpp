/*
=============================================================================
                    《植物大战僵尸》游戏主程序
=============================================================================

【游戏总体架构】

本游戏采用面向对象设计，主要包含以下核心系统：

1. 植物系统 (Plant)
   - 豌豆射手 (Peashooter): 发射子弹攻击僵尸
   - 向日葵 (Sunflower): 生产阳光

2. 僵尸系统 (Zombie)
   - 从屏幕右侧生成
   - 向左移动并啃食植物

3. 阳光系统 (Sunshine)
   - 自然掉落阳光
   - 向日葵产出阳光
   - 玩家收集阳光购买植物

4. 子弹系统 (Bullet)
   - 豌豆射手发射
   - 击中僵尸造成伤害

5. 碰撞检测系统
   - 子弹与僵尸碰撞
   - 僵尸与植物碰撞

【游戏循环】

游戏采用固定时间步长 (60 FPS) 运行：
while (游戏运行) {
    1. 计算时间差
    2. 固定时间步长更新游戏逻辑（防止卡顿时物理计算出错）
    3. 渲染画面
    4. 处理用户输入
    5. 检查游戏结束
}

【坐标系统】

- 屏幕宽度: 900像素 (WIN_WIDTH)
- 屏幕高度: 600像素 (WIN_HEIGHT)
- 地图区域: x=256~1000, y=179~489
- 地图网格: 3行 x 9列 (MAP_ROW x MAP_COL)
- 网格宽度: 81像素 (每格)
- 网格高度: 102像素 (每格)
- 卡片区域: x=338~, y<96

=============================================================================
*/

// -------------------- 头文件引用 --------------------
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <graphics.h>  // EasyX图形库
#include <math.h>
#include <chrono>
#include <time.h>
#include "tool.h"       // 工具函数：putimagePNG、getDelay等
#include "vector2.h"    // 二维向量类
#include "Plant.h"      // 植物类定义
#include "Zombie.h"     // 僵尸类定义（必须在 PotatoMine.h 之前）
#include "Peashooter.h" // 豌豆射手类
#include "Sunflower.h"  // 向日葵类
#include "PotatoMine.h" // 土豆地雷类
#include "PlantFactory.h" // 植物工厂
#include "sunshineBall.h" // 阳光球类定义
#include "config.h"     // 游戏配置
#include "Map.h"        // 地图类定义
#include <mmsystem.h>   // 多媒体播放头文件
#pragma comment(lib, "winmm.lib") // 链接winmm.lib库用于播放音乐
using namespace std;

// ============================================================================
//                              全局变量定义
// ============================================================================

// -------------------- 游戏图片资源 --------------------
IMAGE imgBg;                 // 游戏背景图片
IMAGE imgBar;                // 顶部卡片栏图片
IMAGE imgShovelSlot;         // 铲子槽图片（UI元素）
IMAGE imgShovel;             // 铲子图片
IMAGE imgCards[ZHI_WU_COUNT]; // 植物卡片图片数组
IMAGE imgMower;              // 推车图片
IMAGE imgPotatoMine;         // 土豆地雷图片
IMAGE imgPotatoUnderground;  // 土豆地雷地下图片
IMAGE imgPotatoExplode;      // 土豆地雷爆炸图片

// -------------------- 植物动画资源 --------------------
// imgPlant[i][j]: 第i种植物的第j帧动画图片
// 第一维: 植物种类 (0=豌豆射手, 1=向日葵)
// 第二维: 动画帧序号 (最多20帧)
IMAGE* imgPlant[ZHI_WU_COUNT][20];

// -------------------- 用户交互状态 --------------------
int curX, curY;              // 鼠标当前坐标（用于绘制选中植物的跟随效果）
int curZhiWu;                // 当前选中的植物种类 (0=未选中, 1=豌豆射手, 2=向日葵)
int selectCost;              // 选中植物时扣除的阳光数量（用于右键取消时返还）
bool shovelMode;             // 铲子模式标志 (true=铲子模式, false=普通模式)

// -------------------- 游戏地图 --------------------
// map[row][col]: 二维数组，每个元素包含一个格子信息
// 格子信息: 是否有植物、植物类型、动画帧等
MapCell map[MAP_ROW][MAP_COL];

// -------------------- 阳光系统 --------------------
// balls[]: 阳光球对象数组（屏幕上同时存在的阳光数量有限，最多10个）
// 使用{}进行值初始化，确保所有成员变量被零初始化
SunshineBall balls[10] = {};
IMAGE imgSunshineBall[29];   // 阳光球动画帧图片（共29帧）
int sunshine;                // 当前阳光数量（资源值）

// -------------------- 僵尸系统 --------------------
// zms[]: 僵尸对象数组（最多同时存在10个僵尸）
// 使用{}进行值初始化，确保所有成员变量被零初始化
class Zombie zms[10] = {};
IMAGE imgZM[22];             // 僵尸走路动画帧图片（共22帧）
IMAGE imgZMDead[20];         // 僵尸死亡动画帧图片
IMAGE imgZMEat[21];          // 僵尸吃植物动画帧图片
IMAGE imgZmStand[11];        // 僵尸站立等待动画帧图片

// -------------------- 子弹系统 --------------------
struct bullet {
    int x, y;                // 子弹坐标
    int row;                 // 子弹所在行（用于判断击中哪个僵尸）
    bool used;               // 子弹是否被使用
    int speed;               // 子弹移动速度
    bool blast;              // 子弹是否在爆炸状态
    int frameIndex;          // 子弹爆炸动画帧序号
    int injury;              // 子弹伤害值
};
struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBullBlast[4];

// -------------------- Explosion effects --------------------
ExplosionEffect explosions[10];  // From Plant.h
IMAGE imgExplosion[8];  // Explosion animation frames (备用)

// -------------------- UI Function Forward Declarations --------------------
void drawPauseButton();
void drawWaveIndicator();
void drawMowers();
void drawCardShade();
void drawPlantHealthBar();
void drawMessage();
void drawWaveWarning();
void drawPauseOverlay();
void drawExplosions();
void drawUI();
void updateCardCooldowns(double dt);
void updateMessages(double dt);
void updateWaveSystem(double dt);
void updateExplosions(double dt);
void updateMowers();

// -------------------- UI State Variables --------------------
// 游戏状态
bool gamePaused = false;           // 游戏是否暂停
bool shovelSelected = false;       // 铲子是否选中(用于UI高亮)
int selectedPlantIndex = -1;       // 当前选中的卡片索引(-1=无)

// 波次系统
int currentWave = 1;              // 当前波次
int totalWaves = 5;               // 总波次数
int zombiesPerWave = 3;           // 每波僵尸数量
int zombiesSpawnedInWave = 0;     // 当前波已生成的僵尸数
bool waveWarningActive = false;    // 是否显示波次警告
float waveWarningTimer = 0;        // 波次警告计时器

// 卡牌冷却
float cardCooldown[ZHI_WU_COUNT] = {0};  // 每张卡牌的冷却时间
float cardCooldownMax = 5.0f;     // 冷却最大时间(秒)

// 消息系统
float messageTimer = 0;           // 消息显示计时器
char messageText[128] = "";       // 消息文本

// 关卡选择
int currentLevel = 1;             // 当前关卡索引

// 推车状态
bool mowerActivated[MAP_ROW] = {false};  // 每行推车是否已激活
int mowerX[MAP_ROW];  // 每行推车的X位置（移动时使用）

// ============================================================================
//                              辅助函数
// ============================================================================

/**
 * @brief 检查文件是否存在
 * @param name 文件路径
 * @return true 文件存在
 * @return false 文件不存在
 */
bool fileExist(const char* name) {
    FILE* fp = nullptr;
    fopen_s(&fp, name, "r");
    if (fp == NULL) {
        return false;
    } else {
        fclose(fp);
        return true;
    }
}

// ============================================================================
//                              游戏初始化
// ============================================================================

/**
 * @brief 游戏初始化函数
 *
 * 在游戏开始前调用，负责：
 * 1. 初始化图形窗口
 * 2. 加载所有图片资源
 * 3. 初始化游戏状态变量
 */
void gameInit() {
    // -------- 初始化图形窗口 --------
    // 窗口大小: WIN_WIDTH x WIN_HEIGHT
    initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

    // -------- 加载游戏背景和UI图片 --------
    loadimage(&imgBg, "res/background.jpg");           // 游戏背景
    loadimage(&imgBar, "res/bar5.png");          // 顶部卡片栏
    loadimage(&imgShovelSlot, "res/shovel_slot.png"); // 铲子槽
    loadimage(&imgShovel, "res/shovel.png");       // 铲子图片
    loadimage(&imgMower, "res/mower.png");      // 推车图片
    loadimage(&imgPotatoMine, "res/potato_mine_dir/potato_mine_ready.gif"); // 土豆地雷图片
    loadimage(&imgPotatoUnderground, "res/potato_mine_dir/potato_underground.gif"); // 土豆地雷地下图片
    loadimage(&imgPotatoExplode, "res/explosion/0.gif"); // 土豆地雷爆炸图片

    // -------- 初始化数组和地图 --------
    memset(imgPlant, 0, sizeof(imgPlant));      // 植物动画数组置0
    memset(map, 0, sizeof(map));                 // 地图数据置0

    // -------- 初始化游戏状态 --------
    killCount = 0;   // 已击杀的僵尸数量
    zmCount = 0;     // 已生成的僵尸数量
    gameStatus = GOING; // 游戏状态设为进行中

    // -------- 初始化波次系统 --------
    currentWave = 1;
    totalWaves = 5;
    zombiesPerWave = 3;
    zombiesSpawnedInWave = 0;
    waveWarningActive = true;
    waveWarningTimer = 3.0f;

    // -------- 加载植物卡片和动画帧 --------
    char name[64];
    const char* cardNames[ZHI_WU_COUNT] = {
        "card_peashooter.png",   // WAN_DOU
        "card_sunflower.png",    // XIANG_RI_KUI
        "card_potato_mine.png"   // TU_DAO
    };
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        // 加载植物卡片图片
        sprintf_s(name, sizeof(name), "res/Cards/%s", cardNames[i]);
        loadimage(&imgCards[i], name);

        // 加载该植物的所有动画帧
        for (int j = 0; j < 20; j++) {
            sprintf_s(name, sizeof(name), "res/plants/%d/%d.png", i, j + 1);
            if (fileExist(name)) {
                imgPlant[i][j] = new IMAGE;
                loadimage(imgPlant[i][j], name);
            } else {
                break; // 文件不存在，停止加载
            }
        }
    }

    // -------- 加载阳光球动画帧 --------
    for (int i = 0; i < 29; i++) {
        sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
        loadimage(&imgSunshineBall[i], name);
    }

    // -------- 加载爆炸效果 --------
    for (int i = 0; i < 8; i++) {
        sprintf_s(name, sizeof(name), "res/explosion/%d.png", i + 1);
        if (fileExist(name)) {
            loadimage(&imgExplosion[i], name);
        }
    }

    // -------- 初始化爆炸效果数组 --------
    memset(explosions, 0, sizeof(explosions));

    // -------- 初始化随机数生成器 --------
    srand((unsigned int)time(NULL));

    // -------- 设置文字样式 --------
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 30;
    f.lfWeight = 15;
    strcpy_s(f.lfFaceName, "Segoe UI Black");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    setbkmode(TRANSPARENT); // 文字背景透明
    setcolor(BLACK);

    // -------- 加载僵尸动画帧 --------
    memset(zms, 0, sizeof(zms));
    for (int i = 0; i < 22; i++) {
        sprintf_s(name, sizeof(name), "res/zombies_walk/%d.png", i + 1);
        loadimage(&imgZM[i], name);
    }

    // -------- 加载子弹图片 --------
    loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
    memset(bullets, 0, sizeof(bullets));

    // 加载子弹爆炸图片
    loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
    for (int i = 0; i < 3; i++) {
        float k = (float)((i + 1) * 0.2);
        loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
            (int)(imgBullBlast[3].getwidth() * k),
            (int)(imgBullBlast[3].getheight() * k));
    }

    // -------- 加载僵尸死亡动画 --------
    for (int i = 0; i < 20; i++) {
        sprintf_s(name, sizeof(name), "res/zombies_dead/%d.png", i + 1);
        loadimage(&imgZMDead[i], name);
    }

    // -------- 加载僵尸吃东西动画 --------
    for (int i = 0; i < 21; i++) {
        sprintf_s(name, sizeof(name), "res/zombies_eat/%d.png", i + 1);
        loadimage(&imgZMEat[i], name);
    }

    // -------- 加载僵尸站立等待动画 --------
    for (int i = 0; i < 11; i++) {
        sprintf_s(name, sizeof(name), "res/zombies_stand/%d.png", i + 1);
        loadimage(&imgZmStand[i], name);
    }

    // -------- 初始化玩家状态 --------
    curZhiWu = 0;     // 当前没有选中任何植物
    shovelMode = false;
    sunshine = 50;    // 初始阳光值

    // -------- 初始化小推车位置 --------
    for (int i = 0; i < MAP_ROW; i++) {
        mowerX[i] = MOWER_X;
        mowerActivated[i] = false;
    }
}

// ============================================================================
//                              绘制函数（Render）
// ============================================================================

/**
 * @brief 绘制僵尸
 *
 * 遍历所有僵尸，根据其状态绘制不同动画帧：
 * - 死亡状态: 显示死亡动画
 * - 吃东西状态: 显示吃东西动画
 * - 正常状态: 显示走路动画
 */
void drawZombie() {
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    for (int i = 0; i < zmMax; i++) {
        if (zms[i].used) {
            IMAGE* img = nullptr;

            // 根据僵尸状态选择动画帧
            if (zms[i].dead && !zms[i].exploded) {
                // 死亡状态（不包括被土豆地雷炸死的情况）
                img = &imgZMDead[zms[i].frameIndex];
            } else if (zms[i].eating) {
                // 吃东西状态
                img = &imgZMEat[zms[i].frameIndex];
            } else {
                // 正常走路状态
                img = &imgZM[zms[i].frameIndex];
            }

            // 绘制僵尸
            putimagePNG(
                zms[i].x,
                zms[i].y - img->getheight(),
                img
            );
        }
    }
}

/**
 * @brief 绘制子弹
 *
 * 遍历所有子弹，绘制普通子弹或爆炸动画
 */
void drawBullets() {
    int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
    for (int i = 0; i < bulletsMax; i++) {
        if (bullets[i].used) {
            if (bullets[i].blast) {
                // 爆炸状态，绘制爆炸动画帧
                IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
                putimagePNG(bullets[i].x, bullets[i].y, img);
            } else {
                // 普通子弹
                putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
            }
        }
    }
}

/**
 * @brief 绘制阳光球
 *
 * 遍历所有阳光球并绘制，同时在UI栏显示当前阳光数量
 */
void drawSunshines() {
    // 绘制所有阳光球
    for (int i = 0; i < ballMax; i++) {
        if (balls[i].isUsed) {
            IMAGE* img = &imgSunshineBall[balls[i].frame];
            putimagePNG((int)balls[i].currentPos.x, (int)balls[i].currentPos.y, img);
        }
    }

    // 显示阳光数量(放大并偏移)
    char scoreText[8];
    sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);

    // 设置大字体
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 35;
    f.lfWeight = 20;
    strcpy_s(f.lfFaceName, "Segoe UI Black");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    setbkcolor(TRANSPARENT);

    if (sunshine < 100) {
        setcolor(BLACK);
        outtextxy(282, 67, scoreText);
        setcolor(WHITE);
        outtextxy(281, 66, scoreText);
    } else {
        setcolor(BLACK);
        outtextxy(276, 67, scoreText);
        setcolor(WHITE);
        outtextxy(275, 66, scoreText);
    }

    // 恢复默认字体
    f.lfHeight = 30;
    f.lfWeight = 15;
    strcpy_s(f.lfFaceName, "Segoe UI Black");
    settextstyle(&f);
    setcolor(BLACK);
}

/**
 * @brief 绘制植物卡片
 *
 * 在屏幕顶部绘制所有植物卡片
 */
void drawCards() {
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        int x = 338 + i * 65;  // 每张卡片间距65像素
        int y = 5;
        putimage(x, y, &imgCards[i]);
    }
}

/**
 * @brief 绘制地图上的植物
 *
 * 遍历地图，通过多态调用各植物的 draw() 方法
 */
void drawPlant() {
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            if (map[i][j].plant != nullptr) {
                Plant* plant = map[i][j].plant;
                if (!plant->isDead()) {
                    plant->draw();  // 多态调用
                }
            }
        }
    }
}

/**
 * @brief 绘制铲子
 *
 * 在屏幕右上角绘制铲子槽和铲子
 */
void drawShovel() {
    int x = 838;
    int y = 5;
    putimagePNG(x, y, &imgShovelSlot);
    putimagePNG(x, y, &imgShovel);

    // 铲子选中时绘制高亮效果
    if (shovelSelected || shovelMode) {
        setlinestyle(PS_SOLID, 3);
        setlinecolor(RGB(255, 255, 0));
        rectangle(x - 2, y - 2, x + 52, y + 52);
        setlinestyle(PS_SOLID, 1);
    }
}

// ============================================================================
//                              UI绘制函数
// ============================================================================

/**
 * @brief 绘制暂停按钮
 */
void drawPauseButton() {
    int x = PAUSE_BTN_X;
    int y = PAUSE_BTN_Y;
    int size = PAUSE_BTN_SIZE;

    // 绘制暂停图标(两个竖条)
    setfillcolor(RGB(50, 50, 50));
    bar(x + 8, y + 5, x + 15, y + size - 5);
    bar(x + 25, y + 5, x + 32, y + size - 5);
}

/**
 * @brief 绘制波次进度条(右下角)
 */
void drawWaveIndicator() {
    if (currentWave <= 0 || totalWaves <= 0) return;

    // 进度条位置(右下角)
    int barX = WIN_WIDTH - 180;
    int barY = WIN_HEIGHT - 40;
    int barWidth = 150;
    int barHeight = 25;

    // 绘制背景框
    setfillcolor(RGB(50, 50, 50));
    fillrectangle(barX, barY, barX + barWidth, barY + barHeight);

    // 绘制进度
    float progress = (float)currentWave / totalWaves;
    int fillWidth = (int)(barWidth * progress);

    if (fillWidth > 0) {
        setfillcolor(RGB(100, 200, 100));
        fillrectangle(barX, barY, barX + fillWidth, barY + barHeight);
    }

    // 绘制边框
    setlinecolor(RGB(200, 200, 200));
    setlinestyle(PS_SOLID, 2);
    rectangle(barX, barY, barX + barWidth, barY + barHeight);
    setlinestyle(PS_SOLID, 1);

    // 绘制文字
    char text[32];
    sprintf_s(text, "Wave %d/%d", currentWave, totalWaves);
    settextstyle(16, 0, "Segoe UI Black");
    setbkcolor(TRANSPARENT);
    setcolor(RGB(255, 255, 255));
    outtextxy(barX + barWidth / 2 - 35, barY + 4, text);
}

/**
 * @brief 绘制推车/除草机
 */
void drawMowers() {
    for (int row = 0; row < MAP_ROW; row++) {
        int x = mowerX[row];
        int y = MOWER_Y_BASE + row * MOWER_ROW_SPACE;

        // 绘制推车图片（使用putimagePNG处理透明背景）
        if (imgMower.getwidth() > 0) {
            putimagePNG(x, y, &imgMower);
        } else {
            // 如果图片加载失败，使用简化版
            if (mowerActivated[row]) {
                setfillcolor(RGB(255, 50, 50));
            } else {
                setfillcolor(RGB(50, 200, 50));
            }
            fillrectangle(x, y, x + 25, y + 40);
            setfillcolor(RGB(80, 80, 80));
            fillcircle(x + 6, y + 38, 5);
            fillcircle(x + 19, y + 38, 5);
        }
    }
}

/**
 * @brief 绘制卡片遮罩(阳光不足时变灰/冷却中)
 */
void drawCardShade() {
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        int x = CARD_START_X + i * CARD_SPACING;
        int y = CARD_START_Y;

        // 获取植物价格
        int cost;
        if (i == 0) cost = PEASHOOTER_COST;
        else if (i == 1) cost = SUNFLOWER_COST;
        else cost = TUDAO_COST;

        // 阳光不足时绘制红色边框
        if (sunshine < cost) {
            setlinecolor(RGB(200, 50, 50));
            setlinestyle(PS_SOLID, 3);
            rectangle(x, y, x + CARD_WIDTH, y + CARD_HEIGHT);
            setlinestyle(PS_SOLID, 1);
        }

        // 冷却中时绘制蓝色边框进度
        if (cardCooldown[i] > 0) {
            float ratio = cardCooldown[i] / cardCooldownMax;
            setlinecolor(RGB(50, 50, 200));
            setlinestyle(PS_SOLID, 2);
            // 从下往上绘制剩余冷却比例的边框
            int shadeHeight = (int)(CARD_HEIGHT * ratio);
            rectangle(x, y + CARD_HEIGHT - shadeHeight, x + CARD_WIDTH, y + CARD_HEIGHT);
            setlinestyle(PS_SOLID, 1);
        }
    }
}

/**
 * @brief 绘制植物血量条
 */
void drawPlantHealthBar() {
    if (selectedPlantIndex < 0) return;

    // 遍历地图找到选中的植物
    for (int row = 0; row < MAP_ROW; row++) {
        for (int col = 0; col < MAP_COL; col++) {
            Plant* plant = map[row][col].plant;
            if (plant != nullptr && plant->isAlive()) {
                // 如果点击了这个植物，显示血量条
                // 这里可以扩展为记录当前选中的植物
            }
        }
    }
}

/**
 * @brief 绘制消息提示
 */
void drawMessage() {
    if (messageTimer <= 0) return;

    settextstyle(20, 0, "Segoe UI Black");
    setbkcolor(TRANSPARENT);

    // 绘制背景
    setfillcolor(RGB(200, 200, 100));
    int msgWidth = 300;
    int msgHeight = 40;
    fillrectangle(WIN_WIDTH / 2 - msgWidth / 2, WIN_HEIGHT - 100,
                  WIN_WIDTH / 2 + msgWidth / 2, WIN_HEIGHT - 100 + msgHeight);

    // 绘制文字(居中)
    setcolor(RGB(255, 0, 0));
    outtextxy(WIN_WIDTH / 2 - 70, WIN_HEIGHT - 95, messageText);
}

/**
 * @brief 绘制波次警告
 */
void drawWaveWarning() {
    if (!waveWarningActive) return;

    settextstyle(36, 0, "Segoe UI Black");
    setbkcolor(TRANSPARENT);

    // 绘制红色警告
    setcolor(RGB(255, 0, 0));
    outtextxy(WIN_WIDTH / 2 - 100, 80, "一大波僵尸即将来袭!");
}

/**
 * @brief 绘制暂停界面
 */
void drawPauseOverlay() {
    if (!gamePaused) return;

    // 黑色背景
    setfillcolor(RGB(0, 0, 0));
    fillrectangle(0, 0, WIN_WIDTH, WIN_HEIGHT);

    // 暂停文字
    settextstyle(48, 0, "Segoe UI Black");
    setbkcolor(TRANSPARENT);
    setcolor(RGB(255, 255, 255));
    outtextxy(WIN_WIDTH / 2 - 50, 200, "PAUSED");

    // 继续按钮
    setfillcolor(RGB(50, 200, 50));
    fillrectangle(WIN_WIDTH / 2 - 80, 280, WIN_WIDTH / 2 + 80, 330);
    setcolor(RGB(0, 0, 0));
    settextstyle(24, 0, "Segoe UI Black");
    outtextxy(WIN_WIDTH / 2 - 35, 295, "Continue");

    // 退出按钮
    setfillcolor(RGB(200, 50, 50));
    fillrectangle(WIN_WIDTH / 2 - 80, 350, WIN_WIDTH / 2 + 80, 400);
    setcolor(RGB(255, 255, 255));
    outtextxy(WIN_WIDTH / 2 - 15, 365, "Quit");
}

/**
 * @brief 统一绘制所有UI元素
 */
void drawUI() {
    drawPauseButton();
    drawWaveIndicator();
    drawMowers();
    drawCardShade();
    drawMessage();
    drawWaveWarning();
    drawPauseOverlay();
    drawExplosions();
}

/**
 * @brief 更新窗口（主渲染函数）
 *
 * 负责绘制游戏的所有元素：
 * 1. 清空屏幕
 * 2. 绘制背景和UI
 * 3. 绘制植物卡片
 * 4. 绘制地图上的植物
 * 5. 绘制选中植物的跟随效果
 * 6. 绘制铲子跟随效果
 * 7. 绘制阳光
 * 8. 绘制僵尸
 * 9. 绘制子弹
 */
void updateWindow() {
    BeginBatchDraw();     // 开始批量绘制
    cleardevice();        // 清空屏幕

    // 绘制背景
    putimage(0, 0, &imgBg);
    putimagePNG(250, 0, &imgBar);

    // 绘制卡片和铲子
    drawCards();
    drawShovel();

    // 绘制地图上的植物
    drawPlant();

    // 绘制选中植物跟随鼠标效果
    if (curZhiWu > 0) {
        if (curZhiWu == 3) {
            // 土豆地雷使用地下状态图片预览
            putimagePNG(curX - imgPotatoUnderground.getwidth() / 2, curY - imgPotatoUnderground.getheight() / 2, &imgPotatoUnderground);
        } else {
            IMAGE* img = imgPlant[curZhiWu - 1][0];
            putimagePNG(curX - img->getwidth() / 2, curY - img->getwidth() / 2, img);
        }
    }

    // 绘制铲子跟随鼠标效果
    if (shovelMode) {
        putimagePNG(curX - imgShovel.getwidth() / 2, curY - imgShovel.getheight() / 2, &imgShovel);
    }

    // 绘制阳光球
    drawSunshines();

    // 绘制僵尸
    drawZombie();

    // 绘制子弹
    drawBullets();

    // 绘制UI元素
    drawUI();

    EndBatchDraw();       // 结束批量绘制
}

// ============================================================================
//                              用户输入处理
// ============================================================================

/**
 * @brief 收集阳光
 *
 * 当鼠标点击阳光球时，将阳光状态改为飞向UI栏
 * @param msg 鼠标消息
 */
void collectSunshine(ExMessage* msg) {
    int count = sizeof(balls) / sizeof(balls[0]);
    int w = imgSunshineBall[0].getwidth();   // 阳光球宽度
    int h = imgSunshineBall[0].getheight(); // 阳光球高度

    for (int i = 0; i < count; i++) {
        if (balls[i].isUsed) {
            int x = (int)balls[i].currentPos.x;
            int y = (int)balls[i].currentPos.y;

            // 检测鼠标是否点击了阳光球（点击范围在阳光球矩形内）
            if (msg->x > x && msg->x < x + w &&
                msg->y > y && msg->y < y + h) {

                balls[i].state = SUNSHINE_COLLECT;  // 改为飞向UI栏状态

                // 播放阳光收集音效
                mciSendString("play res/sunshine.mp3", 0, 0, 0);

                // 设置阳光飞行参数：起点为当前位置，终点为UI栏阳光池
                balls[i].controlP1 = balls[i].currentPos;
                balls[i].controlP4 = vector2(262, 0);  // 阳光池位置
                balls[i].timeT = 0;

                // 根据距离计算飞行速度
                float distance = (float)dis(balls[i].controlP1 - balls[i].controlP4);
                float off = 8.0f;
                balls[i].speed = (float)(1.0 / (distance / off));
                break;
            }
        }
    }
}

/**
 * @brief 处理用户点击
 *
 * 处理鼠标左键、右键、移动等消息：
 * - 左键点击卡片区：选择植物
 * - 左键点击地图区：放置植物
 * - 左键点击铲子区：切换铲子模式
 * - 左键点击已有植物：铲子模式下删除植物
 * - 右键点击：取消当前操作，返还阳光
 * - 鼠标移动：更新跟随位置
 */
void userClick() {
    ExMessage msg;
    static int status = 0;       // 内部状态（0=未选中植物，1=已选中植物等待放置）
    int isPlantArea = 0;        // 标记是否在植物卡片区域内

    if (peekmessage(&msg)) {     // 获取鼠标消息
        // -------------------- 鼠标左键按下 --------------------
        if (msg.message == WM_LBUTTONDOWN) {
            // ----------- 暂停界面 -----------
            if (gamePaused) {
                // 点击继续按钮
                if (msg.x >= WIN_WIDTH / 2 - 80 && msg.x <= WIN_WIDTH / 2 + 80 &&
                    msg.y >= 280 && msg.y <= 330) {
                    gamePaused = false;
                }
                // 点击退出按钮
                else if (msg.x >= WIN_WIDTH / 2 - 80 && msg.x <= WIN_WIDTH / 2 + 80 &&
                         msg.y >= 350 && msg.y <= 400) {
                    // 退出游戏
                    exit(0);
                }
                return;
            }

            // ----------- 暂停按钮 -----------
            if (msg.x >= PAUSE_BTN_X && msg.x <= PAUSE_BTN_X + PAUSE_BTN_SIZE &&
                msg.y >= PAUSE_BTN_Y && msg.y <= PAUSE_BTN_Y + PAUSE_BTN_SIZE) {
                gamePaused = !gamePaused;
                return;
            }

            // 在游戏区域内点击，尝试收集阳光
            if (!isPlantArea && status == 0) {
                collectSunshine(&msg);
            }

            // ----------- 检查是否点击了植物卡片区 -----------
            // 卡片区范围: x=338~338+65*ZHI_WU_COUNT, y<96
            if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {
                int index = (msg.x - 338) / 65;  // 计算点击的是第几张卡片

                // 检查冷却
                if (cardCooldown[index] > 0) {
                    strcpy_s(messageText, "卡片冷却中!");
                    messageTimer = MSG_DURATION;
                    return;
                }

                // 第一张卡片：豌豆射手（消耗100阳光）
                if (index == 0 && sunshine >= 100) {
                    sunshine -= 100;       // 扣除阳光
                    selectCost = 100;      // 记录花费（用于返还）
                    status = 1;            // 标记已选中植物
                    curZhiWu = index + 1;  // 设置当前植物类型
                    isPlantArea = 1;      // 标记在卡片区域
                    shovelSelected = false;  // 取消铲子选中
                }
                // 第二张卡片：向日葵（消耗50阳光）
                else if (index == 1 && sunshine >= 50) {
                    sunshine -= 50;
                    selectCost = 50;
                    status = 1;
                    curZhiWu = index + 1;
                    isPlantArea = 1;
                    shovelSelected = false;  // 取消铲子选中
                }
                // 第三张卡片：土豆地雷（消耗25阳光）
                else if (index == 2 && sunshine >= TUDAO_COST) {
                    sunshine -= TUDAO_COST;
                    selectCost = TUDAO_COST;
                    status = 1;
                    curZhiWu = index + 1;
                    isPlantArea = 1;
                    shovelSelected = false;  // 取消铲子选中
                }
                // 阳光不足
                else if ((index == 0 && sunshine < 100) || (index == 1 && sunshine < 50) || (index == 2 && sunshine < TUDAO_COST)) {
                    strcpy_s(messageText, "阳光不足!");
                    messageTimer = MSG_DURATION;
                }
            }
            // 在游戏区域外点击，取消选中
            else if (curZhiWu > 0 && (msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489)) {
                curZhiWu = 0;
                status = 0;
            }

            // ----------- 检查是否点击了铲子区域 -----------
            // 铲子区域: x=838~890, y=5~60
            if (msg.x > 838 && msg.x < 890 && msg.y > 5 && msg.y < 60) {
                shovelMode = !shovelMode;  // 切换铲子模式
                shovelSelected = shovelMode;  // 更新UI状态
                curZhiWu = 0;             // 取消选中植物
                status = 0;
            }
            // 点击空白区域，关闭铲子模式
            else if (msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489) {
                isPlantArea = 0;
                if (shovelMode) {
                    shovelMode = false;
                    shovelSelected = false;
                }
            }
        }

        // -------------------- 鼠标右键按下（取消操作） --------------------
        else if (msg.message == WM_RBUTTONDOWN) {
            if (curZhiWu > 0) {
                sunshine += selectCost;  // 返还阳光
            }
            curZhiWu = 0;
            selectCost = 0;
            status = 0;
            shovelMode = false;
            shovelSelected = false;
        }

        // -------------------- 鼠标移动 --------------------
        else if (msg.message == WM_MOUSEMOVE) {
            curX = msg.x;
            curY = msg.y;
        }

        // -------------------- 鼠标左键释放 --------------------
        else if (msg.message == WM_LBUTTONUP) {
            // 在卡片区释放，选中该卡片
            if (msg.x > 338 && msg.x < 338 + 65 * curZhiWu && msg.y < 96) {
                int index = (msg.x - 338) / 65;
                status = 1;
                curZhiWu = index + 1;
                isPlantArea = 1;
            }
            // 在非植物区域释放，取消选中
            else if (curZhiWu > 0 && (msg.x < 256 || msg.x > 1000 || msg.y < 179 || msg.y > 489)) {
                curZhiWu = 0;
                status = 0;
                isPlantArea = 0;
            }
            // 在地图有效区域内释放，放置植物
            else if (msg.x > 256 && msg.x < 1000 && msg.y > 179 && msg.y < 489) {
                int row = (msg.y - 179) / 102;  // 计算行号
                int col = (msg.x - 256) / 81;  // 计算列号

                // 边界检查，防止数组越界
                if (row < 0 || row >= MAP_ROW || col < 0 || col >= MAP_COL) {
                    curZhiWu = 0;
                    status = 0;
                    return;
                }

                // 如果该位置没有植物且已经选中植物
                if ((map[row][col].plant == nullptr || map[row][col].plant->isDead()) && status == 1) {
                    // 清除死亡植物的内存
                    if (map[row][col].plant != nullptr) {
                        delete map[row][col].plant;
                    }
                    // 使用工厂创建植物
                    map[row][col].plant = PlantFactory::createPlant(curZhiWu, row, col);

                    if (map[row][col].plant != nullptr) {
                        // 启动卡片冷却（curZhiWu范围1~3，对应索引0~2）
                        cardCooldown[curZhiWu - 1] = cardCooldownMax;

                        // 重置选中状态
                        curZhiWu = 0;
                        status = 0;
                    }
                }
                // 如果点击位置有植物
                else if (map[row][col].plant != nullptr) {
                    Plant* p = map[row][col].plant;
                    // 铲子模式下删除植物
                    if (shovelMode) {
                        delete p;
                        map[row][col].plant = nullptr;
                        shovelMode = false;
                        shovelSelected = false;
                    }
                    curZhiWu = 0;
                    status = 0;
                }
            }
        }
    }
}

// ============================================================================
//                              阳光系统
// ============================================================================

/**
 * @brief 创建阳光
 *
 * 每隔一定帧数在随机位置创建一个掉落的阳光球
 * 同时检查所有向日葵是否需要产出阳光
 */
void createSunshine() {
    static int count = 0;
    static int fre = 200;  // 下一波生成阳光需要的帧数
    count++;

    // 到达指定帧数后生成阳光
    if (count >= fre) {
        fre = 300 + rand() % 200;  // 300~500帧后再次生成
        count = 0;

        // 找一个未使用的阳光球
        int i;
        for (i = 0; i < ballMax && balls[i].isUsed; i++);
        if (i >= ballMax) return;

        // 初始化阳光球属性
        balls[i].isUsed = true;
        balls[i].frame = 0;
        balls[i].timer = 0;
        balls[i].state = SUNSHINE_DOWN;  // 下落状态
        balls[i].timeT = 0;

        // 设置初始位置（屏幕顶部随机位置）
        balls[i].controlP1 = vector2(260 - 112 + rand() % (900 - 320 + 112), 60);

        // 设置目标位置（地面位置）
        balls[i].controlP4 = vector2((int)balls[i].controlP1.x, 200 + (rand() % 4) * 90);

        // 根据距离计算速度
        float off = 2.0f;
        float distance = (float)(balls[i].controlP4.y - balls[i].controlP1.y);
        balls[i].speed = (float)(1.0 / (distance / off));
    }
}

/**
 * @brief 更新阳光球状态
 *
 * 根据阳光球状态更新其位置和动画：
 * - SUNSHINE_DOWN: 下落状态，从初始位置下落到地面
 * - SUNSHINE_GROUND: 地面状态，等待玩家收集
 * - SUNSHINE_COLLECT: 收集状态，飞向UI栏
 * - SUNSHINE_PRODUCT: 生产状态，向日葵产出的阳光飞向天空
 */
void updateSunshine() {
    for (int i = 0; i < ballMax; i++) {
        if (balls[i].isUsed) {
            balls[i].frame = (balls[i].frame + 1) % 29;  // 更新动画帧

            // 根据状态更新阳光球
            if (balls[i].state == SUNSHINE_DOWN) {
                // 下落状态：沿直线下落
                SunshineBall* sun = &balls[i];
                sun->timeT += (float)(sun->speed * 0.6);
                sun->currentPos = sun->controlP1 + sun->timeT * (sun->controlP4 - sun->controlP1);
                if (sun->timeT >= 1) {
                    sun->state = SUNSHINE_GROUND;
                    sun->timeT = 0;
                    sun->timer = 0;
                }
            } else if (balls[i].state == SUNSHINE_GROUND) {
                // 地面状态：计时等待收集
                balls[i].timer++;
                if (balls[i].timer > 300) {
                    if (balls[i].timer > 400) {
                        // 超过400帧自动消失
                        balls[i].isUsed = false;
                        balls[i].timer = 0;
                    }
                }
            } else if (balls[i].state == SUNSHINE_COLLECT) {
                // 收集状态：飞向UI栏
                SunshineBall* sun = &balls[i];
                sun->timeT += (float)sun->speed;
                sun->currentPos = sun->controlP1 + sun->timeT * (sun->controlP4 - sun->controlP1);
                if (sun->timeT > 1) {
                    sun->isUsed = false;
                    sunshine += 25;  // 收集成功，阳光+25
                    sun->timeT = 0;
                }
            } else if (balls[i].state == SUNSHINE_PRODUCT) {
                // 生产状态：贝塞尔曲线飞向天空
                SunshineBall* sun = &balls[i];
                sun->timeT += (float)sun->speed;
                sun->currentPos = calcBezierPoint(sun->timeT, sun->controlP1, sun->controlP2, sun->controlP3, sun->controlP4);
                if (sun->timeT > 1) {
                    sun->state = SUNSHINE_GROUND;
                    sun->timeT = 0;
                    sun->timer = 0;
                }
            }
        }
    }
}

// ============================================================================
//                              僵尸系统
// ============================================================================

/**
 * @brief 创建僵尸
 *
 * 按波次系统在屏幕右侧生成僵尸
 */
void createZombie() {
    // 警告期间不生成僵尸
    if (waveWarningActive) {
        return;
    }

    // 停止生成（当前波僵尸已全生成）
    if (zombiesSpawnedInWave >= zombiesPerWave) {
        return;
    }

    // 根据波次调整生成间隔（波次越高，间隔越短）
    int baseInterval = max(150, 400 - currentWave * 30);  // 最小150帧
    static int zmFre = baseInterval;
    static int count = 0;
    count++;

    if (count > zmFre) {
        count = 0;
        zmFre = baseInterval + rand() % 150;

        // 找一个未使用的僵尸
        for (int i = 0; i < ZOMBIE_MAX; i++) {
            if (!zms[i].used) {
                // 初始化僵尸属性
                zms[i].used = true;
                zms[i].x = WIN_WIDTH;
                zms[i].row = rand() % MAP_ROW;
                zms[i].y = 172 + (1 + zms[i].row) * 100;
                // 波次越高，僵尸速度越快
                zms[i].speed = 1;
                zms[i].blood = 100;
                zms[i].dead = false;
                zms[i].eating = false;
                zms[i].frameIndex = 0;
                zmCount++;
                zombiesSpawnedInWave++;
                break;
            }
        }
    }
}

/**
 * @brief 更新僵尸
 *
 * 处理僵尸的移动、动画更新和死亡判定
 */
void updateZombie() {
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    static int count = 0;
    count++;

    // 每5帧更新一次僵尸位置
    if (count > 5) {
        count = 0;
        for (int i = 0; i < zmMax; i++) {
            if (zms[i].used) {
                zms[i].x -= zms[i].speed;  // 向左移动

                // 僵尸到达左侧边界，游戏失败
                if (zms[i].x < 56) {
                    gameStatus = FAIL;
                }
            }
        }
    }

    // 每6帧更新一次僵尸动画
    static int count2 = 0;
    count2++;
    if (count2 > 6) {
        count2 = 0;
        for (int i = 0; i < zmMax; i++) {
            if (zms[i].used) {
                if (zms[i].dead) {
                    // 被土豆地雷炸死的僵尸不播放死亡动画，直接移除
                    if (zms[i].exploded) {
                        zms[i].used = false;
                        killCount++;
                        if (killCount == ZOMBIE_MAX) {
                            gameStatus = WIN;
                        }
                    } else {
                        // 普通死亡播放死亡动画
                        zms[i].frameIndex++;
                        if (zms[i].frameIndex >= 20) {
                            zms[i].used = false;
                            killCount++;
                            if (killCount == ZOMBIE_MAX) {
                                gameStatus = WIN;
                            }
                        }
                    }
                } else if (zms[i].eating) {
                    // 吃东西动画（21帧循环）
                    zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
                } else {
                    // 走路动画（22帧循环）
                    zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
                }
            }
        }
    }
}

// ============================================================================
//                              射击系统
// ============================================================================


/**
 * @brief 更新子弹
 *
 * 移动子弹，处理子弹飞出屏幕和爆炸动画
 */
void updateBullets() {
    int countMax = sizeof(bullets) / sizeof(bullets[0]);
    for (int i = 0; i < countMax; i++) {
        if (bullets[i].used) {
            bullets[i].x += bullets[i].speed;  // 移动子弹

            // 子弹飞出屏幕
            if (bullets[i].x > WIN_WIDTH) {
                bullets[i].used = false;
            }

            // 子弹爆炸动画
            if (bullets[i].blast) {
                bullets[i].frameIndex++;
                if (bullets[i].frameIndex >= 4) {
                    bullets[i].used = false;
                }
            }
        }
    }
}

// ============================================================================
//                              碰撞检测
// ============================================================================

/**
 * @brief 子弹击中僵尸检测
 *
 * 遍历所有子弹和僵尸，检测子弹是否击中僵尸
 */
void checkBullet_to_ZM() {
    int bCount = sizeof(bullets) / sizeof(bullets[0]);
    int zCount = sizeof(zms) / sizeof(zms[0]);

    for (int i = 0; i < bCount; i++) {
        // 跳过未使用或已爆炸的子弹
        if (bullets[i].used == false || bullets[i].blast) continue;

        for (int k = 0; k < zCount; k++) {
            // 跳过未使用的僵尸
            if (zms[k].used == false) continue;

            // 计算子弹和僵尸的碰撞区域
            int x1 = zms[k].x + 80;
            int x2 = zms[k].x + 110;
            int x = bullets[i].x;

            // 检测子弹是否击中僵尸
            if (zms[k].dead == false &&
                bullets[i].row == zms[k].row &&
                x > x1 && x < x2) {

                zms[k].blood -= 10;           // 僵尸受伤

                // 子弹击中僵尸后进入爆炸状态
                bullets[i].blast = true;
                bullets[i].frameIndex = 0;
                bullets[i].speed = 0;  // 停止移动

                // 僵尸死亡
                if (zms[k].blood <= 0) {
                    zms[k].dead = true;
                    zms[k].eating = false;
                    zms[k].speed = 0;
                    zms[k].frameIndex = 0;
                    zms[k].exploded = false;  // 重置爆炸标记，确保播放死亡动画
                }
            }
        }
    }
}

/**
 * @brief 僵尸吃植物检测
 *
 * 检测僵尸是否进入植物攻击范围，处理啃食逻辑
 */
void checkZombie_to_Plant() {
    int zCount = sizeof(zms) / sizeof(zms[0]);

    for (int i = 0; i < zCount; i++) {
        int row = zms[i].row;
        if (zms[i].dead) continue;  // 跳过死亡的僵尸

        for (int k = 0; k < MAP_COL; k++) {
            // 该位置没有植物或植物已死亡，跳过
            if (!map[row][k].plant || map[row][k].plant->isDead()) continue;

            // 计算植物和僵尸的位置
            int zhiWuX = 256 + k * 81;
            int x1 = zhiWuX + 10;     // 攻击范围左边界
            int x2 = zhiWuX + 60;    // 攻击范围右边界
            int x3 = zms[i].x + 80; // 僵尸身体X坐标

            // 僵尸在攻击范围内
            if (x3 > x1 && x3 < x2) {
                // 土豆地雷升起或准备好状态，触发爆炸
                Plant* plant = map[row][k].plant;
                if (plant->getType() == POTATO_MINE) {
                    PotatoMine* potato = static_cast<PotatoMine*>(plant);
                    PotatoMineState ps = potato->getState();
                    // 土豆地雷在 READY 或 RISING 状态且僵尸靠近时爆炸
                    // 使用 BLAST_RANGE 作为检测范围
                    if ((ps == PotatoMineState::READY || ps == PotatoMineState::RISING) && abs(x3 - zhiWuX) <= PotatoMine::getBlastRange()) {
                        potato->triggerExplosion();
                        continue;  // 爆炸后跳过啃食逻辑
                    }
                    // HIDDEN 状态不爆炸，让僵尸继续啃食
                    if (ps == PotatoMineState::HIDDEN) {
                        // 继续执行下面的啃食逻辑
                    } else {
                        continue;  // 其他状态跳过
                    }
                }

                if (zms[i].eating) {
                    // 僵尸正在吃东西
                    zms[i].eatingTime++;
                    if (zms[i].eatingTime >= 30) {  // 啃食30帧造成一次伤害
                        Plant* plant = map[row][k].plant;
                        if (plant != nullptr) {
                            plant->takeDamage(10);  // 植物受到10点伤害
                            zms[i].eatingTime = 0;

                            // 植物死亡
                            if (!plant->isAlive()) {
                                delete map[row][k].plant;
                                map[row][k].plant = nullptr;

                                // 该僵尸停止吃东西，继续前进
                                for (int j = 0; j < zCount; j++) {
                                    if (zms[j].eating && zms[j].row == row) {
                                        int zombieX = zms[j].x + 80;
                                        if (zombieX > x1 && zombieX < x2) {
                                            zms[j].eating = false;
                                            zms[j].speed = 1;
                                            zms[j].eatingTime = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // 僵尸开始吃东西
                    zms[i].eating = true;
                    cout << "ZOMBIE " << i << " START EATING at x=" << zms[i].x << " col=" << k << endl;
                    zms[i].speed = 0;
                    zms[i].eatingTime = 0;
                }
            }
        }
    }
}

/**
 * @brief 碰撞检测总函数
 *
 * 调用子弹-僵尸和僵尸-植物的碰撞检测
 */
void collisionCheek() {
    checkBullet_to_ZM();
    checkZombie_to_Plant();
}

// ============================================================================
//                              植物更新
// ============================================================================

// ============================================================================
//                              游戏主循环
// ============================================================================

/**
 * @brief 游戏逻辑更新
 *
 * 在固定时间步长下更新所有游戏逻辑
 * @param dt 时间步长（秒）
 */
void updateGame(double dt) {
    // UI更新
    updateCardCooldowns(dt);   // 更新卡牌冷却
    updateMessages(dt);         // 更新消息计时器
    updateWaveSystem(dt);       // 更新波次系统

    // 植物更新（多态调用每株植物的 update）
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            if (map[i][j].plant != nullptr && map[i][j].plant->isAlive()) {
                map[i][j].plant->update(dt);
            }
        }
    }

    updateExplosions(dt); // 更新爆炸效果
    updateMowers();      // 更新小推车
    createSunshine();    // 生成阳光
    updateSunshine();    // 更新阳光动画和状态

    createZombie();      // 生成僵尸
    updateZombie();      // 更新僵尸
    updateBullets();     // 更新子弹

    collisionCheek();    // 碰撞检测
}

// ============================================================================
//                              UI更新函数
// ============================================================================

/**
 * @brief 更新卡牌冷却
 */
void updateCardCooldowns(double dt) {
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        if (cardCooldown[i] > 0.0) {
            cardCooldown[i] -= (float)dt;
            if (cardCooldown[i] < 0.0) {
                cardCooldown[i] = 0.0;
            }
        }
    }
}

/**
 * @brief 更新消息计时器
 */
void updateMessages(double dt) {
    if (messageTimer > 0.0) {
        messageTimer -= (float)dt;
        if (messageTimer < 0.0) {
            messageTimer = 0.0;
        }
    }
}

/**
 * @brief 更新波次系统
 */
void updateWaveSystem(double dt) {
    // 波次警告计时
    if (waveWarningActive) {
        waveWarningTimer -= (float)dt;
        if (waveWarningTimer <= 0) {
            waveWarningActive = false;
        }
        return;
    }

    // 检查当前波僵尸是否都清理完
    int aliveZombies = 0;
    for (int i = 0; i < ZOMBIE_MAX; i++) {
        if (zms[i].used && !zms[i].dead) {
            aliveZombies++;
        }
    }

    // 如果没有存活的僵尸，且当前波僵尸已全生成，进入下一波
    if (aliveZombies == 0 && zombiesSpawnedInWave >= zombiesPerWave) {
        if (currentWave < totalWaves) {
            currentWave++;
            zombiesPerWave = 2 + currentWave;  // 每波僵尸递增: 1波=3, 2波=4, 3波=5...
            zombiesSpawnedInWave = 0;  // 重置当前波已生成计数
            waveWarningActive = true;
            waveWarningTimer = 3.0f;  // 3秒警告
        } else {
            // 所有波次完成，游戏胜利
            gameStatus = WIN;
        }
    }
}

/**
 * @brief 更新小推车
 *
 * 检测僵尸是否到达左侧边界，激活小推车
 * 激活后小推车向右移动并击杀经过的僵尸
 */
void updateMowers() {
    // 检测是否需要激活小推车
    for (int i = 0; i < ZOMBIE_MAX; i++) {
        if (zms[i].used && !zms[i].dead) {
            int row = zms[i].row;

            // 如果推车还没激活且僵尸到达小推车位置
            if (!mowerActivated[row] && zms[i].x < MOWER_X) {
                mowerActivated[row] = true;
                cout << "MOWER ACTIVATED on row " << row << endl;
            }
        }
    }

    // 更新已激活的小推车位置并检测碰撞
    for (int row = 0; row < MAP_ROW; row++) {
        if (mowerActivated[row]) {
            // 向右移动
            mowerX[row] += MOWER_SPEED;

            // 检测与僵尸的碰撞
            for (int i = 0; i < ZOMBIE_MAX; i++) {
                if (zms[i].used && !zms[i].dead && zms[i].row == row) {
                    int zmCenterX = zms[i].x + 40;  // 僵尸中心
                    int mowerFrontX = mowerX[row] + imgMower.getwidth();  // 推车前端

                    // 如果推车前端碰到僵尸
                    if (mowerFrontX >= zmCenterX - 20 && mowerX[row] <= zmCenterX + 20) {
                        zms[i].used = false;  // 直接移除僵尸（不播放死亡动画）
                        killCount++;  // 增加击杀计数
                        cout << "MOWER killed zombie " << i << ", killCount=" << killCount << endl;

                        // 检查是否胜利
                        if (killCount >= ZOMBIE_MAX) {
                            gameStatus = WIN;
                        }
                    }
                }
            }

            // 如果推车移出屏幕右侧，标记为已完成（不再显示）
            if (mowerX[row] > WIN_WIDTH) {
                mowerActivated[row] = false;  // 隐藏推车
            }
        }
    }
}

/**
 * @brief 更新爆炸效果
 */
void updateExplosions(double dt) {
    for (int i = 0; i < 10; i++) {
        if (explosions[i].used) {
            explosions[i].frameIndex++;
            if (explosions[i].frameIndex >= PotatoMine::getExplodeTime()) {
                explosions[i].used = false;
            }
        }
    }
}

/**
 * @brief 绘制爆炸效果
 */
void drawExplosions() {
    for (int i = 0; i < 10; i++) {
        if (explosions[i].used) {
            if (imgPotatoExplode.getwidth() > 0) {
                putimagePNG(explosions[i].x - 40, explosions[i].y - 40, &imgPotatoExplode);
            }
        }
    }
}

// ============================================================================
//                              UI界面
// ============================================================================

/**
 * @brief 开始界面
 *
 * 显示游戏开始界面，点击开始游戏
 */
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
            // 检测开始按钮点击
            if (msg.message == WM_LBUTTONDOWN &&
                msg.x > 474 && msg.x < 474 + 300 &&
                msg.y > 75 && msg.y < 75 + 140) {
                flag = 1;
            } else if (msg.message == WM_LBUTTONUP && flag) {
                EndBatchDraw();
                break;
            }
        }

        EndBatchDraw();
    }
}

/**
 * @brief 获取当前时间
 *
 * @return double 当前时间戳（秒）
 */
double getTime() {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

/**
 * @brief 显示场景
 *
 * 显示僵尸从右边走过来的动画
 */
void viewScence() {
    int xMin = WIN_WIDTH - imgBg.getwidth();
    vector2 points[9] = {
        {550, 80}, {530, 160}, {630, 170}, {530, 200}, {515, 270},
        {565, 370}, {605, 340}, {705, 280}, {690, 340}
    };
    int index[9];
    for (int i = 0; i < 9; i++) {
        index[i] = rand() % 11;
    }

    const int frameChangeRate = 3;
    const int sleepTime = 10;
    int frameCounter = 0;

    // 显示僵尸动画
    auto updateZombies = [&](int xOffset) {
        frameCounter++;
        BeginBatchDraw();
        putimage(xOffset, 0, &imgBg);

        for (int k = 0; k < 9; k++) {
            putimagePNG((int)((int)points[k].x - (int)xMin + (int)xOffset), (int)points[k].y, &imgZmStand[index[k]]);
            if (frameCounter >= frameChangeRate) {
                index[k] = (index[k] + 1) % 11;
            }
        }

        if (frameCounter >= frameChangeRate) {
            frameCounter = 0;
        }

        EndBatchDraw();
        Sleep(sleepTime);
    };

    // 僵尸进场
    for (int x = 0; x >= xMin; x -= 2) {
        updateZombies(x);
    }

    // 僵尸停留
    for (int i = 0; i < 100; i++) {
        updateZombies(xMin);
    }

    // 僵尸退场
    for (int x = xMin; x <= 0; x += 2) {
        updateZombies(x);
    }
}

/**
 * @brief 卡槽下移动画
 *
 * 游戏开始时，植物卡片从上方滑入
 */
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

/**
 * @brief 检查游戏是否结束
 *
 * @return true 游戏结束
 * @return false 游戏继续
 */
bool checkOver() {
    int ret = false;
    if (gameStatus == WIN) {
        loadimage(0, "res/win.png");
        Sleep(5000);
        ret = true;
    } else if (gameStatus == FAIL) {
        loadimage(0, "res/fail.png");
        Sleep(5000);
        ret = true;
    }
    return ret;
}

// ============================================================================
//                              程序入口
// ============================================================================

/**
 * @brief 主函数
 *
 * 游戏程序入口：
 * 1. 初始化游戏
 * 2. 显示开始界面
 * 3. 进入游戏主循环
 */
int main() {
    // 游戏初始化
    gameInit();

    // 显示开始界面
    startUI();

    // 显示场景动画
    viewScence();

    // 卡槽下移动画
    barsDown();

    // 游戏主循环
    const double TIMESTEP = 1.0 / 60.0;  // 60FPS时间步长
    double accumulator = 0.0;              // 时间累积器
    double lastTime = getTime();           // 上次时间
    bool gameIsRunning = true;            // 游戏运行标志

    while (gameIsRunning) {
        // 如果暂停，不更新游戏逻辑但继续渲染
        if (!gamePaused) {
            // 获取当前时间
            double currentTime = getTime();
            double deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            // 防止时间差过大（卡顿时）
            if (deltaTime > 0.25) {
                deltaTime = 0.25;
            }

            // 累积时间
            accumulator += deltaTime;

            // 固定时间步长更新游戏逻辑
            // 这样可以保证物理计算的一致性，不会因为帧率波动而出现bug
            while (accumulator >= TIMESTEP) {
                updateGame(TIMESTEP);
                accumulator -= TIMESTEP;
            }
        }

        // 渲染画面
        updateWindow();

        // 处理用户输入
        userClick();

        // 检查游戏是否结束
        if (checkOver()) {
            break;
        }
    }

    return 0;
}
