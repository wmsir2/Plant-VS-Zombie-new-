#pragma once
#include <cmath>

/**
 * @file vector2.h
 * @brief 二维向量结构体定义
 *
 * 二维向量用于表示游戏中的位置、速度等：
 * - 包含x、y两个分量
 * - 支持向量运算（加、减、乘、除、点积、叉积）
 * - 提供贝塞尔曲线计算功能
 *
 * 设计理由：将向量运算封装成结构体，便于代码复用和数学计算
 * 游戏中的很多逻辑都涉及向量运算（飞行轨迹、碰撞检测等）
 */

/**
 * @struct vector2
 * @brief 二维向量结构体
 *
 * 表示二维平面上的一个点或向量：
 * - x: X坐标/分量
 * - y: Y坐标/分量
 *
 * 支持的运算：
 * - 加法：向量相加
 * - 减法：向量相减
 * - 乘法：向量与标量相乘（支持左右两种顺序）
 * - 除法：向量除以向量（用于复数除法）
 * - 取模：向量取模运算
 */
struct vector2 {
    /**
     * @brief 默认构造函数
     * @param _x X坐标
     * @param _y Y坐标
     */
    vector2(int _x = 0, int _y = 0) : x(_x), y(_y) {}

    /**
     * @brief 从数组构造
     * @param data 包含两个int的数组 [x, y]
     */
    vector2(int* data) : x(data[0]), y(data[1]) {}

    long long x;  /**< X分量 */
    long long y;  /**< Y分量 */
};

// -------------------- 向量运算运算符 --------------------

// 加法：两个向量相加
vector2 operator +(vector2 x, vector2 y);

// 减法：两个向量相减
vector2 operator -(vector2 x, vector2 y);

// 乘法：两个向量相乘（复数乘法，用于向量旋转）
vector2 operator *(vector2 x, vector2 y);

// 乘法：向量与标量相乘
vector2 operator *(vector2, float);
vector2 operator *(float, vector2);

// -------------------- 向量数学运算 --------------------

/**
 * @brief 叉积（Cross Product）
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 叉积结果
 *
 * 计算方式：x.x * y.y - x.y * y.x
 * 几何意义：两个向量构成的平行四边形面积
 */
long long cross(vector2 x, vector2 y);

/**
 * @brief 点积（Dot Product）
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 点积结果
 *
 * 计算方式：x.x * y.x + x.y * y.y
 * 几何意义：用于判断向量夹角、投影等
 */
long long dot(vector2 x, vector2 y);

/**
 * @brief 四舍五入除法
 * @param a 被除数
 * @param b 除数
 * @return 四舍五入后的结果
 *
 * 设计理由：普通整数除法会截断小数，四舍五入更符合直觉
 */
long long dv(long long a, long long b);

// -------------------- 向量属性 --------------------

/**
 * @brief 向量模长的平方
 * @param x 向量
 * @return 模长的平方（用于避免开方运算）
 */
long long len(vector2 x);

/**
 * @brief 向量的模（长度）
 * @param x 向量
 * @return 向量的长度
 *
 * 计算方式：sqrt(x.x * x.x + x.y * x.y)
 */
long long dis(vector2 x);

// -------------------- 向量除法和取模 --------------------

// 向量除法：复数除法（用于向量旋转）
vector2 operator /(vector2 x, vector2 y);

// 向量取模：向量对向量取模
vector2 operator %(vector2 x, vector2 y);

/**
 * @brief 向量最大公约数（用于向量归一化）
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 两个向量的GCD
 */
vector2 gcd(vector2 x, vector2 y);

// -------------------- 贝塞尔曲线 --------------------

/**
 * @brief 计算贝塞尔曲线上的一点
 * @param t 时间参数（0~1）
 * @param p0 控制点0（曲线起点）
 * @param p1 控制点1
 * @param p2 控制点2
 * @param p3 控制点3（曲线终点）
 * @return 曲线上对应t位置的点坐标
 *
 * 使用三次贝塞尔曲线公式：
 * B(t) = (1-t)^3 * P0 + 3*(1-t)^2*t * P1 + 3*(1-t)*t^2 * P2 + t^3 * P3
 *
 * 设计理由：贝塞尔曲线可以平滑地连接两个点
 * 用于阳光球飞向UI栏的轨迹动画
 */
vector2 calcBezierPoint(float t, vector2 p0, vector2 p1, vector2 p2, vector2 p3);