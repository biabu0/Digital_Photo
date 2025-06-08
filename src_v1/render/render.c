#include<config.h>
#include<page_manager.h>
#include<disp_manager.h>
#include<file.h>
#include<stdio.h>
#include<stdlib.h>
#include<render.h>


// bmp.c中的
extern T_PicFileParser g_tBMPParser;

// int GetFontPixel(){

// }

// int GetPicturePixel(){

// }

// int DrawPixel(){
    
// }

void FlushVideoMemToDev(PT_VideoMem ptVideoMem){

    // 获取当前使用的设备
    PT_DispOpr ptDefaultDisoOpr = GetDefaultDispDev();
    //lcd存在显存，但不同设备其显存操作方式可能不一样
    //memcpy(ptDefaultDisoOpr->pucDispMem, ptVideoMem->tPixelDatas.aucPixelDatas, ptVideoMem->tPixelDatas.iHeight*ptVideoMem->tPixelDatas.iLineBytes);
    // 不同设备显存操作不同，直接通过定义设备的显示操作来处理
    if(!ptVideoMem->iDevFrameBuffer){
        // 如果是设备显存，则不需要刷新操作
        ptDefaultDisoOpr->ShowPage(ptVideoMem);
    }    
    return ; 
}

int GetPixelDatasFrmBMP(char *strFileName, PT_PixelDatas ptPixelDatas)
{
	T_FileMap tFileMap;
	int iError;
	int iXres, iYres, iBpp;

	/* 图标存在 /etc/digitpic/icons */
	snprintf(tFileMap.strFileName, 128, "%s/%s", ICON_PATH, strFileName);
	tFileMap.strFileName[127] = '\0';
	
	iError = MapFile(&tFileMap);
	if (iError)
	{
		DBG_PRINTF("MapFile %s error!\n", strFileName);
		return -1;
	}

	iError = g_tBMPParser.isSupport(tFileMap.pucFileMapMem);
	if (iError == 0)
	{
		DBG_PRINTF("%s is not bmp file\n", strFileName);
		return -1;
	}

	GetDispResolution(&iXres, &iYres, &iBpp);
	ptPixelDatas->iBpp = iBpp;
	iError = g_tBMPParser.GetPixelDatas(tFileMap.pucFileMapMem, ptPixelDatas);
	if (iError)
	{
		DBG_PRINTF("GetPixelDatas for %s error!\n", strFileName);
		return -1;
	}

	return 0;
}

void FreePixeDatasForIcon(PT_PixelDatas ptPixelDatas){
    g_tBMPParser.FreePixelDatas(ptPixelDatas);
}

static void InvertButton(PT_Layout ptLayout){
    int iY;
    int i;
    int iButtonWithBytes;
    unsigned char *pucVideoMem;
    PT_DispOpr ptDispOpr = GetDefaultDispDev();

    // 显存位置
    pucVideoMem = ptDispOpr->pucDispMem;
    // 图标在显存中的起始位置
    pucVideoMem += ptLayout->iTopLeftY * ptDispOpr->iLineWidth + ptLayout->iTopLeftX * ptDispOpr->iBpp / 8;

    // 每一个像素由多个字节构成，计算总的字节数
    iButtonWithBytes = (ptLayout->iBotRightX - ptLayout->iTopLeftX) * ptDispOpr->iBpp / 8;

    for(iY = ptLayout->iTopLeftY; iY < ptLayout->iBotRightY; iY++){
        for(i= 0; i < iButtonWithBytes; i++){
            pucVideoMem[i] = ~pucVideoMem[i];  //取反
        }
        pucVideoMem += ptDispOpr->iLineWidth;
    }
}
void ReleaseButton(PT_Layout ptLayout){
    //直接在显存中修改
    InvertButton(ptLayout);
}
void PressButton(PT_Layout ptLayout){
    InvertButton(ptLayout);
}