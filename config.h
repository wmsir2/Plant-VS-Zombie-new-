#pragma once

#include <iostream>
#include "config.h"
using namespace std;

/**
 * @file config.h
 * @brief 游戏全局配置头文件
 *
 * 本文件包含所有游戏相关的：
 * - 宏定义（窗口大小、网格尺寸、速度等）
 * - 枚举定义（阳光状态、植物类型、游戏状态等）
 * - 全局变量声明（击杀数、僵尸数、波次系统等）
 *
 * 设计理由：集中管理游戏配置，便于调整游戏平衡性和调试
 */

/* ==================== 窗口与地图尺寸 ==================== */

// 窗口尺寸：宽900像素，高600像素
// 这是游戏的主画面大小，UI和地图都在这个范围内
#define WIN_WIDTH    900
#define WIN_HEIGHT   600

// 地图网格：3行 x 9列
// 每行对应一个植物种植槽位，每列是一块可种植物的土地
#define MAP_ROW      3
#define MAP_COL      9

/* ==================== 对象数量限制 ==================== */

// 阳光球最大数量：场景中同时存在的阳光球上限
// 避免阳光太多导致界面混乱，也控制内存使用
#define ballMax      10

// 僵尸最大数量：同时存在的僵尸上限
// 限制同时面对的僵尸数量，保持游戏可玩性
#define ZOMBIE_MAX   10

/* ==================== 动画帧数配置 ==================== */

// 通用动画帧数：大部分动画默认60帧循环
#define TOTAL_FRAMES 60

// 僵尸动画帧数：走路、吃东西、死亡各有不同帧数
#define ZOMBIE_EATING_FRAMES 60   // 僵尸吃东西动画帧数
#define ZOMBIE_WALKING_FRAMES 60  // 僵尸走路动画帧数
#define ZOMBIE_DEAD_FRAMES 60     // 僵尸死亡动画帧数

/* ==================== 阳光球状态枚举 ==================== */
/**
 * @brief 阳光球生命周期状态机
 *
 * 阳光球从生成到消失经历4个状态：
 *
 * [自然下落] SUNSHINE_DOWN
 *    ↓ (到达地面)
 * [地面等待] SUNSHINE_GROUND
 *    ↓ (玩家点击收集)
 * [飞向UI] SUNSHINE_COLLECT
 *    ↓ (到达阳光池)
 *
 * [生产状态] SUNSHINE_PRODUCT (向日葵专用)
 *    ↓ (贝塞尔曲线飞向天空)
 * [地面等待] SUNSHINE_GROUND
 */
enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };

/* ==================== 植物类型枚举 ==================== */
/**
 * @brief 植物类型定义
 *
 * 注意：索引从0开始，这与UI卡片的排列顺序一致
 * - WAN_DOU (0): 豌豆射手，攻击植物
 * - XIANG_RI_KUI (1): 向日葵，生产阳光
 * - TU_DAO (2): 土豆地雷，陷阱植物
 * - ZHI_WU_COUNT: 植物总数，用于数组大小定义
 */
enum { WAN_DOU, XIANG_RI_KUI, TU_DAO, ZHI_WU_COUNT };

/* ==================== 游戏状态枚举 ==================== */
/**
 * @brief 游戏运行状态
 *
 * - GOING: 游戏正在进行中
 * - WIN: 玩家获胜（击杀所有波次僵尸）
 * - FAIL: 玩家失败（僵尸到达左侧边界）
 */
enum { GOING, WIN, FAIL };

/* ==================== 全局统计变量 ==================== */
// 这些变量在main.cpp中定义，用于跟踪游戏状态

static int killCount;   // 已击杀僵尸总数（用于判断胜利）
static int zmCount;     // 已生成僵尸总数（用于统计）
static int gameStatus;  // 当前游戏状态（GOING/WIN/FAIL）

/* ==================== 波次系统变量 ==================== */
/**
 * @brief 波次系统说明
 *
 * 游戏分为5波，每波僵尸数量递增：
 * - 第1波：3只僵尸
 * - 第2波：4只僵尸（每波+1）
 * - ...
 * - 第5波：7只僵尸
 *
 * 每波之间有3秒警告时间，用于玩家准备
 */
extern int currentWave;           // 当前波次 (1~5)
extern int totalWaves;            // 总波次数
extern int zombiesPerWave;        // 当前波僵尸数量
extern int zombiesSpawnedInWave;  // 当前波已生成的僵尸数
extern bool waveWarningActive;    // 是否处于波次警告阶段
extern float waveWarningTimer;    // 警告倒计时

/* ==================== UI常量 ==================== */

/**
 * @brief 植物卡片栏配置
 *
 * 卡片区域位于屏幕顶部，x=338开始，共3张卡片
 * 每张卡片宽65像素，高90像素，间距65像素
 */
#define CARD_START_X    338   // 第一张卡片的X坐标
#define CARD_START_Y    5     // 卡片Y坐标（距顶部5像素）
#define CARD_WIDTH      65    // 卡片宽度
#define CARD_HEIGHT     90    // 卡片高度
#define CARD_SPACING    65    // 卡片间距

/* ==================== 植物价格 ==================== */
/**
 * @brief 植物阳光消耗
 *
 * 玩家点击卡片时扣除对应阳光，右键取消时返还
 * - 豌豆射手：100阳光（攻击型，需要更多阳光）
 * - 向日葵：50阳光（辅助型，产阳光回收快）
 * - 土豆地雷：25阳光（陷阱型，最便宜）
 */
#define PEASHOOTER_COST 100
#define SUNFLOWER_COST  50
#define TUDAO_COST      25

/* ==================== 小推车/除草机配置 ==================== */
/**
 * @brief 小推车系统说明
 *
 * 每行左侧有一辆除草机，是最后一道防线：
 * - 当僵尸到达推车位置时，推车自动激活
 * - 推车向右移动并击杀经过的僵尸（一击必杀）
 * - 推车只能使用一次，激活后移出屏幕
 */
#define MOWER_X         170   // 推车初始X坐标（屏幕左边界+20）
#define MOWER_Y_BASE    193   // 第一行推车Y坐标（与植物行对齐）
#define MOWER_ROW_SPACE 102   // 每行推车间距（与植物行距一致）
#define MOWER_SPEED     8     // 推车激活后移动速度（像素/帧）

/* ==================== 暂停按钮 ==================== */
// 暂停按钮位于屏幕右上角
#define PAUSE_BTN_X     860
#define PAUSE_BTN_Y     60
#define PAUSE_BTN_SIZE  40

/* ==================== 消息显示 ==================== */
// 消息提示显示时长（秒）
#define MSG_DURATION    2.0f