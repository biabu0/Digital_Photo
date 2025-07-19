#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <config.h>
#include <input_manager.h>
#include <disp_manager.h>
#include <render.h>
#include <pthread.h>
/******************************************************
 * @brief  实现鼠标输入设备
 * 
 * @author  bia布
 * @date    2025/07/18
 * @version 1.0
 ******************************************************/

// 鼠标图标的图形表示
#define MOUSE_XRES  (12)
#define MOUSE_YRES  (16)
static char map_mouse [MOUSE_XRES * MOUSE_YRES] = {
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
};

// 鼠标描述结构体，显存用于显示鼠标图形
typedef struct FbTmpBuf {
    int x;         // x 坐标
    int y;         // y 坐标
    int xlen;      // x 方向字节长度  xlen = xres * bpp / 8
    int ylen;      // y 方向高度     ylen = yres
    char* buf;     // 一小块 "显存"
}T_FbTmpBuf, *PT_FbTmpBuf;

static int mouse_visible = 0;

#define MOUSE_DEVICE_NAME "/dev/input/event3"
static int g_fb_fd;
static int g_mouse_fd;


// 显示屏参数
static struct fb_var_screeninfo g_tFBVar;
static struct fb_fix_screeninfo g_tFBFix;
static unsigned char *g_pucFBMem;
static unsigned int g_dwScreenSize;
static unsigned int g_dwLineWidth;
static unsigned int g_dwPixelWidth;

// 鼠标起始位置
int mouse_x, mouse_y;
static T_FbTmpBuf lastfb_buf;




// 实现fb初始化获取屏幕的参数
static int fbDeviceInit(void){
	int ret;
	g_fb_fd = open(FB_DEVICE_NAME, O_RDWR);
	if(g_fb_fd < 0){
		DBG_PRINTF(APP_ERR"Can't open frame buffer device: %s\n", FB_DEVICE_NAME);
	}
	ret = ioctl(g_fb_fd, FBIOGET_VSCREENINFO, &g_tFBVar);
	if (ret < 0)
	{
		DBG_PRINTF("can't get fb's var\n");
		return -1;
	}

	ret = ioctl(g_fb_fd, FBIOGET_FSCREENINFO, &g_tFBFix);
	if (ret < 0)
	{
		DBG_PRINTF("can't get fb's fix\n");
		return -1;
	}
	if(g_tFBVar.bits_per_pixel != 16 && g_tFBVar.bits_per_pixel != 24 && g_tFBVar.bits_per_pixel != 32) {
        DBG_PRINTF("Unsupported bits_per_pixel: %d\n", g_tFBVar.bits_per_pixel);
		return -1;
    }
	g_dwScreenSize = g_tFBVar.xres * g_tFBVar.yres * g_tFBVar.bits_per_pixel / 8;
	// 存储映射IO
	g_pucFBMem = (unsigned char *)mmap(NULL,g_dwScreenSize,PROT_READ|PROT_WRITE,MAP_SHARED, g_fb_fd, 0);
	if(-1 == (int)g_pucFBMem){
		DBG_PRINTF(APP_ERR"Can't mmap frame buffer device: %s\n", FB_DEVICE_NAME);
		return -1;
	}
	g_dwLineWidth  = g_tFBVar.xres * g_tFBVar.bits_per_pixel / 8;
	g_dwPixelWidth = g_tFBVar.bits_per_pixel / 8;
	
	return 0;
}

static int MouseDevInit(void){
	int ret;
	g_mouse_fd = open(MOUSE_DEVICE_NAME, O_RDONLY);
	if(g_mouse_fd < 0){
		DBG_PRINTF(APP_ERR"Can't open mouse device: %s\n", MOUSE_DEVICE_NAME);
		return -1;
	}
	// 初始化帧缓冲设备，用于鼠标的定位
	if(fbDeviceInit()){
		DebugPrint(APP_ERR"file : %s,%s FBDeviceInit error! \n",__FILE__,__FUNCTION__);
		close(g_mouse_fd);
		return -1;
	}

	// 鼠标起始位置：屏幕中间位置
	mouse_x = g_tFBVar.xres >> 1;
	mouse_y = g_tFBVar.yres >> 1;
	lastfb_buf.x = g_tFBVar.xres;
	lastfb_buf.y = g_tFBVar.yres;
	lastfb_buf.xlen = MOUSE_XRES * g_dwPixelWidth;
	lastfb_buf.ylen = MOUSE_YRES;
	lastfb_buf.buf = (char *)malloc(lastfb_buf.xlen * lastfb_buf.ylen);

	return 0;
}

static int MouseDevExit(void){
	close(g_mouse_fd);
	close(g_fb_fd);
	return 0;
}



static void draw_mouse(unsigned char * dst, int row, int len)
{
    int i = 0, j = 0;
    char * map = map_mouse + row * 12;
    int __len = len / g_dwPixelWidth;
    while (i < __len) {
        if(map[i] == 1) {
            dst[j++] = 0xFF;
            dst[j++] = 0xFF;
        }
        else
            j += 2;
        i++;
    }
}
/*
设计目的：实现无闪烁的鼠标移动
问题背景：直接绘制鼠标光标会导致旧位置的图像被覆盖，移动时可能出现残影。
解决方案：
保存旧位置数据：在移动前，将旧位置的屏幕内容保存到 lastfb_buf。
恢复旧位置图像：移动后，将 lastfb_buf 中的旧数据写回显存，覆盖鼠标光标残留。
绘制新位置鼠标：将新位置的屏幕内容保存到 lastfb_buf，并调用 draw_mouse 绘制光标。

lastfb_buf 是实现鼠标移动时屏幕内容平滑更新的核心机制，
通过保存旧位置数据、恢复旧位置图像、更新新位置数据三步操作，确保鼠标移动无残留且高效。
*/
static void mouse_move(int dx, int dy, int isJmp){
	int iXstart, iXend;
	int iYstart, iYend;
	int i;
	mouse_x = mouse_x + dx;
	if(mouse_x < 0) mouse_x = 0;
	if(mouse_x >= g_tFBVar.xres) mouse_x = g_tFBVar.xres -1;
	mouse_y += dy;
	if (mouse_y < 0) mouse_y = 0;
    if (mouse_y >= g_tFBVar.yres) mouse_y = g_tFBVar.yres - 1;
	// 如果旧位置超过屏幕，或者是强制跳转绘制，则上次保存的鼠标位置无效，跳转到新的位置重新绘制
	// isJmp参数为真时表示非连续移动（如鼠标初始化/位置重置），跳过显存恢复步骤直接绘制新光标
	if(lastfb_buf.x >= g_tFBVar.xres || lastfb_buf.y >= g_tFBVar.yres || isJmp)
        goto LABLE_MOVE;
	iXstart = lastfb_buf.x * g_dwPixelWidth;
	iXend = iXstart + lastfb_buf.xlen;
	if(iXend > g_dwLineWidth) iXend = g_dwLineWidth;
	iYstart = lastfb_buf.y;
	iYend = iYstart + lastfb_buf.ylen;
	if(iYend > g_tFBVar.yres) iYend = g_tFBVar.yres;
	// 将lastfb_buf保存的旧光标区域背景写回显存
	for(i = 0;iYstart < iYend; iYstart++) {
        memcpy(g_pucFBMem + iXstart + iYstart * g_dwLineWidth,
               lastfb_buf.buf + i++ * lastfb_buf.xlen,
               iXend - iXstart);
    }

LABLE_MOVE:
// 更新 lastfb_buf 的坐标为当前鼠标位置
    lastfb_buf.x = mouse_x;
    lastfb_buf.y = mouse_y;
	// 计算新位置的显存区域
	iXstart = lastfb_buf.x * g_dwPixelWidth;
	iXend = iXstart + lastfb_buf.xlen;
	if(iXend > g_dwLineWidth) iXend = g_dwLineWidth;
	iYstart = lastfb_buf.y;
	iYend = iYstart + lastfb_buf.ylen;
	if(iYend > g_tFBVar.yres) iYend = g_tFBVar.yres;
	// 将显存新位置的数据保存到 lastfb_buf（覆盖旧数据）
	for(i = 0; iYstart < iYend; iYstart++){
		// 将显存中的图片数据拷贝到鼠标缓存中
		memcpy(lastfb_buf.buf + i * lastfb_buf.xlen, g_pucFBMem + iXstart + iYstart * g_dwLineWidth, iXend - iXstart);
		// 在显存新位置绘制鼠标
		draw_mouse(g_pucFBMem + iXstart + iYstart * g_dwLineWidth, i, iXend - iXstart);
		i++;
	}
}


static int mouse_rel_event(__u16 code, __s32 value){
	// 鼠标x,y轴的移动事件，相对位移量value
	if(code == REL_X){
		mouse_move((signed char)(value & 0xFF), 0, 0);
		return 1;
	}else if(code == REL_Y){
		mouse_move(0, (signed char)(value & 0xFF), 0);
		return 1;
	}else if(code == REL_WHEEL){
		// 滚轮事件
	}
	return 0;

}


static int MouseGetInputEvent(PT_InputEvent ptInputEvent)
{
	/*
	//Linux输入子系统中，struct input_event 是一个核心数据结构，用于表示输入设备（如鼠标、键盘、触摸板等）产生的事件。
	struct input_event {    // <linux/input.h> 中定义
		struct timeval time;   // 事件发生的时间
		__u16 type;            // 事件类型如 EV_KEY EV_REL
		__u16 code;            // 事件代码如 REL_X REL_Y BTN_LEFT
		__s32 value;           // 事件的值
	};*/
	struct input_event tEvent;
	int iLen;
	static int lastPressure = 0;
 
	while (1)
	{
		iLen = read(g_mouse_fd, &tEvent, sizeof(struct input_event)); /* 如果无数据则休眠 */
		if (iLen != sizeof(struct input_event)){
			DBG_PRINTF(APP_ERR"read input_event form mouse failed.\n");
			break;
		}
		
		if (!tEvent.type){
			// 为0的时候是同步事件，无需关注
			continue;
		}
		switch(tEvent.type){
			case EV_KEY:
				// 鼠标左键事件
				if(tEvent.code == BTN_LEFT){
					mouse_visible = 1;
					ptInputEvent->tTime = tEvent.time;
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					ptInputEvent->iPressure = lastPressure = tEvent.value;
					ptInputEvent->iX = lastfb_buf.x;
					ptInputEvent->iY = lastfb_buf.y;
					return 0;
				}
				break;
			case EV_REL:
				// 鼠标移动事件
				if(mouse_rel_event(tEvent.code, tEvent.value)){
					mouse_visible = 1;
					ptInputEvent->tTime = tEvent.time;
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					ptInputEvent->iPressure = lastPressure;
					ptInputEvent->iX = lastfb_buf.x;
					ptInputEvent->iY = lastfb_buf.y;
					return 0;
				}
				break;
			default:
				break;
		}
	}
	return -1;
}

// 强制重绘鼠标光标
static void mouse_render_event(void){
	if(mouse_visible){
		mouse_move(0, 0 , 1);
	}
}

static T_InputOpr g_tMouseOpr = {
	.name          = "Mouse",
	.DeviceInit    = MouseDevInit,
	.DeviceExit    = MouseDevExit,
	.GetInputEnvent = MouseGetInputEvent,
};
int MouseInit(void){
	if(register_render_event(mouse_render_event)) {
        DBG_PRINTF("MouseInit register_render_event error !\n");
        return -1;
    }
	return RegisterInputOpr(&g_tMouseOpr);
}