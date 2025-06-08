#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include <pic_operation.h>



// 内存块的状态
typedef enum{
	VMS_FREE = 0,				//空闲
	VMS_USED_FOR_PREPARE,		//该内存给页面管理的子线程prepare使用
	VMS_USED_FOR_CUR, 			//该主线程使用
}E_VideoMemState;

// 内存块的数据是否准备好
typedef enum{
	PDS_BLANK = 0,
	PDS_GENERATING,
	PDS_GENERATED,
}E_PicDataState;

// 内存块的结构体
typedef struct VideoMem{
	int iID;							//记录内存块的标识
	int iDevFrameBuffer;				//是否是设备的framebuffer//将framebuffer也加入到链表中
	E_VideoMemState eVideoMemState;		//内存块是否被使用以及被谁使用
	E_PicDataState ePicDataState;		//内存块数据状态
	T_PixelDatas tPixelDatas;			//内存数据的分辨率和位宽
	struct VideoMem *ptNext;			//链表指针
}T_VideoMem, *PT_VideoMem;


typedef struct DispOpr {
	char *name;
	int iXres;
	int iYres;
	int iBpp;
	int iLineWidth;
	unsigned char *pucDispMem;
	int (*DeviceInit)(void);
	int (*ShowPixel)(int iPenX, int iPenY, unsigned int dwColor);
	int (*CleanScreen)(unsigned int dwBackColor);
	int (*ShowPage)(PT_VideoMem ptVideoMem);
	struct DispOpr *ptNext;
}T_DispOpr, *PT_DispOpr;




int RegisterDispOpr(PT_DispOpr ptDispOpr);
void ShowDispOpr(void);
int DisplayInit(void);
int AllocVideoMem(int iNum);
int GetDispResolution(int *piXres, int *piYres, int *piBpp);
int FBInit(void);
void SelectAndInitDefaultDispDev(char *name);
PT_VideoMem GetVideoMem(int iID, int iCur);
void PutVideoMem(PT_VideoMem ptVideoMem);
PT_DispOpr GetDefaultDispDev(void);

#endif /* _DISP_MANAGER_H */

