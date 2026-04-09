/**
 * @file vector2.cpp
 * @brief 二维向量实现文件
 *
 * 包含所有向量运算的实现
 */

#include <cmath>
#include "vector2.h"

/**
 * @brief 向量加法
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 两个向量相加的结果
 *
 * 计算方式：将两个向量的x、y分量分别相加
 */
vector2 operator +(vector2 x, vector2 y) {
    return vector2(x.x + y.x, x.y + y.y );
}

/**
 * @brief 向量减法
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 两个向量相减的结果
 *
 * 计算方式：将两个向量的x、y分量分别相减
 */
vector2 operator -(vector2 x, vector2 y) {
    return vector2(x.x - y.x, x.y - y.y);
}

/**
 * @brief 向量乘法（复数乘法）
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 两个向量相乘的结果（复数乘法）
 *
 * 计算方式：(x.x * y.x - x.y * y.y, x.y * y.x + x.x * y.y)
 *
 * 设计理由：这实际上是复数的乘法运算
 * 可以用于向量的旋转操作
 */
vector2 operator *(vector2 x, vector2 y) {
    return vector2(x.x * y.x - x.y * y.y, x.y * y.x + x.x * y.y);
}

/**
 * @brief 向量与标量乘法（右乘）
 * @param y 向量
 * @param x 标量
 * @return 缩放后的向量
 */
vector2 operator *(vector2 y, float x) {
    return vector2(x*y.x, x*y.y);
}

/**
 * @brief 向量与标量乘法（左乘）
 * @param x 标量
 * @param y 向量
 * @return 缩放后的向量
 */
vector2 operator *(float x, vector2 y) {
    return vector2(x * y.x, x * y.y);
}

/**
 * @brief 向量叉积（Cross Product）
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 两个向量的叉积
 *
 * 计算方式：x.x * y.y - x.y * y.x
 *
 * 几何意义：
 * - 结果为正表示y在x的逆时针方向
 * - 结果为负表示y在x的顺时针方向
 * - 结果的绝对值表示两个向量构成的平行四边形面积
 */
long long cross(vector2 x, vector2 y) { return x.y * y.x - x.x * y.y; }

/**
 * @brief 向量点积（Dot Product）
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 两个向量的点积
 *
 * 计算方式：x.x * y.x + x.y * y.y
 *
 * 几何意义：
 * - 可以判断两个向量的夹角（大于0为锐角，小于0为钝角，等于0为垂直）
 * - 可以计算向量投影长度
 */
long long dot(vector2 x, vector2 y) { return x.x * y.x + x.y * y.y; }

/**
 * @brief 四舍五入整数除法
 * @param a 被除数
 * @param b 除数
 * @return 四舍五入后的结果
 *
 * 算法：
 * 1. 处理负数情况（转正后计算，结果取反）
 * 2. (a + b/2) / b 实现四舍五入
 *
 * 设计理由：普通整数除法会直接截断小数
 * 这个实现会四舍五入，更符合直觉
 */
long long dv(long long a, long long b) {
    return b < 0 ? dv(-a, -b)
        : (a < 0 ? -dv(-a, b)
            : (a + b / 2) / b);
}

/**
 * @brief 向量模长的平方
 * @param x 向量
 * @return 模长的平方
 *
 * 计算方式：x.x * x.x + x.y * x.y
 *
 * 设计理由：计算平方根比较慢，先计算平方
 * 在只需要比较距离时使用平方可以避免开方
 */
long long len(vector2 x) { return x.x * x.x + x.y * x.y; }

/**
 * @brief 向量的模（长度）
 * @param x 向量
 * @return 向量的长度
 *
 * 计算方式：sqrt(x.x * x.x + x.y * x.y)
 *
 * 使用平方根函数计算向量的欧几里得长度
 */
long long dis(vector2 x) { return sqrt(x.x * x.x + x.y * x.y); }

/**
 * @brief 向量除法（复数除法）
 * @param x 被除向量
 * @param y 除数向量
 * @return 除法结果
 *
 * 计算方式：将向量除法转化为复数除法
 * 使用公式：x/y = (x * conjugate(y)) / |y|^2
 *
 * 设计理由：这是复数除法的向量形式
 * 可以用于向量的旋转（当y是单位向量时）
 */
vector2 operator /(vector2 x, vector2 y) {
    long long l = len(y);
    return vector2(dv(dot(x, y), l), dv(cross(x, y), l));
}

/**
 * @brief 向量取模
 * @param x 被取模向量
 * @param y 模数向量
 * @return 取模结果
 *
 * 计算方式：x - ((x / y) * y)
 * 相当于数学中的模运算：x mod y
 */
vector2 operator %(vector2 x, vector2 y) { return x - ((x / y) * y); }

/**
 * @brief 向量GCD（最大公约数）
 * @param x 第一个向量
 * @param y 第二个向量
 * @return 两个向量的GCD
 *
 * 使用欧几里得算法计算向量的最大公约数
 * 用于向量的归一化
 */
vector2 gcd(vector2 x, vector2 y) { return len(y) ? gcd(y, x % y) : x; }

/**
 * @brief 计算三次贝塞尔曲线上的一点
 * @param t 时间参数（0~1，0为起点，1为终点）
 * @param p0 控制点0（曲线起点）
 * @param p1 控制点1
 * @param p2 控制点2
 * @param p3 控制点3（曲线终点）
 * @return 曲线上对应t位置的点坐标
 *
 * 三次贝塞尔曲线公式：
 * B(t) = (1-t)^3 * P0 + 3*(1-t)^2*t * P1 + 3*(1-t)*t^2 * P2 + t^3 * P3
 *
 * 几何意义：
 * - t=0 时返回P0（起点）
 * - t=1 时返回P3（终点）
 * - P1、P2控制曲线的形状（控制点）
 *
 * 设计理由：贝塞尔曲线可以平滑地连接两个点
 * 用于阳光球飞向UI栏的轨迹动画，让飞行更自然
 */
vector2 calcBezierPoint(float t, vector2 p0, vector2 p1, vector2 p2, vector2 p3) {
    float u = 1 - t;       // 1-t，用于计算权重
    float tt = t * t;      // t^2
    float uu = u * u;     // (1-t)^2
    float uuu = uu * u;    // (1-t)^3
    float ttt = tt * t;    // t^3

    // 套用贝塞尔曲线公式
    vector2 p = uuu * p0;  // (1-t)^3 * P0
    p = p + 3 * uu * t * p1;   // + 3*(1-t)^2*t * P1
    p = p + 3 * u * tt * p2;   // + 3*(1-t)*t^2 * P2
    p = p + ttt * p3;          // + t^3 * P3

    return p;
}