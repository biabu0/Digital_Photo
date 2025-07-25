
/******************************************************
 * @brief  实现music界面设计
 * 
 * @author  bia布
 * @date    2025/07/21
 * @version 1.0
 ******************************************************/

#include <page_manager.h>
#include <pic_operation.h>
#include <debug_manager.h>
#include <disp_manager.h>
#include <file.h>
#include <fonts_manager.h>
#include <music_manager.h>
#include <render.h>
#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define HALT_ICON_NAME "halt.bmp"
#define PLAY_ICON_NAME "play.bmp"

// 定义菜单区域的图标布局
static T_Layout g_atMusicPageIconsLayout[] = {
	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, "play.bmp"},
	{0, 0, 0, 0, "decvol.bmp"},
	{0, 0, 0, 0, "addvol.bmp"},
	{0, 0, 0, 0, "pre_music.bmp"},
    {0, 0, 0, 0, "next_music.bmp"},
	{0, 0, 0, 0, NULL},
};

// 菜单区域页面布局
static T_PageLayout g_tMusicPageMenuLayout = {
    .iMaxTotalBytes = 0,
    .atLayout = g_atMusicPageIconsLayout,
};

static T_Layout g_tMusicDisLayout;


static void MusicPageRun(T_PageParams *ptParentPageIdentify);
static int MusicGetInputEvent(struct PageLayout *ptPageLayout, struct InputEvent *ptInputEvent);

static T_PageAction g_tMusicPageAction = {
	.name          = "music",
    .GetInputEvent = MusicGetInputEvent,
	.Run           = MusicPageRun,
};

static void CalcMusicPageMenusLayout(struct PageLayout *ptPageLayout)
{
	int iXres;
	int iYres;
	int iBpp;

	int iHeight;
	int iWidth;
	int iVerticalDis;
	int iProportion;    /* 图像与各个图像间距的比例 */
	int iTmpTotalBytes;
	int iIconNum;
	int iIconTotal;     /* 图标总的个数 */
	
	struct Layout *atLayout;

	/* 获得图标数组 */
	atLayout = ptPageLayout->atLayout;
    GetDispResolution(&iXres, &iYres, &iBpp);
	ptPageLayout->iBpp = iBpp;

	iIconTotal = sizeof(g_atMusicPageIconsLayout) / sizeof(struct Layout) - 1;
	iProportion = 2000;	/* 图像与间隔的比例 */

	/* 计算高，图像间距，宽 */
	iHeight = iYres * iProportion / 
		(iProportion * iIconTotal + iIconTotal + 1);
	iVerticalDis = iHeight / iProportion;
	iWidth  = iXres / 8;	/* 宽为 LCD 长的 1/4 */
	iIconNum = 0;
    DBG_PRINTF(APP_INFO"%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 循环作图，结束标志是名字为 NULL */
	while(atLayout->strIconName != NULL){
		atLayout->iTopLeftX  = 5;
		atLayout->iTopLeftY  = iVerticalDis * (iIconNum + 1)
			+ iHeight * iIconNum;
		atLayout->iBotRightX = atLayout->iTopLeftX + iWidth - 1;
		atLayout->iBotRightY = atLayout->iTopLeftY + iHeight - 1;
		
		iTmpTotalBytes = (atLayout->iBotRightX -  atLayout->iTopLeftX + 1) * iBpp / 8
			* (atLayout->iBotRightY - atLayout->iTopLeftY + 1);

		/* 这个是为了生成图像的时候为每个图标分配空间用 */
		if(ptPageLayout->iMaxTotalBytes < iTmpTotalBytes){
			ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
		}

		iIconNum ++;
		atLayout ++;
	}
}
/* 文件的图标 layout 要在里面进行分配 */
static int CalcMusicPageFilesLayout()
{
	int iXres, iYres, iBpp;
	int iTopLeftX, iTopLeftY;
	int iBotRightX, iBotRightY;

	GetDispResolution(&iXres, &iYres, &iBpp);

	/* 紧跟着菜单图标的右边排列,一直到 LCD 的最右边 */
	/* _____________________________
	 *|       _____________________ |
	 *| menu |                     ||
	 *|      |                     ||
	 *| menu |                     ||
	 *|      |         音乐        ||
	 *| menu |                     ||
	 *|      |                     ||
	 *| menu |_____________________||
	 *|_____________________________|
	 */
	iTopLeftX  = g_atMusicPageIconsLayout[0].iBotRightX + 1;
	iBotRightX = iXres - 1;
	iTopLeftY  = 0;
	iBotRightY = iYres - 1;

	g_tMusicDisLayout.iTopLeftX	= iTopLeftX;
	g_tMusicDisLayout.iBotRightX  = iBotRightX;
	g_tMusicDisLayout.iTopLeftY	= iTopLeftY;
	g_tMusicDisLayout.iBotRightY  = iBotRightY;
	g_tMusicDisLayout.strIconName  = NULL;

	return 0;
}


// 将mp3文件名称显示在指定区域中，如果名称超过区域限制，则使用...表示截断


  /**
 * @brief  将mp3文件名称显示在指定区域中，如果名称超过区域限制，则使用...表示截断
 * 
 * @param  
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/24
 * @version 2.0
 */

static void GenerateMusicTextPage(char *strName, struct Layout *ptDisLayout, struct VideoMem *ptVideoMem)
{
	struct Layout tCleanDisLayout;
	char strTmp[256];
	char *pcTmp;

	/* 找到最后一个反斜杠 */
    // 使用strrchr函数从字符串strName中查找最后一个'/'字符的位置
    //返回指向该位置的指针pcTmp
	pcTmp = strrchr(strName, '/');
	strcpy(strTmp, pcTmp + 1);

	DBG_PRINTF(APP_INFO"----------------strTmp--------------: %s\n", strTmp);

	/* 清空要设置的区域 */
	tCleanDisLayout.iTopLeftX  = ptDisLayout->iTopLeftX;
	tCleanDisLayout.iTopLeftY  = ptDisLayout->iTopLeftY;
	tCleanDisLayout.iBotRightX = ptDisLayout->iBotRightX;
	tCleanDisLayout.iBotRightY = ptDisLayout->iBotRightY * 2 / 3;
	tCleanDisLayout.strIconName = NULL;

	//SetFontSize(30);

	/* 显示名字 */
	MergerStringToCenterOfRectangleInVideoMem(ptDisLayout->iTopLeftX, ptDisLayout->iTopLeftY,
					ptDisLayout->iBotRightX, ptDisLayout->iBotRightY * 2 / 3,
					(unsigned char *)strTmp, ptVideoMem);
}


// 音乐播放器进度条背景的生成
static void GenerateMusicProgressBarBG(T_Layout *ptDisLayout, T_VideoMem *ptVideoMem){
    T_Layout tCleanDisLayout;

    /* 清空要设置的区域 */
	tCleanDisLayout.iTopLeftX  = ptDisLayout->iTopLeftX;
	tCleanDisLayout.iTopLeftY  = ptDisLayout->iBotRightY * 2 / 3;
	tCleanDisLayout.iBotRightX = ptDisLayout->iBotRightX - 100;
	tCleanDisLayout.iBotRightY = ptDisLayout->iBotRightY;
	tCleanDisLayout.strIconName = NULL;
    ClearVideoMemRegion(ptVideoMem, &tCleanDisLayout, CONFIG_PROGRESS_BG_COLOR);
}


// 音乐播放器运行时进度条的颜色的生成，通过歌曲运行比例来调节
/* iRatio 歌曲运行的时间比例，0 - 100 */
static void GenerateMusicProgressBar(int iRatio, struct Layout *ptDisLayout, struct VideoMem *ptVideoMem)
{
	struct Layout tCleanDisLayout;
	
	/* 清空要设置的区域 */
	tCleanDisLayout.iTopLeftX  = ptDisLayout->iTopLeftX;
	tCleanDisLayout.iTopLeftY  = ptDisLayout->iBotRightY * 2 / 3;
	tCleanDisLayout.iBotRightX = ptDisLayout->iBotRightX - 100;
	tCleanDisLayout.iBotRightY = ptDisLayout->iBotRightY;
	tCleanDisLayout.strIconName = NULL;

	tCleanDisLayout.iBotRightX = tCleanDisLayout.iTopLeftX + iRatio * (tCleanDisLayout.iBotRightX - tCleanDisLayout.iTopLeftX + 1) / 1000;
	ClearVideoMemRegion(ptVideoMem, &tCleanDisLayout, CONFIG_PROGRESS_COLOR);
}


static void GenerateMusicVolText(int iVol, T_Layout *ptDisLayout, T_VideoMem *ptVideoMem){
    T_Layout tCleanDisLayout;
    char strTmp[3];
    /* 清空要设置的区域 */
	tCleanDisLayout.iTopLeftX  = ptDisLayout->iBotRightX - 100;
	tCleanDisLayout.iTopLeftY  = ptDisLayout->iBotRightY * 2 / 3;
	tCleanDisLayout.iBotRightX = ptDisLayout->iBotRightX;
	tCleanDisLayout.iBotRightY = ptDisLayout->iBotRightY;
	tCleanDisLayout.strIconName = NULL;
    //SetFontSize(30);
    sprintf(strTmp, "%d", iVol);
    // 显示音量
	MergerStringToCenterOfRectangleInVideoMem(ptDisLayout->iBotRightX - 100, ptDisLayout->iBotRightY * 2 / 3,
					ptDisLayout->iBotRightX, ptDisLayout->iBotRightY,
					(unsigned char *)strTmp, ptVideoMem);
}

// 显示Music界面布局
static void ShowMusicPage(struct PageLayout *ptPageLayout, char *strPath)
{
	struct Layout *atDisLayout;
	struct VideoMem *ptVideoMem;

	atDisLayout = ptPageLayout->atLayout;
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	ptVideoMem = GetVideoMem(ID("music"), 1);
	if(NULL == ptVideoMem){
        DBG_PRINTF(APP_ERR"malloc music VideoMem error\n");
		return;
	}
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	if(atDisLayout[0].iTopLeftX == 0){
		DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		CalcMusicPageMenusLayout(ptPageLayout);
		DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		CalcMusicPageFilesLayout();
	}
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	GeneratePage(ptPageLayout, ptVideoMem);
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	GenerateMusicTextPage(strPath, &g_tMusicDisLayout, ptVideoMem);
	GeneratePage(ptPageLayout, ptVideoMem);
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	GenerateMusicProgressBarBG(&g_tMusicDisLayout, ptVideoMem);
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	GenerateMusicVolText(0, &g_tMusicDisLayout, ptVideoMem);
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	FlushVideoMemToDev(ptVideoMem);

    PutVideoMem(ptVideoMem);
}

// 设置播放按钮：暂停和播放是两个图标
static int SetPlayHaltIcon(T_Layout *ptIconDisLayout, T_VideoMem *ptVideoMem, char *pcIconName){
    int iError = 0;

	T_PixelDatas tOriginIconDatas;
	T_PixelDatas tIcondatas;

    tIcondatas.iBpp  = g_tMusicPageMenuLayout.iBpp;
    tIcondatas.aucPixelDatas = malloc(g_tMusicPageMenuLayout.iMaxTotalBytes);
    if(NULL == tIcondatas.aucPixelDatas){
        DBG_PRINTF(APP_ERR "malloc error\n");
        return -1;
    }
    iError = GetPixelDatasFrmBMP(pcIconName, &tOriginIconDatas);
    if(iError){
        DBG_PRINTF(APP_ERR"GetPiexlDatasForIcons error\n");
        free(tIcondatas.aucPixelDatas);
        return -1;
    }

    tIcondatas.iHeight = ptIconDisLayout->iBotRightY - ptIconDisLayout->iTopLeftY;
	tIcondatas.iWidth  = ptIconDisLayout->iBotRightX - ptIconDisLayout->iTopLeftX;
	tIcondatas.iLineBytes	= tIcondatas.iWidth * tIcondatas.iBpp / 8;
	tIcondatas.iTotalBytes = tIcondatas.iLineBytes * tIcondatas.iHeight;

    PicZoom(&tOriginIconDatas, &tIcondatas);
    PicMerge(ptIconDisLayout->iTopLeftX, ptIconDisLayout->iTopLeftY, &tIcondatas, &ptVideoMem->tPixelDatas);

    free(tIcondatas.aucPixelDatas);

    return 0;
}

// 线程：显示进度条
void *ProgressBarThread(void* data){
	struct MusicParser *ptMusicParser = (struct MusicParser *)data;
	int iRunTimeRatio = 0;
	struct VideoMem *ptVideoMem = GetDevVideoMem();

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	while(1){
		usleep(100*1000);		//每100ms 刷新一次进度条;
		iRunTimeRatio = CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_GET_RUNTIME);
		GenerateMusicProgressBar(iRunTimeRatio, &g_tMusicDisLayout, ptVideoMem);
		pthread_testcancel();		// 线程取消
	}

	pthread_exit(NULL);
}
static void MusicPageRun(T_PageParams *ptParentPageIdentify){

	int iIndex = 0;
	int iIndexPressed = -1;	/* 判断是否是在同一个图标上按下与松开 */
	int bPressedFlag = 0;
	char strFullPathName[256];

	int iError = 0;
	char strCurDirPath[256];
	char strCurFileName[256];
	char *pcTmp;


	int iDirContentsNumber;
	int iMusicFileIndex;
	int iCurMusicIndex = 0;
	unsigned char bHaltMusic = 0;

	struct VideoMem *ptVideoMem;
	int iXres, iYres, iBpp;
	GetDispResolution(&iXres, &iYres, &iBpp);
	ptVideoMem = GetDevVideoMem();

	pthread_t tProgressBarThreadId;

	T_PageParams tPageIndentify;
	T_InputEvent tInputEvent;

	PT_DirContent *ptDirContents;
	T_FileMap tMusicFileDesc;
	// mp3解析-------------------------------
	struct MusicParser *ptMusicParser;

	tPageIndentify.iPageID = ID("music");
	snprintf(strFullPathName, 256, "%s", ptParentPageIdentify->strCurPicFile);
	strFullPathName[255] = '\0';
	DBG_PRINTF(APP_INFO"strFullPathName: %s\n", strFullPathName);
	// 显示界面
	ShowMusicPage(&g_tMusicPageMenuLayout, strFullPathName);
	// 解析目录路径和文件名称
	strcpy(strCurDirPath, ptParentPageIdentify->strCurPicFile);
	pcTmp = strrchr(strCurDirPath, '/');
	*pcTmp = '\0';
	strcpy(strCurFileName, pcTmp + 1);

	// 获取目录内容
	iError = GetDirContents(strCurDirPath, &ptDirContents, &iDirContentsNumber);

	// 确定当前文件所在位置
	for(iMusicFileIndex = 0; iMusicFileIndex < iDirContentsNumber; iMusicFileIndex++){
		//DBG_PRINTF(APP_INFO"ptDirContents[%d].strName: %s\n", iMusicFileIndex, ptDirContents[iMusicFileIndex]->strName);
		if(0 == strcmp(strCurFileName, ptDirContents[iMusicFileIndex]->strName)){
			iCurMusicIndex = iMusicFileIndex;
			break;
		}
	}
	// 初始化音乐播放
	ptMusicParser = PlayMusic(&tMusicFileDesc, strFullPathName);
	if(NULL == ptMusicParser){
		DBG_PRINTF("PlayMusic %s failed\n", strFullPathName);
		StopMusic(&tMusicFileDesc, ptMusicParser);
	}
	//DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	iError = CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_GET_VOL);
	GenerateMusicVolText(iError, &g_tMusicDisLayout, ptVideoMem);

	// 创建线程用于显示音乐播放进度
	DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	pthread_create(&tProgressBarThreadId, NULL, (void *)ProgressBarThread, (void*)ptMusicParser);
	while(1){
		iIndex = MusicGetInputEvent(&g_tMusicPageMenuLayout, &tInputEvent);
		if(tInputEvent.iPressure == 0){
			if(0 == bPressedFlag){
				goto nextwhilecircle;
			}
			bPressedFlag = 0;
			ReleaseButton(&g_atMusicPageIconsLayout[iIndexPressed]);
			switch(iIndexPressed){
				case 0:{		// 返回上层
					//停止播放
					StopMusic(&tMusicFileDesc, ptMusicParser);
					free(ptDirContents);
					ptDirContents = NULL;
					pthread_cancel(tProgressBarThreadId);		// 取消进度条线程
					pthread_join(tProgressBarThreadId, NULL);
					return ;	// 返回上层
					break;
				}
				case 1:{			// 播放与暂停
					if(!bHaltMusic){
						CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_HALT);
						SetPlayHaltIcon(&g_atMusicPageIconsLayout[iIndexPressed], ptVideoMem, HALT_ICON_NAME);
						bHaltMusic = 1;
					}else{
						CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_PLAY);
						SetPlayHaltIcon(&g_atMusicPageIconsLayout[iIndexPressed], ptVideoMem, PLAY_ICON_NAME);
						bHaltMusic = 0;
					}

					break;
				}
				case 2:{		// 音量增加
					CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_ADD_VOL);
					iError = CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_GET_VOL);
					GenerateMusicVolText(iError, &g_tMusicDisLayout, ptVideoMem);
					break;
				}
				case 3:{		// 音量减少
					CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_DEC_VOL);
					iError = CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_GET_VOL);
					GenerateMusicVolText(iError, &g_tMusicDisLayout, ptVideoMem);
					break;
				}
				case 4:{		// 播放上一曲内容
					iMusicFileIndex = iCurMusicIndex;
					
					while(iMusicFileIndex > 0){
						iMusicFileIndex--;
						if(ptDirContents[iMusicFileIndex]->eFileType == FILETYPE_FILE){
							snprintf(strFullPathName, 256, "%s/%s", strCurDirPath, ptDirContents[iMusicFileIndex]->strName);
							strFullPathName[255] = '\0';							
							if(isMusicSupport(strFullPathName)){

								ShowMusicPage(&g_tMusicPageMenuLayout, strFullPathName);
								iCurMusicIndex = iMusicFileIndex;
								// 取消当前播放音乐的进度条线程
								pthread_cancel(tProgressBarThreadId);
								pthread_join(tProgressBarThreadId, NULL);

								bHaltMusic = 0;
								StopMusic(&tMusicFileDesc, ptMusicParser);
								ptMusicParser = PlayMusic(&tMusicFileDesc, strFullPathName);
								if(NULL == ptMusicParser){
									DBG_PRINTF(APP_ERR"Play %s error\n", strFullPathName);
									StopMusic(&tMusicFileDesc, ptMusicParser);
									break;
								}
								iError = CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_GET_VOL);
								GenerateMusicVolText(iError, &g_tMusicDisLayout, ptVideoMem);

								pthread_create(&tProgressBarThreadId, NULL, ProgressBarThread, ptMusicParser);
								break;

							}
						}
					}

					break;
				}
				case 5:{
					iMusicFileIndex = iCurMusicIndex;
					while(iMusicFileIndex < iDirContentsNumber - 1){
						iMusicFileIndex++;
						if(ptDirContents[iMusicFileIndex]->eFileType == FILETYPE_FILE){
							snprintf(strFullPathName, 256, "%s/%s", strCurDirPath, ptDirContents[iMusicFileIndex]->strName);
							strFullPathName[255] = '\0';

							if(isMusicSupport(strFullPathName)){
								ShowMusicPage(&g_tMusicPageMenuLayout, strFullPathName);
								iCurMusicIndex = iMusicFileIndex;

								pthread_cancel(tProgressBarThreadId);
								pthread_join(tProgressBarThreadId, NULL);

								bHaltMusic = 0;
								StopMusic(&tMusicFileDesc, ptMusicParser);
								ptMusicParser = PlayMusic(&tMusicFileDesc, strFullPathName);
								if(NULL == ptMusicParser){
									DBG_PRINTF(APP_ERR "PlayMusic %s failed!\n", strFullPathName);
									StopMusic(&tMusicFileDesc, ptMusicParser);
									break;
								}
								iError = CtrlMusic(ptMusicParser, MUSIC_CTRL_CODE_GET_VOL);
								GenerateMusicVolText(iError, &g_tMusicDisLayout, ptVideoMem);

								// 重新创建进度条线程
								pthread_create(&tProgressBarThreadId, NULL, (void*)ProgressBarThread, ptMusicParser);
								break;
							}
						}	
					}
					break;
				}
				default:{
					break;
				}
			}

		}else{
			if(iIndex == -1){
				goto nextwhilecircle;
			}
			if(0 == bPressedFlag){
				bPressedFlag = 1;
				iIndexPressed = iIndex;
				PressButton(&g_atMusicPageIconsLayout[iIndexPressed]);
				goto nextwhilecircle;
			}
		}
nextwhilecircle:
		iError = 0;
	}

}
static int MusicGetInputEvent(struct PageLayout *ptPageLayout, struct InputEvent *ptInputEvent){
	return GenericPageGetInputEvent(ptPageLayout, ptInputEvent);
}

int MusicPageInit(void)
{
	return RegisterPageAction(&g_tMusicPageAction);
}


