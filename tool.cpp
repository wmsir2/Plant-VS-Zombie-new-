#include "tool.h"

// ����PNGͼ��ȥ͸������
void _putimagePNG(int  picture_x, int picture_y, IMAGE* picture) //xΪ����ͼƬ��X���꣬yΪY����
{
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()���������ڻ�ȡ��ͼ�豸���Դ�ָ�룬EASYX�Դ�
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //��ȡpicture���Դ�ָ��
	int picture_width = picture->getwidth(); //��ȡpicture�Ŀ��ȣ�EASYX�Դ�
	int picture_height = picture->getheight(); //��ȡpicture�ĸ߶ȣ�EASYX�Դ�
	int graphWidth = getwidth();       //��ȡ��ͼ���Ŀ��ȣ�EASYX�Դ�
	int graphHeight = getheight();     //��ȡ��ͼ���ĸ߶ȣ�EASYX�Դ�
	int dstX = 0;    //���Դ������صĽǱ�

	// ʵ��͸����ͼ ��ʽ�� Cp=��p*FP+(1-��p)*BP �� ��Ҷ˹���������е���ɫ�ĸ��ʼ���
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //���Դ������صĽǱ�
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA��͸����
			int sr = ((src[srcX] & 0xff0000) >> 16); //��ȡRGB���R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //���Դ������صĽǱ�
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)
					| (sb * sa / 255 + db * (255 - sa) / 255);
			}
		}
	}
}

void putimagePNG(int x, int y, IMAGE* picture) {
    // 获取屏幕宽高
    int winWidth = getwidth();
    int winHeight = getheight();

    // 图像临时变量
    IMAGE imgTmp, imgTmp2, imgTmp3;

    // 如果图像的 y 坐标超出上边界，则需要裁剪图像
    if (y < 0) {
        int newHeight = picture->getheight() + y; // 计算新高度
        if (newHeight > 0) {
            SetWorkingImage(picture);
            getimage(&imgTmp, 0, -y, picture->getwidth(), newHeight);
            SetWorkingImage(); // 还原原始绘图环境
            y = 0; // 将 y 坐标重设为 0
            picture = &imgTmp; // 更新图片指针
        }
        else {
            return; // 如果新高度无效，则不绘制
        }
    }

    // 如果图片的 y 坐标超出下边界
    if (y + picture->getheight() > winHeight) {
        int newHeight = winHeight - y; // 计算新高度
        if (newHeight > 0 && newHeight <= picture->getheight()) { // 确保 newHeight 在图片高度范围内
            SetWorkingImage(picture);
            getimage(&imgTmp, 0, 0, picture->getwidth(), newHeight);
            SetWorkingImage(); // 还原绘图环境
            picture = &imgTmp; // 更新图片指针为裁剪后的图像
        }
        else {
            return; // 如果 newHeight 无效则不绘制
        }
    }


    // 如果 x 坐标超出左边界
    if (x < 0) {
        int newWidth = picture->getwidth() + x; // 计算新宽度
        if (newWidth > 0) {
            SetWorkingImage(picture);
            getimage(&imgTmp2, -x, 0, newWidth, picture->getheight());
            SetWorkingImage();
            x = 0;
            picture = &imgTmp2;
        }
        else {
            return; // 无效则不绘制
        }
    }

    // 如果 x 坐标超出右边界
    if (x + picture->getwidth() > winWidth) {
        int newWidth = winWidth - x; // 计算新宽度
        if (newWidth > 0) {
            SetWorkingImage(picture);
            getimage(&imgTmp3, 0, 0, newWidth, picture->getheight());
            SetWorkingImage();
            picture = &imgTmp3;
        }
        else {
            return; // 无效则不绘制
        }
    }

    // 最终绘制
    _putimagePNG(x, y, picture);
}


int getDelay() {
	static unsigned long long lastTime = 0;
	unsigned long long currentTime = GetTickCount();
	if (lastTime == 0) {
		lastTime = currentTime;
		return 0;
	}
	else {
		int ret = currentTime - lastTime;
		lastTime = currentTime;
		return ret;
	}
}