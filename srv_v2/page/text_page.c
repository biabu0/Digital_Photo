#include<config.h>
#include<page_manager.h>
#include<stdio.h>
#include<stdlib.h>
#include<disp_manager.h>
#include<render.h>
#include<input_manager.h>
#include<file.h>
#include<string.h>

static T_Layout g_atTextMenuIconsLayout[] = {
	{0, 0, 0, 0, "pre_page.bmp"},
	{0, 0, 0, 0, "next_page.bmp"},
	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, NULL},
};

static struct PageLayout g_tTextPageLayout = {
    .iMaxTotalBytes = 0,
    .atLayout = g_atTextMenuIconsLayout
};
T_Layout g_tTxtLayout;


static int TextGetInputEvent(struct PageLayout *ptPageLayout, struct InputEvent *ptInputEvent)
{
	return GenericPageGetInputEvent(ptPageLayout, ptInputEvent);
}
static void CalcTextPageLayout(struct PageLayout *ptPageLayout)
{
	int iWidth;
    int iHeight;
    int iX, iY, iBpp;
    int i;
    // 记录图标中字节数最大的
    int iTmpTotalBytes;

    PT_Layout aptLayout = ptPageLayout->atLayout;

    GetDispResolution(&iX, &iY, &iBpp);
    ptPageLayout->iBpp = iBpp;
    DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    // 显示屏的长宽可能由于设备的不同而有所变化
    if(iX < iY){
	 
		iWidth  = iX / 3;
		iHeight = iWidth;
		
		/* return图标 */
		aptLayout[0].iTopLeftY  = 0;
		aptLayout[0].iBotRightY = aptLayout[0].iTopLeftY + iHeight - 1;
		aptLayout[0].iTopLeftX  = 0;
		aptLayout[0].iBotRightX = aptLayout[0].iTopLeftX + iWidth - 1;

		/* up图标 */
		aptLayout[1].iTopLeftY  = 0;
		aptLayout[1].iBotRightY = aptLayout[1].iTopLeftY + iHeight - 1;
		aptLayout[1].iTopLeftX  = aptLayout[0].iBotRightX + 1;
		aptLayout[1].iBotRightX = aptLayout[1].iTopLeftX + iWidth - 1;

		/* select图标 */
		aptLayout[2].iTopLeftY  = 0;
		aptLayout[2].iBotRightY = aptLayout[2].iTopLeftY + iHeight - 1;
		aptLayout[2].iTopLeftX  = aptLayout[1].iBotRightX + 1;
		aptLayout[2].iBotRightX = aptLayout[2].iTopLeftX + iWidth - 1;

    }else{
        		/*	 iYres/4
		 *	  ----------------------------------
		 *	   up		  
		 *
		 *    select
		 *
		 *    pre_page
		 *  
		 *   next_page
		 *
		 *	  ----------------------------------
		 */

        iHeight = iY / 3;
        iWidth = iHeight;

        // return 图标
        aptLayout[0].iTopLeftX = 0;
        aptLayout[0].iBotRightX = aptLayout[0].iTopLeftX + iWidth -1;
        aptLayout[0].iTopLeftY = 0;
        aptLayout[0].iBotRightY = aptLayout[0].iTopLeftY + iHeight -1;

        // up
        aptLayout[1].iTopLeftX = 0;
        aptLayout[1].iBotRightX = aptLayout[1].iTopLeftX + iWidth -1;
        aptLayout[1].iTopLeftY = aptLayout[0].iBotRightY + 1;
        aptLayout[1].iBotRightY = aptLayout[1].iTopLeftY + iHeight -1;
        /* select图标 */
		aptLayout[2].iTopLeftY  = aptLayout[1].iBotRightY + 1;
		aptLayout[2].iBotRightY = aptLayout[2].iTopLeftY + iHeight - 1;
		aptLayout[2].iTopLeftX  = 0;
		aptLayout[2].iBotRightX = aptLayout[2].iTopLeftX + iWidth - 1;	
    }

    i = 0;
    while(aptLayout[i].strIconName){
        iTmpTotalBytes = (aptLayout[i].iBotRightX - aptLayout[i].iTopLeftX) * (aptLayout[i].iBotRightY - aptLayout[i].iTopLeftY) * iBpp / 8;
        if(iTmpTotalBytes > ptPageLayout->iMaxTotalBytes){
            ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
        }
        i++;
    }
	g_tTxtLayout.iTopLeftX = g_atTextMenuIconsLayout[0].iBotRightX + 1;
	g_tTxtLayout.iTopLeftY = 0;

}

static void ShowTextPage(PT_PageLayout ptPageLayout,PT_PageParams ptParentPageParams)
{

	struct VideoMem *ptTextPageVM;

    PT_Layout atDisLayout;
    atDisLayout = ptPageLayout->atLayout;

	/* 获得一块内存以显示 Text 页面 */
	ptTextPageVM = GetVideoMem(ID("text"), 1);
	if(NULL == ptTextPageVM){
		DebugPrint(APP_ERR"Get Text-page video memory error\n");
		return;
	}

	/* 把三个图标画上去 */
	if(atDisLayout[0].iTopLeftX == 0){
		CalcTextPageLayout(ptPageLayout);
	}

	GeneratePage(ptPageLayout, ptTextPageVM);
	ShowTextInReadingPage(ptTextPageVM, ptParentPageParams->strCurPicFile);
	
	FlushVideoMemToDev(ptTextPageVM);

	/* 释放用完的内存，以供别的程序使用 */
    PutVideoMem(ptTextPageVM);
}

static void TextlPageRun(PT_PageParams ptParentPageParams)
{
	int iIndex;
	int iError = 0;
	int iIndexPressed = -1;	/* 判断是否是在同一个图标上按下与松开 */
	int bPressedFlag = 0;
	struct InputEvent tInputEvent;
    T_PageParams tPageParams;
    tPageParams.iPageID = ID("text");
    DebugPrint(APP_DEBUG"text page \n");  
	ShowTextPage(&g_tTextPageLayout, ptParentPageParams);

	while(1){
		/* 该函数会休眠 */
		iIndex = TextGetInputEvent(&g_tTextPageLayout, &tInputEvent);

		//DebugPrint(APP_DEBUG"text page index = %d****************\n", iIndex);
		if(tInputEvent.iPressure == 0){
			/* 说明曾经有按下，这里是松开 */
			if(bPressedFlag){
				bPressedFlag = 0;
				ReleaseButton(&g_atTextMenuIconsLayout[iIndexPressed]);
				DebugPrint(APP_DEBUG"Release button****************\n");

				/* 在同一个按钮按下与松开 */
//				if(iIndexPressed == iIndex){
//					goto nextwhilecircle;
//				}
					switch(iIndexPressed){
                        case 0: {   /*上一页 */
							// GetPageOpr("interval")->RunPage(&tPageIdentify);
							// ShowTextPage(&g_tTextPageLayout);
							break;
						}
						case 1: {/* 下一页 */
							//return; 
                            break;
						}
						case 2: {   /* 选择目录 */
                            Page("browse")->Run(ptParentPageParams);
                            Page("main")->Run("NULL");
							ShowTextPage(&g_tTextPageLayout, ptParentPageParams);
							break;
						}

						default: {
							DebugPrint(APP_INFO"Somthing wrong\n");
							break;
						}
					}
				iIndexPressed = -1;
			}
		}else{
			if(iIndex != -1){
				if(0 == bPressedFlag){
					bPressedFlag = 1;
					iIndexPressed = iIndex;
					PressButton(&g_atTextMenuIconsLayout[iIndexPressed]);
				}			
			}
		}	
	}
}


static T_PageAction g_tManualPageAction = {
	.name = "text",
	.Run = TextlPageRun,
	// .GetInputEvent = TextGetInputEvent,
//	.Prepare  =    /* 后台准备函数，待实现 */
};
int TextPageInit(void)
{
	return RegisterPageAction(&g_tManualPageAction);
}


