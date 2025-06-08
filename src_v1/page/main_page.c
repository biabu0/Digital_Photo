#include<stdio.h>
#include<stdlib.h>
#include<config.h>
#include<page_manager.h>
#include<disp_manager.h>
#include<render.h>
#include<input_manager.h>

static int MainPageGetInputEvent(PT_Layout aptLayout, PT_InputEvent ptInputEvent);

// a:array数组
static T_Layout g_atMainPageLayout[] = {
    {0,0,0,0,"browse_mode.bmp"},
    {0,0,0,0,"continue_mod.bmp"},
    {0,0,0,0,"setting.bmp"},
    {0,0,0,0,NULL},// 结尾
};


static void ShowMainPage(PT_Layout atLayout){
    
    PT_VideoMem ptVideoMem;
    T_PixelDatas tOriginIconPixelDatas;
    T_PixelDatas tIconPixelDatas;
    int iIconHeight;
    int iIconWidth;
    int iIconX;
    int iIconY;
    int iXres, iYres, iBpp;
    int iError;

    // 1. 获得内存块
    ptVideoMem = GetVideoMem(ID("main"),  1);
    if(ptVideoMem == NULL){
        DBG_PRINTF("<3>GetVideoMem error!\n");
        return ;
    }
    
    if(ptVideoMem == NULL){
        DBG_PRINTF("<3>Can't get video mem for main page!\n");
        return ;
    }
    
    // 2. 描画数据，如果内存中的数据已经存在，则直接刷新到显存即可
    
    if(ptVideoMem->ePicDataState != PDS_GENERATED){
        GetDispResolution(&iXres, &iYres, &iBpp);
        iIconHeight = iYres * 2 / 10;
        iIconWidth = iIconHeight * 2;
        iIconX = (iXres - iIconWidth) / 2;
        iIconY = iYres / 10;

        tIconPixelDatas.iBpp = iBpp;
        tIconPixelDatas.iHeight = iIconHeight;
        tIconPixelDatas.iWidth = iIconWidth;
        tIconPixelDatas.iLineBytes = iIconWidth * iBpp / 8;
        tIconPixelDatas.iTotalBytes = iIconHeight * tIconPixelDatas.iLineBytes;
        tIconPixelDatas.aucPixelDatas = malloc(tIconPixelDatas.iTotalBytes);
        if(tIconPixelDatas.aucPixelDatas == NULL){
            DBG_PRINTF("<3>tIconPixelDatas.aucPixelDatas malloc error!\n");
            return ;
        }
        while(atLayout->strIconName){
            atLayout->iTopLeftX = iIconX;
            atLayout->iTopLeftY = iIconY;
            atLayout->iBotRightX = iIconX + iIconWidth - 1;
            atLayout->iBotRightY = iIconY + iIconHeight - 1;
            
            // 从BMP文件中获得原始像素数据
            iError = GetPixelDatasFrmBMP(atLayout->strIconName, &tOriginIconPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>GetPixelDatasFrmBMP error!\n");
                return ;
            }
            // 将像素数据缩放一下
           
            iError = PicZoom(&tOriginIconPixelDatas, &tIconPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>PicZoom error!\n");
                return ;
            }
            // 将像素数据合并到内存块中
            iError = PicMerge(iIconX, iIconY, &tIconPixelDatas, &ptVideoMem->tPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>PicMerge error!\n");
                return ;
            }
            // 更新内存状态
            FreePixeDatasForIcon(&tOriginIconPixelDatas);
            atLayout++;
            iIconY += iYres * 3 / 10;

        }
        free(tIconPixelDatas.aucPixelDatas);
        // 数据描述完成
        ptVideoMem->ePicDataState = PDS_GENERATED;

    }

    // 3. 刷新到数据上

    FlushVideoMemToDev(ptVideoMem);

    // 4. 释放内存块
    PutVideoMem(ptVideoMem);

}

static void MainPageRun(void){

    int iIndex;
    T_InputEvent tInputEvent;
    // b:bool布尔类型
    int bPressed = 0;
    int iIndexPressed = -1;

    // 1.显示main_page界面
    ShowMainPage(g_atMainPageLayout);

    // 2.创建Prepare线程：用户可能会停顿部分时间，在这段时间将可能得下一个页面准备好，便于快速切换，流畅

    // 调用GetInputEvent获取输入事件处理
    
    while(1){

        iIndex = MainPageGetInputEvent(g_atMainPageLayout, &tInputEvent);

        /*可能存在在按钮部分按下，然后移动到其他位置松开的情况
         */
        // 松开状态
        if(tInputEvent.iPressure == 0){
            // 松开状态之前有按钮按下
            if(bPressed){
                DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                ReleaseButton(&g_atMainPageLayout[iIndexPressed]);
                bPressed = 0;
                iIndexPressed = -1;
            }
        }else{
            if(iIndex != -1){
                DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                // 未曾按下按钮
                if(!bPressed){
                    bPressed = 1;
                    iIndexPressed = iIndex;
                    PressButton(&g_atMainPageLayout[iIndexPressed]);

                }    
            }
        }
    }
}

static int MainPageGetInputEvent(PT_Layout aptLayout, PT_InputEvent ptInputEvent){

    T_InputEvent tInputEvent;
    int iRet;
    int i = 0;

    // 获取原始触摸屏数据:调用input_manager.c中函数，使得当前线程处于休眠状态，有触摸屏数据到来的时候唤醒该线程
    // 将获取到的触摸屏数据存放到tInputEvent中，包含触摸屏的触摸点位置已经压力状态等
    iRet = GetInputEvent(&tInputEvent);
    if(iRet != 0){
        DBG_PRINTF("<3>GetInputEvent error!\n");
        return -1;
    }

    // 将得到的输入事件返回，无论该事件是否是点击了按钮都要进行返回，如果不是点击了按钮，可能是对图片大小进行缩放等操作
    *ptInputEvent = tInputEvent;

    // 不是触摸屏的数据先不进行处理
    if(tInputEvent.iType != INPUT_TYPE_TOUCHSCREEN){
        //DBG_PRINTF("<3>tInputEvent.iType is not INPUT_TYPE_TOUCHSCREEN!\n");
        return -1;
    }

    // 处理数据
    // 1.确定触点位于哪一个按钮上
    while(aptLayout[i].strIconName){
        if((tInputEvent.iX >= aptLayout[i].iTopLeftX) && (tInputEvent.iX <= aptLayout[i].iBotRightX) \
            && (tInputEvent.iY >= aptLayout[i].iTopLeftY) && (tInputEvent.iY <= aptLayout[i].iBotRightY)){
            // 找到被点中的按钮，返回按钮的下标
            return i;
        }else{
            i++;
        }
    }

    // 点击位置不位于按钮内部
    return -1;
}


static T_PageAction g_tMainPageAction = {
    .name          = "main",
    .GetInputEvent = MainPageGetInputEvent,
    .Run           = MainPageRun,
    //.Prepare       = MainPagePrepare,
};

int MainPageInit(void){
    return RegisterPageAction(&g_tMainPageAction);
}
