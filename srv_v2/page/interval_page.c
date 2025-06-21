#include <config.h>
#include <render.h>
#include <stdlib.h>
#include <fonts_manager.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


static int g_iIntervalSecond = 1;



static T_Layout g_tIntervalNumberLayout;

// 定义菜单区域的图标布局
static T_Layout g_atIntervalPageIconsLayout[] = {
	{0, 0, 0, 0, "inc.bmp"},
	{0, 0, 0, 0, "time.bmp"},
	{0, 0, 0, 0, "dec.bmp"},
	{0, 0, 0, 0, "ok.bmp"},
	{0, 0, 0, 0, "cancel.bmp"},
	{0, 0, 0, 0, NULL},
};

// 菜单区域页面布局
static T_PageLayout g_tIntervalPageLayout = {
    .iMaxTotalBytes = 0,
    .atLayout = g_atIntervalPageIconsLayout
};

void GetIntervalTime(int *piIntervalSecond)
{
    *piIntervalSecond = g_iIntervalSecond;
}

static int IntervalGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent){
    return GenericPageGetInputEvent(ptPageLayout, ptInputEvent);
}



static void CalIntervalPageLayout(PT_PageLayout ptPageLayout){
    int iStartY;
	int iWidth;
	int iHeight;
	int iXres, iYres, iBpp;
	int iTmpTotalBytes;
	PT_Layout atLayout;

	atLayout = ptPageLayout->atLayout;
	GetDispResolution(&iXres, &iYres, &iBpp);
	ptPageLayout->iBpp = iBpp;

	/*   
	 *    ----------------------
	 *                          1/2 * iHeight
	 *          inc.bmp         iHeight * 28 / 128     
	 *         time.bmp         iHeight * 72 / 128
	 *          dec.bmp         iHeight * 28 / 128     
	 *                          1/2 * iHeight
	 *    ok.bmp     cancel.bmp 1/2 * iHeight
	 *                          1/2 * iHeight
	 *    ----------------------
	 */
	iHeight = iYres / 3;
	iWidth  = iHeight;
	iStartY = iHeight / 2;

	/* inc图标 */
	atLayout[0].iTopLeftY  = iStartY;
	atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight * 28 / 128 - 1;
	atLayout[0].iTopLeftX  = (iXres - iWidth * 52 / 128) / 2;
	atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth * 52 / 128 - 1;

	iTmpTotalBytes = (atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1) * (atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* time图标 */
	atLayout[1].iTopLeftY  = atLayout[0].iBotRightY + 1;
	atLayout[1].iBotRightY = atLayout[1].iTopLeftY + iHeight * 72 / 128 - 1;
	atLayout[1].iTopLeftX  = (iXres - iWidth) / 2;
	atLayout[1].iBotRightX = atLayout[1].iTopLeftX + iWidth - 1;
	iTmpTotalBytes = (atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1) * (atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* dec图标 */
	atLayout[2].iTopLeftY  = atLayout[1].iBotRightY + 1;
	atLayout[2].iBotRightY = atLayout[2].iTopLeftY + iHeight * 28 / 128 - 1;
	atLayout[2].iTopLeftX  = (iXres - iWidth * 52 / 128) / 2;
	atLayout[2].iBotRightX = atLayout[2].iTopLeftX + iWidth * 52 / 128 - 1;
	iTmpTotalBytes = (atLayout[2].iBotRightX - atLayout[2].iTopLeftX + 1) * (atLayout[2].iBotRightY - atLayout[2].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* ok图标 */
	atLayout[3].iTopLeftY  = atLayout[2].iBotRightY + iHeight / 2 + 1;
	atLayout[3].iBotRightY = atLayout[3].iTopLeftY + iHeight / 2 - 1;
	atLayout[3].iTopLeftX  = (iXres - iWidth) / 3;
	atLayout[3].iBotRightX = atLayout[3].iTopLeftX + iWidth / 2 - 1;
	iTmpTotalBytes = (atLayout[3].iBotRightX - atLayout[3].iTopLeftX + 1) * (atLayout[3].iBotRightY - atLayout[3].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* ok图标 */
	atLayout[4].iTopLeftY  = atLayout[3].iTopLeftY;
	atLayout[4].iBotRightY = atLayout[3].iBotRightY;
	atLayout[4].iTopLeftX  = atLayout[3].iTopLeftX * 2 + iWidth/2;
	atLayout[4].iBotRightX = atLayout[4].iTopLeftX + iWidth/2 - 1;
	iTmpTotalBytes = (atLayout[4].iBotRightX - atLayout[4].iTopLeftX + 1) * (atLayout[4].iBotRightY - atLayout[4].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* 用来显示数字的区域比较特殊, 单独处理
	 * time.bmp原图大小为128x72, 里面的两个数字大小为52x40
	 * 经过CalcIntervalPageLayout后有所缩放
	 */
	iWidth  = atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1;
	iHeight = atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1;

	g_tIntervalNumberLayout.iTopLeftX  = atLayout[1].iTopLeftX + (128 - 52) / 2 * iWidth / 128;
	g_tIntervalNumberLayout.iBotRightX = atLayout[1].iBotRightX - (128 - 52) / 2 * iWidth / 128 + 1;

	g_tIntervalNumberLayout.iTopLeftY  = atLayout[1].iTopLeftY + (72 - 40) / 2 * iHeight / 72;
	g_tIntervalNumberLayout.iBotRightY = atLayout[1].iBotRightY - (72 - 40) / 2 * iHeight / 72 + 1;
}


static int GenerateIntervalPageSpecialIcon(int dwNumber, PT_VideoMem ptVideoMem){
    int iError;
    char strNumber[3];
    unsigned int dwFontSize;

    dwFontSize = g_tIntervalNumberLayout.iBotRightY - g_tIntervalNumberLayout.iTopLeftY;
    SetFontSize(dwFontSize);

    if(dwNumber > 59){
        return -1;
    }

    snprintf(strNumber, 3, "%02d", dwNumber);

    iError = MergerStringToCenterOfRectangleInVideoMem(g_tIntervalNumberLayout.iTopLeftX, g_tIntervalNumberLayout.iTopLeftY, g_tIntervalNumberLayout.iBotRightX, g_tIntervalNumberLayout.iBotRightY, (unsigned char *)strNumber, ptVideoMem);

    return iError;
}

static void ShowIntervalPage(PT_PageLayout ptPageLayout){
    int iError;
    PT_Layout ptLayout;
    ptLayout = ptPageLayout->atLayout;

    PT_VideoMem ptVideoMem;
    // 0. 获取显存
    ptVideoMem = GetVideoMem(ID("interval"), 1);
    if(ptVideoMem == NULL){
    	DBG_PRINTF("Can't get video mem for Interval page!\n");
		return;
    }
   
    // 1. 绘制页面布局
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    if(ptLayout[0].iTopLeftX == 0){
        CalIntervalPageLayout(ptPageLayout);
    }
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    // 2. 绘制页面
    GeneratePage(ptPageLayout, ptVideoMem);
    iError = GenerateIntervalPageSpecialIcon(g_iIntervalSecond, ptVideoMem);
    if (iError)
	{
		DBG_PRINTF("GenerateIntervalPageSpecialIcon error!\n");
	}
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    // 3. 刷新到显存
    FlushVideoMemToDev(ptVideoMem);
    // 4. 释放内存块
    PutVideoMem(ptVideoMem);
}

static void IntervalPageRun(PT_PageParams ptParentPageParams){

    int iIndex;
    int bPressedButton = 0;
    int iPressedButtonIndex = -1;
    int iIntervalSecond = g_iIntervalSecond;

    T_InputEvent tInputEvent;
    PT_VideoMem ptDevVideoMem;
    ptDevVideoMem = GetDevVideoMem();

    // 1. 显示
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    ShowIntervalPage(&g_tIntervalPageLayout);

    // 2. 获取输入事件
    while(1){
        iIndex = IntervalGetInputEvent(&g_tIntervalPageLayout, &tInputEvent);
        if(tInputEvent.iPressure == 0){
            if(bPressedButton){
                ReleaseButton(&g_atIntervalPageIconsLayout[iPressedButtonIndex]);
                bPressedButton = 0;
                switch(iPressedButtonIndex){
                    case 0:     //增加
                    {
                        iIntervalSecond++;
                        if (iIntervalSecond == 60){
                            iIntervalSecond = 0;
                        }
                        GenerateIntervalPageSpecialIcon(iIntervalSecond, ptDevVideoMem);
                        break;
                    }
                    case 2:
                    {
                        iIntervalSecond--;
                        if (iIntervalSecond == -1)
                        {
                            iIntervalSecond = 59;
                        }
                        GenerateIntervalPageSpecialIcon(iIntervalSecond, ptDevVideoMem);
                        break;
                    }
                    case 3:
                    {
                        g_iIntervalSecond = iIntervalSecond;
                        return ;
                        break;
                    }
                    case 4:
                    {
                        return ;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                iPressedButtonIndex = -1;
            }
        }else{
            if(iIndex != -1){
                if(!bPressedButton){
                    bPressedButton = 1;
                    iPressedButtonIndex = iIndex;
                    PressButton(&g_atIntervalPageIconsLayout[iPressedButtonIndex]);
                }
            }
        }
    }
}

static T_PageAction g_tIntervalPageAction = {
	.name          = "interval",
    //.GetInputEvent = IntervalGetInputEvent,
	.Run           = IntervalPageRun,
};



int IntervalPageInit(void){
	return RegisterPageAction(&g_tIntervalPageAction);
}







