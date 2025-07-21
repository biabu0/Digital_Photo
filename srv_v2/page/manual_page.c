/**
 *  文件内容界面的实现
 *  时间：2025/06/08 
 *  作者：biabu
 *  -------------------------------------
 *  预读线程提前加载图片数据，提高图片切换效率
 *  时间：2025/07/21
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
#include<pthread.h>
// 放大/缩小系数 在定义宏的时候要注意()尤其是宏是一个表达式的时候
#define ZOOM_RATIO (0.9)

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
static PT_PixelDatas GetOriginPictureFilePixelDatas(char *strFileName, PT_PixelDatas ptPixelDatas){
    int iError;
    if(g_tOriginPicPixelDatas.aucPixelDatas){
        free(g_tOriginPicPixelDatas.aucPixelDatas);
        g_tOriginPicPixelDatas.aucPixelDatas = NULL;
    }
    if(ptPixelDatas){
        iError = 0;
        memcpy(&g_tOriginPicPixelDatas, ptPixelDatas, sizeof(T_PixelDatas));
        free(ptPixelDatas);
    }else{
        iError = GetPixelDatasFrmFile(strFileName, &g_tOriginPicPixelDatas);
    }
    //DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
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
	float k;
    int iXres, iYres, iBpp;
    
	GetDispResolution(&iXres, &iYres, &iBpp);
    if (g_tZoomedPicPixelDatas.aucPixelDatas)
    {
        free(g_tZoomedPicPixelDatas.aucPixelDatas);
        g_tZoomedPicPixelDatas.aucPixelDatas = NULL;
    }
    // DBG_PRINTF("ptOriginPicPixelDatas->iHeight: %d\n", ptOriginPicPixelDatas->iHeight);
    // DBG_PRINTF("ptOriginPicPixelDatas->iWidth: %d\n",ptOriginPicPixelDatas->iWidth);
    // 记录尺度
    k = (float)ptOriginPicPixelDatas->iHeight / ptOriginPicPixelDatas->iWidth;
    // DBG_PRINTF("k = %f\n", k);
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
    // DBG_PRINTF("zoomed pic pixel data size = %d\n", g_tZoomedPicPixelDatas.iTotalBytes);
    // DBG_PRINTF("zoomed pic pixel data line bytes = %d\n", g_tZoomedPicPixelDatas.iLineBytes);
    // DBG_PRINTF("g_tZoomedPicPixelDatas.iHeight = %d\n", g_tZoomedPicPixelDatas.iHeight);
    // DBG_PRINTF("g_tZoomedPicPixelDatas.iWidth = %d\n", g_tZoomedPicPixelDatas.iWidth);



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
static int ShowPictureInManualPage(PT_VideoMem ptVideoMem, char *strFileName, PT_PixelDatas pic_data){
    int iPicLayoutWidth;
    int iPicLayoutHeight;
    int iTopLeftX, iTopLeftY;
    PT_PixelDatas ptOriginPicPixelDatas;
    PT_PixelDatas ptZoomedPicPixelDatas;
    // 1. 打开文件，获取文件信息
    ptOriginPicPixelDatas = GetOriginPictureFilePixelDatas(strFileName, pic_data);
    // GetPixelDatasFrmFile(strFileName, &g_tOriginPicPixelDatas);
    // ptOriginPicPixelDatas = &g_tOriginPicPixelDatas;


    if (!ptOriginPicPixelDatas)
    {
        DBG_PRINTF("<3>GetOriginPictureFilePixelDatas error!\n");
        return -1;
    }
    //DBG_PRINTF("<6>g_tOriginPicPixelDatas.iTotalBytes:%d\n", g_tOriginPicPixelDatas.iTotalBytes);
    // 2. 缩放到指定大小
    iPicLayoutWidth = g_tManualPictureLayout.iBotRightX - g_tManualPictureLayout.iTopLeftX + 1;
    iPicLayoutHeight = g_tManualPictureLayout.iBotRightY - g_tManualPictureLayout.iTopLeftY + 1;
    ptZoomedPicPixelDatas = GetZoomedPicPixelDatas(&g_tOriginPicPixelDatas, iPicLayoutWidth, iPicLayoutHeight);
    //DBG_PRINTF("iTotalBytes: %d\n", ptZoomedPicPixelDatas->iTotalBytes);
    if (!ptZoomedPicPixelDatas)
    {
        return -1;
    }
    // 3. 刷新到内存块
    iTopLeftX = g_tManualPictureLayout.iTopLeftX + (iPicLayoutWidth - ptZoomedPicPixelDatas->iWidth) / 2;
    iTopLeftY = g_tManualPictureLayout.iTopLeftY + (iPicLayoutHeight - ptZoomedPicPixelDatas->iHeight) / 2;
    g_iXofZoomedPicShowInCenter = ptZoomedPicPixelDatas->iWidth / 2;
    g_iYofZoomedPicShowInCenter = ptZoomedPicPixelDatas->iHeight / 2;

    // 显示之前先清空数据区域
    ClearVideoMemRegion(ptVideoMem, &g_tManualPictureLayout, COLOR_BACKGROUND);
    //DBG_PRINTF("iTotalBytes: %d\n", ptZoomedPicPixelDatas->iTotalBytes);
    PicMerge(iTopLeftX, iTopLeftY, ptZoomedPicPixelDatas, &ptVideoMem->tPixelDatas);
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
static void ShowManualPage(PT_PageLayout ptPageLayout, char *strFileName, PT_PixelDatas pic_data){
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
    iError = ShowPictureInManualPage(ptVideoMem, strFileName, pic_data);
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

/**
 * @brief  为"manual页面"获得输入数据,判断输入事件位于哪一个图标上
 * 
 * @param  ptPageLayout - 内含多个图标的显示区域
 * @param  ptInputEvent - 内含得到的输入数据
 * @return int  -1     - 输入数据不位于任何一个图标之上
 *            其他值 - 输入数据所落在的图标(PageLayout->atLayout数组的哪一项)
 * @author  bia布
 * @date    2025/06/15
 * @version 1.0
 */
static int ManualPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent){
    return GenericPageGetInputEvent(ptPageLayout, ptInputEvent);
}




/**
 * @brief  在"manual页面"中显示经过缩放的图片
 * 
 * @param  ptZoomedPicPixelDatas - 内含已经缩放的图片的象素数据
 * @param  ptVideoMem            - 在这个VideoMem中显示
 * @return void
 * 
 * @author  bia布
 * @date    2025/06/15
 * @version 1.0
 */
static void ShowZoomedPictureInLayout(PT_PixelDatas ptZoomedPicPixelDatas, PT_VideoMem ptVideoMem){
    int iStartXofNewPic, iStartYofNewPic;
    int iStartXofOldPic, iStartYofOldPic;
    int iWidthPictureInPlay, iHeightPictureInPlay;
    int iPictureLayoutWidth, iPictureLayoutHeight;
    int iDeltaX, iDeltaY;

    iPictureLayoutWidth  = g_tManualPictureLayout.iBotRightX - g_tManualPictureLayout.iTopLeftX + 1;
    iPictureLayoutHeight = g_tManualPictureLayout.iBotRightY - g_tManualPictureLayout.iTopLeftY + 1;
    
    /* 显示新数据 */
    iStartXofNewPic = g_iXofZoomedPicShowInCenter - iPictureLayoutWidth/2;
    if (iStartXofNewPic < 0)
    {
        iStartXofNewPic = 0;
    }
    if (iStartXofNewPic > ptZoomedPicPixelDatas->iWidth)
    {
        iStartXofNewPic = ptZoomedPicPixelDatas->iWidth;
    }

    /* 
     * g_iXofZoomedPicShowInCenter - iStartXofNewPic = PictureLayout中心点X坐标 - iStartXofOldPic
     */
    iDeltaX = g_iXofZoomedPicShowInCenter - iStartXofNewPic;
    iStartXofOldPic = (g_tManualPictureLayout.iTopLeftX + iPictureLayoutWidth / 2) - iDeltaX;
    if (iStartXofOldPic < g_tManualPictureLayout.iTopLeftX)
    {
        iStartXofOldPic = g_tManualPictureLayout.iTopLeftX;
    }
    if (iStartXofOldPic > g_tManualPictureLayout.iBotRightX)
    {
        iStartXofOldPic = g_tManualPictureLayout.iBotRightX + 1;
    }
        
    if ((ptZoomedPicPixelDatas->iWidth - iStartXofNewPic) > (g_tManualPictureLayout.iBotRightX - iStartXofOldPic + 1))
        iWidthPictureInPlay = (g_tManualPictureLayout.iBotRightX - iStartXofOldPic + 1);
    else
        iWidthPictureInPlay = (ptZoomedPicPixelDatas->iWidth - iStartXofNewPic);
    
    iStartYofNewPic = g_iYofZoomedPicShowInCenter - iPictureLayoutHeight/2;
    if (iStartYofNewPic < 0)
    {
        iStartYofNewPic = 0;
    }
    if (iStartYofNewPic > ptZoomedPicPixelDatas->iHeight)
    {
        iStartYofNewPic = ptZoomedPicPixelDatas->iHeight;
    }

    /* 
     * g_iYofZoomedPicShowInCenter - iStartYofNewPic = PictureLayout中心点Y坐标 - iStartYofOldPic
     */
    iDeltaY = g_iYofZoomedPicShowInCenter - iStartYofNewPic;
    iStartYofOldPic = (g_tManualPictureLayout.iTopLeftY + iPictureLayoutHeight / 2) - iDeltaY;

    if (iStartYofOldPic < g_tManualPictureLayout.iTopLeftY)
    {
        iStartYofOldPic = g_tManualPictureLayout.iTopLeftY;
    }
    if (iStartYofOldPic > g_tManualPictureLayout.iBotRightY)
    {
        iStartYofOldPic = g_tManualPictureLayout.iBotRightY + 1;
    }
    
    if ((ptZoomedPicPixelDatas->iHeight - iStartYofNewPic) > (g_tManualPictureLayout.iBotRightY - iStartYofOldPic + 1))
    {
        iHeightPictureInPlay = (g_tManualPictureLayout.iBotRightY - iStartYofOldPic + 1);
    }
    else
    {
        iHeightPictureInPlay = (ptZoomedPicPixelDatas->iHeight - iStartYofNewPic);
    }
        
    ClearVideoMemRegion(ptVideoMem, &g_tManualPictureLayout, COLOR_BACKGROUND);
    PicMergeRegion(iStartXofNewPic, iStartYofNewPic, iStartXofOldPic, iStartYofOldPic, iWidthPictureInPlay, iHeightPictureInPlay, ptZoomedPicPixelDatas, &ptVideoMem->tPixelDatas);

}


// 预读图片的线程，优化图片切换
static void* StartNextPicture(void* filename){
    PT_PixelDatas ptNextPicDatas = malloc(sizeof(T_PixelDatas));
    if(!ptNextPicDatas){
        DBG_PRINTF(APP_ERR "StartNextPicture picture_data malloc failed\n");
        pthread_exit(NULL);
    }
    DBG_PRINTF(APP_INFO"<Start Parse> %s!\n", filename);
    if(GetPixelDatasFrmFile(filename, ptNextPicDatas)){
        DBG_PRINTF(APP_ERR"<fault> can't get picture data from %s!\n", filename);
        free(ptNextPicDatas);
        pthread_exit(NULL);
    }
    DBG_PRINTF(APP_INFO"<End Parse> %s!\n", filename);
    pthread_exit(ptNextPicDatas);
}

static void ManualPageRun(PT_PageParams ptParentPageParams){

    // 下一张图片的线程
    pthread_t tNextPicThread;
    PT_PixelDatas ptNextPicPixelDatas;


    // 当前文件的路径
    char strFullPathName[256];
    // 当前文件所在的目录地址
    char strDirName[256];
    char strFileName[256];
    char *strTmp;

    
    int iDirContentsNumber;
    int iError;
    int iIndex;
    int iPicFileIndex;
    int bButtonPressed = 0;
    int iIndexPressed = -1;
    int iZoomedWidth;
    int iZoomedHeight;
    PT_PixelDatas ptZoomedPicPixelDatas = &g_tZoomedPicPixelDatas;
    T_InputEvent tInputEvent;
    T_PageParams tPageParams;
    PT_VideoMem ptDevVideoMem;
    PT_DirContent *aptDirContents;

    tPageParams.iPageID = ID("manual");
    
    // 获取显存，直接改变显存内容
    ptDevVideoMem = GetDevVideoMem();
    //DBG_PRINTF("<3>GetDevVideoMem\n");
    strcpy(strFullPathName, ptParentPageParams->strCurPicFile);

    // 1.显示界面：显示菜单和文件内容界面
    DBG_PRINTF("<3>ShowManualPage\n");
    ShowManualPage(&g_tManualPageMenuIconsLayout, strFullPathName, NULL);


    // 2. 处理路径信息用于显示其他文件
    strcpy(strDirName, ptParentPageParams->strCurPicFile);
    strTmp = strrchr(strDirName, '/');
    *strTmp = '\0';
    // 当前文件的名称
    strcpy(strFileName, strTmp+1);
    // 获取当前目录下所有文件信息
    iError = GetDirContents(strDirName, &aptDirContents, &iDirContentsNumber);
    // 确定当前显示文件的索引
    for(iPicFileIndex = 0; iPicFileIndex < iDirContentsNumber; iPicFileIndex++){
        if(0 == strcmp(aptDirContents[iPicFileIndex]->strName, strFileName)){
            break;
        }
    }

    // 循环寻找下一张图片路径，为其创建线程
    int start_index_1 = iPicFileIndex; // 记录起始位置
    do {
        // 递增索引（循环）
        iPicFileIndex = (iPicFileIndex + 1) % iDirContentsNumber;
        // 防止无限循环（遍历完所有文件后退出）
        if(iPicFileIndex == start_index_1) {
            DBG_PRINTF(APP_INFO"All files processed\n");
            break;
        }
        // 构造路径
        snprintf(strFullPathName, 256, "%s/%s", 
            strDirName, aptDirContents[iPicFileIndex]->strName);
        strFullPathName[255] = '\0';
    
        // 检查文件类型
        if(isPictureFileSupported(strFullPathName)){
            pthread_create(&tNextPicThread, NULL, 
                        (void*)StartNextPicture, strFullPathName);
            break;
        }
    } while(1);

    // 3. 调用GetInputEvent获取输入事件并处理
    while(1){
        iIndex = ManualPageGetInputEvent(&g_tManualPageMenuIconsLayout, &tInputEvent);
        if(tInputEvent.iPressure == 0){
            if(bButtonPressed){
                ReleaseButton(&g_atMenuIconsLayout[iIndexPressed]);
                bButtonPressed = 0;
                switch(iIndexPressed){
                    case 0://返回
                    {
                        return;
                        break;
                    }
                    case 1://缩小
                    {
                        iZoomedWidth = (float)g_tZoomedPicPixelDatas.iWidth * ZOOM_RATIO;
                        iZoomedHeight = (float)g_tZoomedPicPixelDatas.iHeight * ZOOM_RATIO;
                        ptZoomedPicPixelDatas = GetZoomedPicPixelDatas(&g_tOriginPicPixelDatas, iZoomedWidth, iZoomedHeight);
                        
                        g_iXofZoomedPicShowInCenter = (float)g_iXofZoomedPicShowInCenter * ZOOM_RATIO;
                        g_iYofZoomedPicShowInCenter = (float)g_iYofZoomedPicShowInCenter * ZOOM_RATIO;

                        ShowZoomedPictureInLayout(ptZoomedPicPixelDatas, ptDevVideoMem);
                        break;
    
                    }
                    case 2: //放大
                    {
                        iZoomedWidth = (float)g_tZoomedPicPixelDatas.iWidth / ZOOM_RATIO;
                        iZoomedHeight = (float)g_tZoomedPicPixelDatas.iHeight / ZOOM_RATIO;
                        ptZoomedPicPixelDatas = GetZoomedPicPixelDatas(&g_tOriginPicPixelDatas, iZoomedWidth, iZoomedHeight);
                        
                        g_iXofZoomedPicShowInCenter = (float)g_iXofZoomedPicShowInCenter / ZOOM_RATIO;
                        g_iYofZoomedPicShowInCenter = (float)g_iYofZoomedPicShowInCenter / ZOOM_RATIO;

                        ShowZoomedPictureInLayout(ptZoomedPicPixelDatas, ptDevVideoMem);
                        break;
                    }
                    case 3: //上一张
                    {
                        while(iPicFileIndex > 0){
                            iPicFileIndex--;
                            snprintf(strFullPathName, 256, "%s/%s", strDirName, aptDirContents[iPicFileIndex]->strName);
                            strFullPathName[255] = '\0';
                            if(isPictureFileSupported(strFullPathName)){
                                ShowPictureInManualPage(ptDevVideoMem, strFullPathName, NULL);
                                break;
                            }
                        }
                        break;
                    }
                    case 4: //下一张
                    {
                        //DBG_PRINTF(APP_INFO"__LINE__ = %d\n",__LINE__);
                        if(pthread_join(tNextPicThread, (void**)&ptNextPicPixelDatas)){
                            DBG_PRINTF(APP_INFO"__LINE__ = %d\n",__LINE__);
                            DBG_PRINTF(APP_ERR"pthread_join error\n");
                            break;
                        }
                        if(ptNextPicPixelDatas){
                            ShowPictureInManualPage(ptDevVideoMem, NULL, ptNextPicPixelDatas);
                        }
                        //DBG_PRINTF(APP_INFO"__LINE__ = %d\n",__LINE__);
                        // 循环寻找下一张图片路径，为其创建线程
                        int start_index = iPicFileIndex; // 记录起始位置
                        do {
                            // 递增索引（循环）
                            iPicFileIndex = (iPicFileIndex + 1) % iDirContentsNumber;
                            // 防止无限循环（遍历完所有文件后退出）
                            if(iPicFileIndex == start_index) {
                                DBG_PRINTF(APP_INFO"All files processed\n");
                                break;
                            }
                            // 构造路径
                            snprintf(strFullPathName, 256, "%s/%s", 
                                strDirName, aptDirContents[iPicFileIndex]->strName);
                            strFullPathName[255] = '\0';
                        
                            // 检查文件类型
                            if(isPictureFileSupported(strFullPathName)){
                                pthread_create(&tNextPicThread, NULL, 
                                            (void*)StartNextPicture, strFullPathName);
                                break;
                            }
                        } while(1);

                        //DBG_PRINTF(APP_INFO"__LINE__ = %d\n",__LINE__);
                        break;
                        
                    }
                    case 5://连播
                    {
                        if(ptParentPageParams->iPageID == ID("browse")){
                            strcpy(tPageParams.strCurPicFile, strFullPathName);
                            Page("auto")->Run(&tPageParams);
                            ShowManualPage(&g_tManualPageMenuIconsLayout, tPageParams.strCurPicFile, NULL);
                        }else{
                            return ;
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }

                }
                iIndexPressed = -1;
            }

        }else{
            // 菜单栏按钮
            if(iIndex != -1){
                if(!bButtonPressed){
                    bButtonPressed = 1;
                    iIndexPressed = iIndex;                    
                    PressButton(&g_atMenuIconsLayout[iIndexPressed]);
                }

            }else{
                // 				/* 如果没有按钮被按下 */
				// if (!bButtonPressed && !bPicSlipping)
				// {
				// 	bPicSlipping = 1;
                //     tPreInputEvent = tInputEvent;
				// }

				// if (bPicSlipping)
				// {
                //     /* 如果触点滑动距离大于规定值, 则挪动图片 */
                //     if (DistanceBetweenTwoPoint(&tInputEvent, &tPreInputEvent) > SLIP_MIN_DISTANCE)
                //     {                            
                //         /* 重新计算中心点 */
                //         g_iXofZoomedPicShowInCenter -= (tInputEvent.iX - tPreInputEvent.iX);
                //         g_iYofZoomedPicShowInCenter -= (tInputEvent.iY - tPreInputEvent.iY);
                        
                //         /* 显示新数据 */
                //         ShowZoomedPictureInLayout(ptZoomedPicPixelDatas, ptDevVideoMem);
                        
                //         /* 记录滑动点 */
                //         tPreInputEvent = tInputEvent;                            
                //     }
				// }

            }
        }
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
