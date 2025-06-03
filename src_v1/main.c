#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "include/draw.h"
#include "include/encoding_manager.h"
#include "include/fonts_manager.h"
#include "include/config.h"
#include "include/input_manager.h"
#include "include/disp_manager.h"
#include "include/pic_operation.h"
#include "include/render.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>



#include "include/debug_manager.h"


/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */
#define ArraySize 128

// ./digitalpic <bmp_file>
int main(int argc, char ** argv){

	int iFdBmp;
	int iRet;
	unsigned char *pucBMPMem;
	struct stat tBMPStat;
	PT_DispOpr ptDispOpr;
	T_PixelDatas tPixelDatas;
	T_PixelDatas tPixelDatasSmall;
	T_PixelDatas tPixelDatasFB;
	extern T_PicFileParser g_tBMPParser;

	if(argc != 2){
		printf("%s <bmp_file>\n", argv[0]);
		return -1;
	}
	
	// 初始化调试
	DebugInit();
	InitDebugChannel();

	DisplayInit();

	ptDispOpr = GetDispOpr("fb");
	ptDispOpr->DeviceInit();
	ptDispOpr->CleanScreen(0);

	/* 打开BMP文件 */
	iFdBmp = open(argv[1], O_RDWR);
	if(iFdBmp == -1){
		DBG_PRINTF("can't open %s\n",  argv[1]);
	}

	fstat(iFdBmp, &tBMPStat);

	pucBMPMem = (unsigned char *)mmap(NULL , tBMPStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFdBmp, 0);
	if(pucBMPMem == (void *)-1){
		DBG_PRINTF("mmap error\n");
	}

	/* 提取BMP文件的RGB数据，缩放，在LCD上显示出来*/
	iRet = g_tBMPParser.isSupport(pucBMPMem);
	if(iRet == 0){
		DBG_PRINTF("%s is not support\n", argv[1]);
		return -1;
	}

	tPixelDatas.iBpp = ptDispOpr->iBpp;
	iRet = g_tBMPParser.GetPixelDatas(pucBMPMem, &tPixelDatas);
	if(iRet){
		DBG_PRINTF("g_tBMPParser->GetPixelDatas error!\n");
		return -1;
	}

	// 将framebuffer作为一张大图片
	tPixelDatasFB.iWidth = ptDispOpr->iXres;
	tPixelDatasFB.iHeight = ptDispOpr->iYres;
	tPixelDatasFB.iBpp = ptDispOpr->iBpp;
	tPixelDatasFB.iLineBytes = ptDispOpr->iXres *  ptDispOpr->iBpp / 8;
	tPixelDatasFB.aucPixelDatas = ptDispOpr->pucDispMem;

	PicMerge(0, 0, &tPixelDatas,  &tPixelDatasFB);

	tPixelDatasSmall.iWidth = tPixelDatas.iWidth / 4;
	tPixelDatasSmall.iHeight = tPixelDatas.iHeight / 4;
	tPixelDatasSmall.iBpp = tPixelDatas.iBpp;
	tPixelDatasSmall.iLineBytes = tPixelDatasSmall.iWidth * tPixelDatasSmall.iBpp / 8;
	tPixelDatasSmall.aucPixelDatas = malloc(tPixelDatasSmall.iLineBytes * tPixelDatasSmall.iHeight);

	PicZoom(&tPixelDatas, &tPixelDatasSmall);

	PicMerge(128, 128, &tPixelDatasSmall,  &tPixelDatasFB);

	free(tPixelDatasSmall.aucPixelDatas);

	return 0;
}