#ifndef _RENDER_H_
#define _RENDER_H_

#include<pic_operation.h>
#include<disp_manager.h>
#include<page_manager.h>

int PicMerge(int iX, int iY, PT_PixelDatas ptSmallPic, PT_PixelDatas ptBigPic);
int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic);
void FlushVideoMemToDev(PT_VideoMem ptVideoMem);
int GetPixelDatasFrmBMP(char *strFileName, PT_PixelDatas ptPixelDatas);
void FreePixeDatasForIcon(PT_PixelDatas ptPixelDatas);
void ReleaseButton(PT_Layout ptLayout);
void PressButton(PT_Layout ptLayout);
#endif