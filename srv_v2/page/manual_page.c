/**
 *  文件内容界面的实现
 *  时间：2025/06/08 - 
 *  作者：biabu
 * */


#include<config.h>
#include<page_manager.h>
#include<stdio.h>
#include<stdlib.h>
#include<disp_manager.h>
#include<render.h>
#include<input_manager.h>
#include<file.h>
#include<string.h>


// 定义菜单区域的图标布局
static T_Layout g_atMenuIconsLayout[] = {
	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, "zoomout.bmp"},
	{0, 0, 0, 0, "zoomin.bmp"},
	{0, 0, 0, 0, "pre_pic.bmp"},
    {0, 0, 0, 0, "next_pic.bmp"},
    {0, 0, 0, 0, "continue_mod_small.bmp"},
	{0, 0, 0, 0, NULL},
};

// 菜单区域页面布局
static T_PageLayout g_tManualPageMenuIconsLayout = {
    .iMaxTotalBytes = 0,
    .atLayout = g_atMenuIconsLayout
};

// 图片的布局
static T_Layout g_tManualPictureLayout;
// 图片数据
static T_PixelDatas g_tOriginPicPixelDatas;
static T_PixelDatas g_tZoomedPicPixelDatas;

// 显示在LCD上的图片, 它的中心点, 在g_tZoomedPicPixelDatas里的坐标
static int g_iXofZoomedPicShowInCenter;  
static int g_iYofZoomedPicShowInCenter;


/**
 * @brief  计算页面中菜单栏图标坐标值
 * 
 * @param  ptPageLayout - 内含计算后的各图标的左上角/右下角座标值
 * @return void
 * 
 * @date    2025/06/10
 * @version 1.0
 */
static void CalcManualPageMenusLayout(PT_PageLayout ptPageLayout){
    

    int iXres, iYres, iBpp;
    int iHeight, iWidth;
    int i= 0;
    int iTmpTotalBytes = 0;
    PT_Layout atLayout = ptPageLayout->atLayout;
    
    GetDispResolution(&iXres, &iYres, &iBpp);
    ptPageLayout->iBpp = iBpp;


    if(iXres > iYres){
        iHeight = iYres / 6;
        iWidth = iHeight;
        atLayout[0].iTopLeftX = 0;
        atLayout[0].iTopLeftY = 0;
        atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth - 1;
        atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight - 1;
        for(i = 1; i < 6; i++){
            atLayout[i].iTopLeftY  = atLayout[i-1].iBotRightY+ 1;
    		atLayout[i].iBotRightY = atLayout[i].iTopLeftY + iHeight - 1;
    		atLayout[i].iTopLeftX  = 0;
    		atLayout[i].iBotRightX = atLayout[i].iTopLeftX + iWidth - 1;
        }

    }else{
        iWidth  = iXres / 6;
		iHeight = iWidth;

		/* return图标 */
		atLayout[0].iTopLeftY  = 0;
		atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight - 1;
		atLayout[0].iTopLeftX  = 0;
		atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth - 1;

        /* 其他5个图标 */
        for (i = 1; i < 6; i++)
        {
    		atLayout[i].iTopLeftY  = 0;
    		atLayout[i].iBotRightY = atLayout[i].iTopLeftY + iHeight - 1;
    		atLayout[i].iTopLeftX  = atLayout[i-1].iBotRightX + 1;
    		atLayout[i].iBotRightX = atLayout[i].iTopLeftX + iWidth - 1;
        }

    }
    i = 0;
    while(atLayout[i].strIconName){
        iTmpTotalBytes = (atLayout[i].iBotRightX - atLayout[i].iTopLeftX + 1) * (atLayout[i].iBotRightY - atLayout[i].iTopLeftY + 1) * iBpp / 8;
        if(ptPageLayout->iMaxTotalBytes <= iTmpTotalBytes){
            ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
        }
        i++;
    }
}




/**
 * @brief  计算页面中存放图片的位置
 * 
 * @return void
 * 
 * @date    2025/06/10
 * @version 1.0
 */
static void CalcManualPagePictureLayout(void){

    int iXres, iYres, iBpp;
    
    GetDispResolution(&iXres, &iYres, &iBpp);
    if(iXres > iYres){
        g_tManualPictureLayout.iTopLeftX = g_atMenuIconsLayout[0].iBotRightX + 1;
        g_tManualPictureLayout.iTopLeftY = 0;
        g_tManualPictureLayout.iBotRightX = iXres - 1;
        g_tManualPictureLayout.iBotRightY = iYres - 1;
    }else{
        g_tManualPictureLayout.iTopLeftX  = 0;
		g_tManualPictureLayout.iBotRightX = iXres - 1;
		g_tManualPictureLayout.iTopLeftY  = g_atMenuIconsLayout[0].iBotRightY + 1;
		g_tManualPictureLayout.iBotRightY = iYres - 1;
    }
    g_tManualPictureLayout.strIconName = NULL;
}



/**
 * @brief   获得图片文件的原始象素数据
 * 
 * @param  strFileName - 文件名(含绝对路径)
 * @return NULL   - 失败
 *         非NULL - 一个PT_PixelDatas结构指针,内含图像象素数据
 * 
 * @author  bia布
 * @date    2025/06/10
 * @version 1.0
 */
static PT_PixelDatas GetOriginPictureFilePixelDatas(char *strFileName){
    int iError;
    if(g_tOriginPicPixelDatas.aucPixelDatas){
        free(g_tOriginPicPixelDatas.aucPixelDatas);
        g_tOriginPicPixelDatas.aucPixelDatas = NULL;
    }
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    iError = GetPixelDatasFrmFile(strFileName, &g_tOriginPicPixelDatas);
    if (iError){
        return NULL;
    }else{
        return &g_tOriginPicPixelDatas;
    }
}



/**
 * @brief 获得缩放后的图片象素数据
 * 
 * @param  ptOriginPicPixelDatas  -原始图片数据
 * @param  iZoomedWidth           -缩放后的图片宽度
 * @param  iZoomedHeight          -缩放后的图片高度
 * @return PT_PixelDatas     NULL   - 失败
 *            非NULL - 一个PT_PixelDatas结构指针,内含图像数据     
 * 
 * @author  bia布
 * @date    2025/06/10
 * @version 1.0
 */
static PT_PixelDatas GetZoomedPicPixelDatas(PT_PixelDatas ptOriginPicPixelDatas, int iZoomedWidth, int iZoomedHeight){
	int k;
    int iXres, iYres, iBpp;
    
	GetDispResolution(&iXres, &iYres, &iBpp);
    if (g_tZoomedPicPixelDatas.aucPixelDatas)
    {
        free(g_tZoomedPicPixelDatas.aucPixelDatas);
        g_tZoomedPicPixelDatas.aucPixelDatas = NULL;
    }
    // 记录尺度
    k = (float)ptOriginPicPixelDatas->iHeight / ptOriginPicPixelDatas->iWidth;
    g_tZoomedPicPixelDatas.iWidth  = iZoomedWidth;
    g_tZoomedPicPixelDatas.iHeight = iZoomedWidth * k;
    if (g_tZoomedPicPixelDatas.iHeight > iZoomedHeight)
    {
        g_tZoomedPicPixelDatas.iWidth  = iZoomedHeight / k;
        g_tZoomedPicPixelDatas.iHeight = iZoomedHeight;
    }
    g_tZoomedPicPixelDatas.iBpp        = iBpp;
    g_tZoomedPicPixelDatas.iLineBytes  = g_tZoomedPicPixelDatas.iWidth * g_tZoomedPicPixelDatas.iBpp / 8;
    g_tZoomedPicPixelDatas.iTotalBytes = g_tZoomedPicPixelDatas.iLineBytes * g_tZoomedPicPixelDatas.iHeight;
    g_tZoomedPicPixelDatas.aucPixelDatas = malloc(g_tZoomedPicPixelDatas.iTotalBytes);
    if (g_tZoomedPicPixelDatas.aucPixelDatas == NULL)
    {
        return NULL;
    }
    
    PicZoom(ptOriginPicPixelDatas, &g_tZoomedPicPixelDatas);
    return &g_tZoomedPicPixelDatas;
}



/**
 * @brief  在"manual页面"中显示图片
 * 
 * @param  ptVideoMem   - 存储图片数据的内存块
 * @param  strFileName - 要显示的文件的名字(含绝对路径)
 * @return int       - 成功
 *            其他值 - 失败
 * 
 * @author  bia布
 * @date    2025/06/10
 * @version 1.0
 */
static int ShowPictureInManualPage(PT_VideoMem ptVideoMem, char *strFileName){
    int iPicLayoutWidth;
    int iPicLayoutHeight;
    int iTopLeftX, iTopLeftY;
    PT_PixelDatas ptOriginPicPixelDatas;
    PT_PixelDatas ptZoomedPicPixelDatas;
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    // 1. 打开文件，获取文件信息
    ptOriginPicPixelDatas = GetOriginPictureFilePixelDatas(strFileName);
    if (!ptOriginPicPixelDatas)
    {
        DBG_PRINTF("<3>GetOriginPictureFilePixelDatas error!\n");
        return -1;
    }
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    DBG_PRINTF("<7>g_tOriginPicPixelDatas.iTotalBytes:%d\n", g_tOriginPicPixelDatas.iTotalBytes);
    // 2. 缩放到指定大小
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    iPicLayoutWidth = g_tManualPictureLayout.iBotRightX - g_tManualPictureLayout.iTopLeftX + 1;
    iPicLayoutHeight = g_tManualPictureLayout.iBotRightY - g_tManualPictureLayout.iTopLeftY + 1;
    ptZoomedPicPixelDatas = GetZoomedPicPixelDatas(&g_tOriginPicPixelDatas, iPicLayoutWidth, iPicLayoutHeight);
    if (!ptZoomedPicPixelDatas)
    {
        return -1;
    }
    // 3. 刷新到内存块
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    iTopLeftX = g_tManualPictureLayout.iTopLeftX + (iPicLayoutWidth - ptZoomedPicPixelDatas->iWidth) / 2;
    iTopLeftY = g_tManualPictureLayout.iTopLeftY + (iPicLayoutHeight - ptZoomedPicPixelDatas->iHeight) / 2;
    g_iXofZoomedPicShowInCenter = ptZoomedPicPixelDatas->iWidth / 2;
    g_iYofZoomedPicShowInCenter = ptZoomedPicPixelDatas->iHeight / 2;

    // 显示之前先清空数据区域
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    ClearVideoMemRegion(ptVideoMem, &g_tManualPictureLayout, COLOR_BACKGROUND);
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    DBG_PRINTF("iTotalBytes: %d\n", ptZoomedPicPixelDatas->iTotalBytes);
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    PicMerge(iTopLeftX, iTopLeftY, ptZoomedPicPixelDatas, &ptVideoMem->tPixelDatas);
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;
}

/**
 * @brief  显示"manual页面": 除了显示菜单图标外,还会显示图片
 * 
 * @param  ptPageLayout - 内含多个图标的文件名和显示区域
 * @param  strFileName  - 要显示的图片
 * @return void
 * 
 * @author  bia布
 * @date    2025/06/10
 * @version 1.0
 */
static void ShowManualPage(PT_PageLayout ptPageLayout, char *strFileName){
    int iError;
    (void)iError;

    PT_VideoMem  ptVideoMem;
    // 获得区域图标
    PT_Layout aptLayout = ptPageLayout->atLayout;
    
    // 1. 获取内存块
    ptVideoMem = GetVideoMem(ID("manual"), 1);
    if(ptVideoMem == NULL){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>GetVideoMem error!\n");
        return ;
    }
    // 2. 描画数据
    // iTopLeftX == 0确定当前是否对数据进行描画
    if(aptLayout[0].iTopLeftX == 0){
        // 计算菜单数据
        CalcManualPageMenusLayout(ptPageLayout);
        // 计算目录和文件布局
        CalcManualPagePictureLayout();
    }
    /* 在videomem上生成图标 */
    iError = GeneratePage(ptPageLayout, ptVideoMem);
    iError = ShowPictureInManualPage(ptVideoMem, strFileName);
    if (iError)
    {
        PutVideoMem(ptVideoMem);
        return;
    }
    // 3. 刷新到显存上
    FlushVideoMemToDev(ptVideoMem);
    // 4. 释放内存块
    PutVideoMem(ptVideoMem);
}


static void ManualPageRun(PT_PageParams ptParentPageParams){
    
    char strFullPathName[256];
    T_PageParams tPageParams;
    PT_VideoMem ptDevVideoMem;

    tPageParams.iPageID = ID("manual");
    
    // 获取显存，直接改变显存内容
    ptDevVideoMem = GetDevVideoMem();
    DBG_PRINTF("<3>GetDevVideoMem\n");
    strcpy(strFullPathName, ptParentPageParams->strCurPicFile);

    // 1.显示界面：显示菜单和文件内容界面
    DBG_PRINTF("<3>ShowManualPage\n");
    ShowManualPage(&g_tManualPageMenuIconsLayout, strFullPathName);

    // 2.创建Prepare线程

    // 调用GetInputEvent获取输入事件处理
    while(1){
    }
}
static T_PageAction g_tManualPageAction = {
    .name          = "manual",
    //.GetInputEvent = ManualPageGetInputEvent,
    .Run           = ManualPageRun,
    //.Prepare       = ManualPagePrepare,
};
int ExplorePageInit(void){
    return RegisterPageAction(&g_tManualPageAction);
}
