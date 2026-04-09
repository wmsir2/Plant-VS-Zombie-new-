#include "tool.h"

/**
 * @file tool.cpp
 * @brief 工具函数实现文件
 *
 * 包含PNG图片绘制和时间延迟的实现
 */

 /**
  * @brief 内部函数：绘制PNG图片（核心透明度混合实现）
  * @param picture_x 图片X坐标
  * @param picture_y 图片Y坐标
  * @param picture 图片指针
  *
  * 实现原理：Alpha预乘混合算法
  * - 遍历图片每个像素
  * - 根据Alpha通道值混合前景色和背景色
  * - 公式：Cp = p*FP + (1-p)*BP
  *   - p: 源像素Alpha值（0~255）
  *   - FP: 前景色（源像素）
  *   - BP: 背景色（目标像素）
  *   - Cp: 混合结果
  *
  * 注意：此函数不处理边界裁剪，由putimagePNG处理
  */
void _putimagePNG(int picture_x, int picture_y, IMAGE* picture)
{
    // 获取屏幕和图片的数据缓冲区指针
    DWORD* dst = GetImageBuffer();    // 目标（屏幕）缓冲区
    DWORD* draw = GetImageBuffer();    // 绘制目标（屏幕）
    DWORD* src = GetImageBuffer(picture); //源图片缓冲区

    // 获取图片尺寸
    int picture_width = picture->getwidth();
    int picture_height = picture->getheight();

    // 获取屏幕尺寸
    int graphWidth = getwidth();
    int graphHeight = getheight();
    int dstX = 0;    // 目标缓冲区索引

    // 实现透明度混合：公式 Cp = p*FP + (1-p)*BP
    // p是Alpha值（0=完全透明，255=完全不透明）
    for (int iy = 0; iy < picture_height; iy++)
    {
        for (int ix = 0; ix < picture_width; ix++)
        {
            int srcX = ix + iy * picture_width; // 源图片像素索引

            // 提取源像素的ARGB分量
            int sa = ((src[srcX] & 0xff000000) >> 24); // Alpha通道
            int sr = ((src[srcX] & 0xff0000) >> 16);   // 红色分量
            int sg = ((src[srcX] & 0xff00) >> 8);      // 绿色分量
            int sb = src[srcX] & 0xff;                 // 蓝色分量

            // 边界检查：确保像素在屏幕范围内
            if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
            {
                // 计算目标像素在屏幕缓冲区中的位置
                dstX = (ix + picture_x) + (iy + picture_y) * graphWidth;

                // 提取目标像素的RGB分量
                int dr = ((dst[dstX] & 0xff0000) >> 16);
                int dg = ((dst[dstX] & 0xff00) >> 8);
                int db = dst[dstX] & 0xff;

                // Alpha预乘混合计算
                // 结果 = 源色 * Alpha/255 + 目标色 * (255-Alpha)/255
                draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)
                    | ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)
                    | (sb * sa / 255 + db * (255 - sa) / 255);
            }
        }
    }
}

/**
 * @brief 绘制PNG图片（对外接口，支持边界裁剪）
 * @param x 图片X坐标
 * @param y 图片Y坐标
 * @param picture 图片指针
 *
 * 功能说明：
 * 1. 支持PNG的Alpha通道透明度（32位PNG）
 * 2. 自动处理图片边界裁剪（不会绘制到屏幕外）
 * 3. 处理四种越界情况：上、下、左、右
 *
 * 裁剪逻辑：
 * - 如果图片顶部在屏幕外：裁剪图片顶部
 * - 如果图片底部在屏幕外：裁剪图片底部
 * - 如果图片左侧在屏幕外：裁剪图片左侧
 * - 如果图片右侧在屏幕外：裁剪图片右侧
 *
 * 设计理由：直接裁剪比检查后跳过更高效
 */
void putimagePNG(int x, int y, IMAGE* picture) {
    // 获取屏幕宽高
    int winWidth = getwidth();
    int winHeight = getheight();

    // 临时图像变量用于存储裁剪后的图片
    IMAGE imgTmp, imgTmp2, imgTmp3;

    // ---- 处理Y轴上边界（图片顶部在屏幕外）----
    if (y < 0) {
        int newHeight = picture->getheight() + y; // 计算裁剪后的高度
        if (newHeight > 0) {
            // 从原图指定位置截取
            SetWorkingImage(picture);
            getimage(&imgTmp, 0, -y, picture->getwidth(), newHeight);
            SetWorkingImage(); // 还原原始绘图环境
            y = 0;            // 将Y坐标重设为0
            picture = &imgTmp; // 更新图片指针指向裁剪后的图片
        }
        else {
            return; // 无效高度，不绘制
        }
    }

    // ---- 处理Y轴下边界（图片底部在屏幕外）----
    if (y + picture->getheight() > winHeight) {
        int newHeight = winHeight - y; // 计算可绘制的高度
        if (newHeight > 0 && newHeight <= picture->getheight()) {
            SetWorkingImage(picture);
            getimage(&imgTmp, 0, 0, picture->getwidth(), newHeight);
            SetWorkingImage();
            picture = &imgTmp; // 更新为裁剪后的图片
        }
        else {
            return; // 无效高度，不绘制
        }
    }

    // ---- 处理X轴左边界（图片左侧在屏幕外）----
    if (x < 0) {
        int newWidth = picture->getwidth() + x; // 计算可绘制的宽度
        if (newWidth > 0) {
            SetWorkingImage(picture);
            getimage(&imgTmp2, -x, 0, newWidth, picture->getheight());
            SetWorkingImage();
            x = 0;
            picture = &imgTmp2;
        }
        else {
            return; // 无效宽度，不绘制
        }
    }

    // ---- 处理X轴右边界（图片右侧在屏幕外）----
    if (x + picture->getwidth() > winWidth) {
        int newWidth = winWidth - x; // 计算可绘制的宽度
        if (newWidth > 0) {
            SetWorkingImage(picture);
            getimage(&imgTmp3, 0, 0, newWidth, picture->getheight());
            SetWorkingImage();
            picture = &imgTmp3;
        }
        else {
            return; // 无效宽度，不绘制
        }
    }

    // 调用核心绘制函数
    _putimagePNG(x, y, picture);
}

/**
 * @brief 获取距离上次调用的毫秒数
 * @return 上次调用到这次调用经过的毫秒数
 *
 * 功能说明：
 * - 第一次调用返回0（因为没有"上次"时间）
 * - 之后的调用返回与上次的时间差
 *
 * 设计理由：
 * - 用于帧率控制和时间测量
 * - 使用静态变量保持上次时间
 * - Windows API GetTickCount() 返回毫秒数
 *
 * 使用示例：
 * int delta = getDelay();  // 获取帧间隔
 * // 根据delta调整动画速度
 */
int getDelay() {
    static unsigned long long lastTime = 0;  // 上次时间（静态变量，持久化）
    unsigned long long currentTime = GetTickCount();  // 当前时间（毫秒）

    if (lastTime == 0) {
        // 首次调用，初始化时间
        lastTime = currentTime;
        return 0;
    }
    else {
        // 计算时间差
        int ret = currentTime - lastTime;
        lastTime = currentTime;  // 更新上次时间
        return ret;
    }
}