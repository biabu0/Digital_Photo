#include <config.h>
#include <render.h>
#include <stdlib.h>
#include <string.h>
#include <page_manager.h>



// 定义菜单区域的图标布局
static T_Layout g_atSettingPageIconsLayout[] = {
	{0, 0, 0, 0, "select_fold.bmp"},
	{0, 0, 0, 0, "interval.bmp"},
	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, NULL},
};

// 菜单区域页面布局
static T_PageLayout g_tSettingPageLayout = {
    .iMaxTotalBytes = 0,
    .atLayout = g_atSettingPageIconsLayout
};


static int SettingGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent){
    return GenericPageGetInputEvent(ptPageLayout, ptInputEvent);
}



static void CalSettingPageLayout(PT_PageLayout ptPageLayout){
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
	 *                           1/2 * iHeight
	 *          select_fold.bmp  iHeight
	 *                           1/2 * iHeight
	 *          interval.bmp     iHeight
	 *                           1/2 * iHeight
	 *          return.bmp       iHeight
	 *                           1/2 * iHeight
	 *    ----------------------
	 */
	 
	iHeight = iYres * 2 / 10;
	iWidth  = iHeight;
	iStartY = iHeight / 2;
	
	/* select_fold图标 */
	atLayout[0].iTopLeftY  = iStartY;
	atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight - 1;
	atLayout[0].iTopLeftX  = (iXres - iWidth * 2) / 2;
	atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1) * (atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}


	/* interval图标 */
	atLayout[1].iTopLeftY  = atLayout[0].iBotRightY + iHeight / 2 + 1;
	atLayout[1].iBotRightY = atLayout[1].iTopLeftY + iHeight - 1;
	atLayout[1].iTopLeftX  = (iXres - iWidth * 2) / 2;
	atLayout[1].iBotRightX = atLayout[1].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1) * (atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* return图标 */
	atLayout[2].iTopLeftY  = atLayout[1].iBotRightY + iHeight / 2 + 1;
	atLayout[2].iBotRightY = atLayout[2].iTopLeftY + iHeight - 1;
	atLayout[2].iTopLeftX  = (iXres - iWidth) / 2;
	atLayout[2].iBotRightX = atLayout[2].iTopLeftX + iWidth - 1;

	iTmpTotalBytes = (atLayout[2].iBotRightX - atLayout[2].iTopLeftX + 1) * (atLayout[2].iBotRightY - atLayout[2].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}
}

static void ShowSettingPage(PT_PageLayout ptPageLayout){
    PT_Layout ptLayout;
    ptLayout = ptPageLayout->atLayout;

    PT_VideoMem ptVideoMem;
    // 0. 获取显存
    ptVideoMem = GetVideoMem(ID("setting"), 1);
    if(ptVideoMem == NULL){
    	DBG_PRINTF("Can't get video mem for setting page!\n");
		return;
    }
   
    // 1. 绘制页面布局
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    if(ptLayout[0].iTopLeftX == 0){
        CalSettingPageLayout(ptPageLayout);
    }
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    // 2. 绘制页面
    GeneratePage(ptPageLayout, ptVideoMem);
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    // 3. 刷新到显存
    FlushVideoMemToDev(ptVideoMem);
    // 4. 释放内存块
    PutVideoMem(ptVideoMem);
}

static void SettingPageRun(PT_PageParams ptParentPageParams){

    int iIndex;
    int bPressedButton = 0;
    int iPressedButtonIndex = -1;
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);


    T_PageParams tPageParams;
    tPageParams.iPageID = ID("setting");
    T_InputEvent tInputEvent;


    // 1. 显示
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    ShowSettingPage(&g_tSettingPageLayout);

    // 2. 获取输入事件
    while(1){
        iIndex = SettingGetInputEvent(&g_tSettingPageLayout, &tInputEvent);
        if(tInputEvent.iPressure == 0){
            if(bPressedButton){
                ReleaseButton(&g_atSettingPageIconsLayout[iPressedButtonIndex]);
                bPressedButton = 0;
                switch(iPressedButtonIndex){
                    case 0:
                    {
                        DBG_PRINTF("<5> From Setting Page to Browse Page\n");
                        Page("browse")->Run(&tPageParams);
                        ShowSettingPage(&g_tSettingPageLayout);
                        break;
                    }
                    case 1:
                    {
                        DBG_PRINTF("<5> From Setting Page to Interval Page\n");
                        Page("interval")->Run(&tPageParams);
                        ShowSettingPage(&g_tSettingPageLayout);
                        break;
                    }
                    case 2:
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
                    PressButton(&g_atSettingPageIconsLayout[iPressedButtonIndex]);
                }
            }
        }
    }
}

static T_PageAction g_tSettingPageAction = {
	.name          = "setting",
    //.GetInputEvent = SettingGetInputEvent,
	.Run           = SettingPageRun,
};



int SettingPageInit(void){
	return RegisterPageAction(&g_tSettingPageAction);
}



