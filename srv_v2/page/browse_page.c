/**
 *  浏览界面的实现
 *  时间：2025/06/05
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

// 文件浏览器界面中图标和文件名的尺寸规范
#define DIR_FILE_ICON_WIDTH    40
#define DIR_FILE_ICON_HEIGHT   DIR_FILE_ICON_WIDTH
#define DIR_FILE_NAME_HEIGHT   20
#define DIR_FILE_NAME_WIDTH   (DIR_FILE_ICON_HEIGHT + DIR_FILE_NAME_HEIGHT)
#define DIR_FILE_ALL_WIDTH    DIR_FILE_NAME_WIDTH
#define DIR_FILE_ALL_HEIGHT   DIR_FILE_ALL_WIDTH

// 文件和目录的索引基础值
#define DIRFILE_ICON_INDEX_BASE 1000

static void BrowsePageRun();
static int BrowsePageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent);

// 定义菜单区域的图标布局
static T_Layout g_atMenuIconsLayout[] = {
//	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, "up.bmp"},
	{0, 0, 0, 0, "select.bmp"},
	{0, 0, 0, 0, "pre_page.bmp"},
	{0, 0, 0, 0, "next_page.bmp"},
	{0, 0, 0, 0, NULL},
};

// 菜单区域页面布局
static T_PageLayout g_tBrowsePageMenuIconsLayout = {
    .iMaxTotalBytes = 0,
    .atLayout = g_atMenuIconsLayout
};

// 存放目录和文件数据
static T_Layout *g_atDirAndFileLayout;

// 文件区域页面布局
static T_PageLayout g_tBrowsePageDirAndFileLayout = {
	.iMaxTotalBytes = 0,
	//.atLayout       = g_atDirAndFileLayout,
};



/*  存储目录和文件区域的bmp数据信息
 * g_tDirClosedIconPixelDatas：存储文件夹关闭状态的图标数据
 * g_tDirOpenedIconPixelDatas：存储文件夹展开状态的图标数据
 * g_tFileIconPixelDatas：存储常规文件的默认图标数据
*/
static T_PixelDatas g_tDirClosedIconPixelDatas;
static T_PixelDatas g_tDirOpenedIconPixelDatas;
static T_PixelDatas g_tFileIconPixelDatas;


/* 用来描述某目录里的内容 */
static PT_DirContent *g_aptDirContents;  /* 数组:存有目录下"顶层子目录","文件"的名字 */
static int g_iDirContentsNumber;         /* g_aptDirContents数组有多少项 */
static int g_iStartIndex = 0;            /* 在屏幕上显示的第1个"目录和文件"是g_aptDirContents数组里的哪一项 */



/* 当前显示的目录 */
static char g_strCurDir[FILE_NAME_SIZE] = DEFAULT_DIR;


/*
 *目录和文件区域的图标文件名
 **/
static char *g_strDirClosedIconName  = "fold_closed.bmp";
static char *g_strDirOpenedIconName  = "fold_opened.bmp";
static char *g_strFileIconName = "file.bmp";

// 文件区域每行和每列的图标数
static int g_iDirFileNumPerCol, g_iDirFileNumPerRow;

/**
 * @brief  计算菜单区域各图标在页面中的位置
 * 
 * @param  ptPageLayout 菜单区域的页面局部结构体
 * 
 * @note   区域中页面局部的图标已经确定，只需要为ptPageLayout中各个图标的位置
 */
static void CalcBrowsePageMenusLayout(PT_PageLayout ptPageLayout){

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
        /*	 iXres/4
		 *	  ----------------------------------
		 *	   up	select	pre_page  next_page
		 *
		 *
		 *
		 *
		 *
		 *
		 *	  ----------------------------------
		 */
		 
		iWidth  = iX / 4;
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

		/* pre_page图标 */
		aptLayout[3].iTopLeftY  = 0;
		aptLayout[3].iBotRightY = aptLayout[3].iTopLeftY + iHeight - 1;
		aptLayout[3].iTopLeftX  = aptLayout[2].iBotRightX + 1;
		aptLayout[3].iBotRightX = aptLayout[3].iTopLeftX + iWidth - 1;

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

        iHeight = iY / 4;
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
		
		/* pre_page图标 */
		aptLayout[3].iTopLeftY  = aptLayout[2].iBotRightY + 1;
		aptLayout[3].iBotRightY = aptLayout[3].iTopLeftY + iHeight - 1;
		aptLayout[3].iTopLeftX  = 0;
		aptLayout[3].iBotRightX = aptLayout[3].iTopLeftX + iWidth - 1;

    }

    i = 0;
    while(aptLayout[i].strIconName){
        iTmpTotalBytes = (aptLayout[i].iBotRightX - aptLayout[i].iTopLeftX) * (aptLayout[i].iBotRightY - aptLayout[i].iTopLeftY) * iBpp / 8;
        if(iTmpTotalBytes > ptPageLayout->iMaxTotalBytes){
            ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
        }
        i++;
    }
}



/**
 * @brief  计算目录和文件的显示区域布局信息
 * 
 * @note   首先根据文件浏览器界面中图标和文件名的尺寸规范确定该区域能够存放的文件和目录的行数和列数，由此获得总的个数，
 *         为g_atDirAndFileLayout分配2N+1个空间，分别存放文件图标和文件对应的名称，最后一个为空，用于判断结束。
 *         为g_tBrowsePageDirAndFileLayout确定显示区域的位置以及各个图标的位置
 */
static void CalcBrowsePageDirAndFilesLayout(void){
    int iXres, iYres, iBpp;
    int iTopLeftX, iTopLeftY;
    int iBotRightX, iBotRightY;
    int iIconWidth, iIconHeight;
    int iNumPerCol, iNumPerRow;
 	int iTopLeftXBak; 
    int iDeltaX, iDeltaY;
    int i, j, k = 0;
    DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    GetDispResolution(&iXres, &iYres, &iBpp);

    // 起始位置确定
    if(iXres < iYres){
        iTopLeftX  = 0;
		iBotRightX = iXres - 1;
		iTopLeftY  = g_atMenuIconsLayout[0].iBotRightY + 1;
		iBotRightY = iYres - 1;
    }else{
        iTopLeftX = g_atMenuIconsLayout[0].iBotRightY + 1;
        iBotRightX = iXres - 1;
        iTopLeftY = 0;
        iBotRightY = iYres - 1;
    }

    //确定一行显示多少个"目录或文件", 显示多少行
    iIconWidth  = DIR_FILE_NAME_WIDTH;
    iIconHeight = iIconWidth;

    iNumPerRow = (iBotRightX - iTopLeftX + 1) / iIconWidth;     
    while(1){
        // 剩余大小
        iDeltaX  = (iBotRightX - iTopLeftX + 1) - iIconWidth * iNumPerRow;
        // 确保两个图标之间的距离大于10个像素
        if((iDeltaX / (iNumPerRow + 1)) < 10){
            iNumPerRow--;
        }else{
            break;
        }
    }

    iNumPerCol = (iBotRightY - iTopLeftY + 1) / iIconHeight;
    while (1)
    {
        iDeltaY  = (iBotRightY - iTopLeftY + 1) - iIconHeight * iNumPerCol;
        if ((iDeltaY / (iNumPerCol + 1)) < 10)
            iNumPerCol--;
        else
            break;
    }
    
    // 每个图标之间的间隔
    iDeltaX = iDeltaX / (iNumPerRow + 1);
    iDeltaY = iDeltaY / (iNumPerCol + 1);

    
    g_iDirFileNumPerRow = iNumPerRow;
    g_iDirFileNumPerCol = iNumPerCol;

    /* 可以显示 iNumPerRow * iNumPerCol个"目录或文件"
     * 分配"两倍+1"的T_Layout结构体: 一个用来表示图标,另一个用来表示名字
     * 最后一个用来存NULL,借以判断结构体数组的末尾
     */
    g_atDirAndFileLayout = malloc(sizeof(T_Layout) * (2 * iNumPerCol * iNumPerRow + 1));
    if (NULL == g_atDirAndFileLayout)
    {
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>malloc error!\n");
        return ;
    }

    g_tBrowsePageDirAndFileLayout.iTopLeftX      = iTopLeftX;
    g_tBrowsePageDirAndFileLayout.iBotRightX     = iBotRightX;
    g_tBrowsePageDirAndFileLayout.iTopLeftY      = iTopLeftY;
    g_tBrowsePageDirAndFileLayout.iBotRightY     = iBotRightY;
    g_tBrowsePageDirAndFileLayout.iBpp           = iBpp;
    g_tBrowsePageDirAndFileLayout.atLayout       = g_atDirAndFileLayout;
    g_tBrowsePageDirAndFileLayout.iMaxTotalBytes = DIR_FILE_ALL_WIDTH * DIR_FILE_ALL_HEIGHT * iBpp / 8;//统一的大小

    /* 确定图标和名字的位置 
     *
     * 图标是一个正方体, "图标+名字"也是一个正方体
     *   --------
     *   |  图  |
     *   |  标  |
     * ------------
     * |   名字   |
     * ------------
     */

    iTopLeftX += iDeltaX;
    iTopLeftY += iDeltaY;
    iTopLeftXBak = iTopLeftX;

    for(i = 0; i < iNumPerCol; i++){
        for(j = 0; j < iNumPerRow; j++){
            /* 图标 */
            g_atDirAndFileLayout[k].iTopLeftX = iTopLeftX + (DIR_FILE_NAME_WIDTH - DIR_FILE_ICON_WIDTH) / 2;
            g_atDirAndFileLayout[k].iBotRightX = g_atDirAndFileLayout[k].iTopLeftX + DIR_FILE_ICON_WIDTH - 1;
            g_atDirAndFileLayout[k].iTopLeftY = iTopLeftY;
            g_atDirAndFileLayout[k].iBotRightY = iTopLeftY + DIR_FILE_ICON_WIDTH - 1;

            /* 名字 */
            g_atDirAndFileLayout[k+1].iTopLeftX  = iTopLeftX;
            g_atDirAndFileLayout[k+1].iBotRightX = iTopLeftX + DIR_FILE_NAME_WIDTH - 1;
            g_atDirAndFileLayout[k+1].iTopLeftY  = g_atDirAndFileLayout[k].iBotRightY + 1;
            g_atDirAndFileLayout[k+1].iBotRightY = g_atDirAndFileLayout[k+1].iTopLeftY + DIR_FILE_NAME_HEIGHT - 1;

            iTopLeftX += DIR_FILE_ALL_WIDTH + iDeltaX;
            k += 2;
        }
        iTopLeftY += DIR_FILE_ALL_HEIGHT + iDeltaY;
        iTopLeftX = iTopLeftXBak;
    }

    /* 结尾 */
    g_atDirAndFileLayout[k].iTopLeftX   = 0;
    g_atDirAndFileLayout[k].iBotRightX  = 0;
    g_atDirAndFileLayout[k].iTopLeftY   = 0;
    g_atDirAndFileLayout[k].iBotRightY  = 0;
    g_atDirAndFileLayout[k].strIconName = NULL;

}


/**
 * @brief  为"浏览页面"生成菜单区域中的图标
 * 
 * @param  ptPageLayout - 内含多个图标的文件名和显示区域
 * @return int  0      - 成功
 *            其他值 - 失败
 * 
 * @note   提取目录和文件区域中需要的bmp信息，包括fold_closed.bmp、fold_opened.bmp、以及file.bmp
 */

static void GenerateDirAndFileIcons(PT_PageLayout ptPageLayout){
    int iError;
    int iXres, iYres, iBpp;
    T_PixelDatas tOriginIconPixelDatas;

    PT_Layout atLayout = ptPageLayout->atLayout;


    GetDispResolution(&iXres, &iYres, &iBpp);

    
    g_tDirClosedIconPixelDatas.iBpp = iBpp;
    g_tDirClosedIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
    if(NULL == g_tDirClosedIconPixelDatas.aucPixelDatas){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3> malloc error!\n");
        return ;        
    }

    g_tDirOpenedIconPixelDatas.iBpp = iBpp;
    g_tDirOpenedIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
    if(NULL == g_tDirOpenedIconPixelDatas.aucPixelDatas){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3> malloc error!\n");
        return ;        
    }


    g_tFileIconPixelDatas.iBpp = iBpp;
    g_tFileIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
    if(NULL == g_tFileIconPixelDatas.aucPixelDatas){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3> malloc error!\n");
        return ;        
    }
    
    /*
     *  从每个bmp文件中提取图像数据
     */
    // 1. fold_closed.bmp
    iError = GetPixelDatasFrmBMP(g_strDirClosedIconName, &tOriginIconPixelDatas);
    if(iError != 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3> GetPixelDatasFrmBMP <fold_closed.bmp> error!\n");
        return ;
    }
    g_tDirClosedIconPixelDatas.iWidth = atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1;
    g_tDirClosedIconPixelDatas.iHeight = atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1;
    g_tDirClosedIconPixelDatas.iLineBytes = g_tDirClosedIconPixelDatas.iWidth * g_tDirClosedIconPixelDatas.iBpp / 8;
    g_tDirClosedIconPixelDatas.iTotalBytes = g_tDirClosedIconPixelDatas.iHeight * g_tDirClosedIconPixelDatas.iLineBytes;
 
    PicZoom(&tOriginIconPixelDatas, &g_tDirClosedIconPixelDatas);
    if(iError != 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3> PicZoom error!\n");
        return ;
    }
    // 缩放后不需要立即进行合并到大的显存中
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    // 2. fold_opened.bmp
    iError = GetPixelDatasFrmBMP(g_strDirOpenedIconName, &tOriginIconPixelDatas);
    if(iError != 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3> GetPixelDatasFrmBMP <fold_opened.bmp> error!\n");
        return ;
    }

    g_tDirOpenedIconPixelDatas.iHeight = atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1;
    g_tDirOpenedIconPixelDatas.iWidth  = atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1;
    g_tDirOpenedIconPixelDatas.iLineBytes  = g_tDirOpenedIconPixelDatas.iWidth * g_tDirOpenedIconPixelDatas.iBpp / 8;
    g_tDirOpenedIconPixelDatas.iTotalBytes = g_tDirOpenedIconPixelDatas.iLineBytes * g_tDirOpenedIconPixelDatas.iHeight;
    PicZoom(&tOriginIconPixelDatas, &g_tDirOpenedIconPixelDatas);
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    // 3. file.bmp
    iError = GetPixelDatasFrmBMP(g_strFileIconName, &tOriginIconPixelDatas);
    if(iError != 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3> GetPixelDatasFrmBMP <file.bmp> error!\n");
        return ;
    }

    g_tFileIconPixelDatas.iHeight = atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1;
    g_tFileIconPixelDatas.iWidth  = atLayout[0].iBotRightX - atLayout[0].iTopLeftX+ 1;
    g_tFileIconPixelDatas.iLineBytes  = g_tDirClosedIconPixelDatas.iWidth * g_tDirClosedIconPixelDatas.iBpp / 8;
    g_tFileIconPixelDatas.iTotalBytes = g_tFileIconPixelDatas.iLineBytes * g_tFileIconPixelDatas.iHeight;
    PicZoom(&tOriginIconPixelDatas, &g_tFileIconPixelDatas);
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

}


/**
 * @brief  为"浏览页面"生成"目录或文件"区域中的图标和文字,就是显示目录内容
 * 
 * @param  iStartIndex         - 在屏幕上显示的第1个"目录和文件"是aptDirContents数组里的哪一项
 * @param  iDirContentsNumber  - aptDirContents数组有多少项
 * @param  aptDirContents      - 数组:存有目录下"顶层子目录","文件"的名字 
 * @param  ptVideoMem          - 在这个VideoMem中构造页面
 * 
 * @return int                 - 0 成功 
 * 
 * @note   aptDirContents存放了当前目录下的目录和文件信息，根据这个信息判断是文件还是目录，然
 *          后分别使用PicMerge将相应的图标和文字布局到ptVideoMem中
 * 
 */
static int GenerateBrowsePageDirAndFile(int iStartIndex, int iDirContentsNumber, PT_DirContent *aptDirContents, PT_VideoMem ptVideoMem){
    

    int i, j, k = 0;
    int iDirContentIndex = iStartIndex;

    
    PT_PageLayout ptPageLayout = &g_tBrowsePageDirAndFileLayout;
    PT_Layout ptLayout = g_tBrowsePageDirAndFileLayout.atLayout;
    ClearRectangleInVideoMem(ptPageLayout->iTopLeftX, ptPageLayout->iTopLeftY, ptPageLayout->iBotRightX, ptPageLayout->iBotRightY, ptVideoMem, COLOR_BACKGROUND);

    for(i = 0; i < g_iDirFileNumPerCol; i++){
        for(j = 0; j < g_iDirFileNumPerRow; j++){
            
            if(iDirContentIndex < iDirContentsNumber){

                // 将目录或者文件图标数据刷新到内存块中
                if(aptDirContents[iDirContentIndex]->eFileType == FILETYPE_DIR){
                    PicMerge(ptLayout[k].iTopLeftX, ptLayout[k].iTopLeftY, &g_tDirClosedIconPixelDatas, &ptVideoMem->tPixelDatas);
                }
                else
                {
                    PicMerge(ptLayout[k].iTopLeftX, ptLayout[k].iTopLeftY, &g_tFileIconPixelDatas, &ptVideoMem->tPixelDatas);
                }

                k++;
                // 将名称刷新到内存块中
                MergerStringToCenterOfRectangleInVideoMem(ptLayout[k].iTopLeftX, ptLayout[k].iTopLeftY, ptLayout[k].iBotRightX, ptLayout[k].iBotRightY, (unsigned char *)aptDirContents[iDirContentIndex]->strName, ptVideoMem);
                k++;
                iDirContentIndex++;
            }else{
                break;
            }

        }
        if(iDirContentIndex >= iDirContentsNumber){
            break;
        }

    }
    return 0;
} 

/**
 * @brief   显示"浏览页面"
 * @param   ptPageLayout - 内含多个图标的文件名和显示区域
 * @return  无
 * 
 * @author  biabu
 * @date    2025/06-05
 * @version 1.0
 */
static void ShowBrowsePage(PT_PageLayout ptPageLayout){

    int iError;
    (void)iError;

    PT_VideoMem  ptVideoMem;
    // 获得区域图标
    PT_Layout aptLayout = ptPageLayout->atLayout;
    
    // 1. 获取内存块
    ptVideoMem = GetVideoMem(ID("browse"), 1);

    if(ptVideoMem == NULL){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>GetVideoMem error!\n");
        return ;
    }
    DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    // 2. 描画数据
    // iTopLeftX == 0确定当前是否对数据进行描画
    if(aptLayout[0].iTopLeftX == 0){
        // 计算菜单数据
        CalcBrowsePageMenusLayout(ptPageLayout);
        // 计算目录和文件布局
        CalcBrowsePageDirAndFilesLayout();
    }
    /* 生成"目录和文件"的图标 */
    if (!g_tDirClosedIconPixelDatas.aucPixelDatas)
    {   
        GenerateDirAndFileIcons(&g_tBrowsePageDirAndFileLayout);
    }

    iError = GeneratePage(ptPageLayout, ptVideoMem);
    iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptVideoMem);
    // 3. 刷新到显存上
    FlushVideoMemToDev(ptVideoMem);
    // 4. 释放内存块
    PutVideoMem(ptVideoMem);
}


/**
 * @brief  为"浏览页面"获得输入数据,判断输入事件位于哪一个菜单栏图标上
 * 
 * @param  ptPageLayout - 内含多个图标的显示区域
 * @param  ptInputEvent - 内含得到的输入数据
 * 
 * @return  -1     - 输入数据不位于任何一个图标之上
 *            其他值 - 输入数据所落在的图标(PageLayout->atLayout数组的哪一项)

 * @author  biabu
 * @date    2025/06-06
 * @version 1.0
 */
static int BrowsePageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent){
    //return GenericPageGetInputEvent(ptPageLayout, ptInputEvent);
    PT_Layout aptLayout = ptPageLayout->atLayout;
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

/**
 * @brief  为"浏览页面"获得输入数据,判断输入事件位于哪一个"目录或文件"上
 * 
 * @param  ptPageLayout - 内含多个图标的显示区域
 * @param  ptInputEvent - 内含得到的输入数据
 * 
 * @return  -1     - 输入数据不位于任何一个图标之上
 *            其他值 - 输入数据所落在的图标(PageLayout->atLayout数组的哪一项)

 * @author  biabu
 * @date    2025/06-06
 * @version 1.0
 */
static int GetInputPositionInPageLayout(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent){
    PT_Layout atLayout = ptPageLayout->atLayout;
    int i= 0;
    while(atLayout[i].iBotRightY){
        if((ptInputEvent->iX >= atLayout[i].iTopLeftX) && (ptInputEvent->iX <= atLayout[i].iBotRightX) && \
            (ptInputEvent->iY >= atLayout[i].iTopLeftY) && (ptInputEvent->iY <= atLayout[i].iBotRightY)){
            return i;
        }else{
            i++;
        }
    }

    return -1;
}



/**
 * @brief  改变显示设备上的"目录或文件"的图标, 表示"已经被选中"
 *         对于目录图标, 把它改为"file_opened图标"
 *         对于文件图标, 只是把图标反色
 * 
 * @param  iDirFileIndex - 选中"目录和文件"区域中哪一个图标索引
 * @return void
 * 
 * @author  biabu
 * @date    2025/06-05
 * @version 1.0
 */
static void SelectDirFileIcon(int iDirFileIndex){

    int iDirFileContentIndex;
    PT_VideoMem ptDevVideoMem;


    // 获取显存，直接改变显存内容
    ptDevVideoMem = GetDevVideoMem();

    iDirFileIndex = iDirFileIndex & ~1;

    iDirFileContentIndex = iDirFileIndex / 2 + g_iStartIndex;       //iDirFileIndex可能为图标或者文字描述区域的id

    PT_Layout atLayout = g_tBrowsePageDirAndFileLayout.atLayout;
    // 如果是文件，则图标和文字颜色取反
    if(g_aptDirContents[iDirFileContentIndex]->eFileType == FILETYPE_FILE){
        PressButton(&atLayout[iDirFileIndex]);
        PressButton(&atLayout[iDirFileIndex + 1]);
    }else{
        // 如果是目录，则使用file_opened图标
        PicMerge(g_atDirAndFileLayout[iDirFileIndex].iTopLeftX, g_atDirAndFileLayout[iDirFileIndex].iTopLeftY, &g_tDirOpenedIconPixelDatas, &ptDevVideoMem->tPixelDatas);
    }
}

/**
 * @brief  改变显示设备上的"目录或文件"的图标, 表示"未被选中"
 *         对于目录图标, 把它改为"file_closedd图标"
 *         对于文件图标, 只是把图标反色
 * 
 * @param  iDirFileIndex - 选中"目录和文件"区域中哪一个图标索引
 * @return void
 * 
 * @author  biabu
 * @date    2025/06-05
 * @version 1.0
 */
static void DeSelectDirFileIcon(int iDirFileIndex){
    int iDirFileContentIndex;
    PT_VideoMem ptDevVideoMem;


    // 获取显存，直接改变显存内容
    ptDevVideoMem = GetDevVideoMem();

    iDirFileIndex = iDirFileIndex & ~1;

    iDirFileContentIndex = iDirFileIndex / 2 + g_iStartIndex;       //iDirFileIndex可能为图标或者文字描述区域的id

    PT_Layout atLayout = g_tBrowsePageDirAndFileLayout.atLayout;
    // 如果是文件，则图标和文字颜色取反
    if(g_aptDirContents[iDirFileContentIndex]->eFileType == FILETYPE_FILE){
        ReleaseButton(&atLayout[iDirFileIndex]);
        ReleaseButton(&atLayout[iDirFileIndex + 1]);
    }else{
        // 如果是目录，则使用file_opened图标
        PicMerge(g_atDirAndFileLayout[iDirFileIndex].iTopLeftX, g_atDirAndFileLayout[iDirFileIndex].iTopLeftY, &g_tDirClosedIconPixelDatas, &ptDevVideoMem->tPixelDatas);
    }
}


/**
 * @brief  "浏览页面"的运行函数: 显示菜单图标,显示目录内容,读取输入数据并作出反应
 *            "浏览页面"有两个区域: 菜单图标, 目录和文件图标
 *             为统一处理, "菜单图标"的序号为0,1,2,3,..., "目录和文件图标"的序号为1000,1001,1002,....
 * 
 * @param  ptParentPageParams - 内含上一个页面(父页面)的参数
 *                                 ptParentPageParams->iPageID等于ID("setting")时,本页面用于浏览/选择文件夹, 点击文件无反应 
 * @return void
 * 
 * @author  biabu
 * @date    2025/06-05 - 
 * @version 1.0
 */
static void BrowsePageRun(PT_PageParams ptParentParams){
    int iError;
    int iIndex;
    int iDirFileContentIndex;
    int bIconPressed = 0;
    int iIndexPressed = -1;
    int bUsedToSelectDir = 0;

    char strTmp[4096];
    T_PageParams tPageParams;

    T_InputEvent tInputEvent;
    // PrePress表示前一次获取的按下的事件，记录按下的时间长度，用于长按“返回”的时候直接返回
    //T_InputEvent tInputEventPrePress;

    // 上一级页面是setting，当前的浏览页面是为了选择一个目录用于自动播放
    if(ptParentParams->iPageID == ID("setting")){
        bUsedToSelectDir = 1;
    }


    PT_VideoMem ptDevVideoMem;
    // 获取显存，直接改变显存内容
    ptDevVideoMem = GetDevVideoMem();

    // 	/* 这两句只是为了避免编译警告 */
	// tInputEventPrePress.tTime.tv_sec = 0;
	// tInputEventPrePress.tTime.tv_usec = 0;

    // 0. 获得要显示的目录的内容
    iError = GetDirContents(g_strCurDir, &g_aptDirContents, &g_iDirContentsNumber);
    if(iError != 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>GetDirContents error!\n");
        return ;
    }
    // 1. 显示main_page界面
    ShowBrowsePage(&g_tBrowsePageMenuIconsLayout);
    // 2. 创建Prepare线程

    // 调用GetInputEvent获取输入事件处理
    while(1){
        // 先确定是否触摸菜单栏按键，无论是否是菜单按键，这里已经获取了输入事件
        iIndex = BrowsePageGetInputEvent(&g_tBrowsePageMenuIconsLayout, &tInputEvent);
        //printf("iIndex : %d\n", iIndex);
        // 触摸点不是菜单栏
        if(iIndex == -1){
            // 判断触点是否在文件或者目录的图标上           
            iIndex = GetInputPositionInPageLayout(&g_tBrowsePageDirAndFileLayout, &tInputEvent);
            //printf("iIndex : %d\n", iIndex);
            if(iIndex != -1){
                //DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                // iIndex/2是因为iIndex包含文件图标和图标名称
                if (g_iStartIndex + iIndex / 2 < g_iDirContentsNumber)  //判断这个触点上是否有图标
                    iIndex += DIRFILE_ICON_INDEX_BASE; /* 这是"目录和文件图标"的索引值 */
                else
                    iIndex = -1;
            }    
        }
        // 松开的时候
        if(tInputEvent.iPressure == 0){
            //DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            if(bIconPressed){
                // 菜单栏图标
                if(iIndexPressed < DIRFILE_ICON_INDEX_BASE)
                {
                    DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    ReleaseButton(&g_atMenuIconsLayout[iIndexPressed]);
                    bIconPressed = 0;
                    // switch(iIndex){
                    //     case 0:
                    //         DBG_PRINTF("<5> Return Pressed!\n");
                    // }
                }else{
                    DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    // if(iIndexPressed != iIndex){
                    //     // 恢复
                    //     DeSelectDirFileIcon(iIndexPressed - DIRFILE_ICON_INDEX_BASE);
                    //     bIconPressed = 0;   
                    // }else{// 单击文件进入该文件下的页面
                    DeSelectDirFileIcon(iIndexPressed - DIRFILE_ICON_INDEX_BASE);
                    bIconPressed = 0;
                    iDirFileContentIndex = (iIndexPressed - DIRFILE_ICON_INDEX_BASE) / 2 + g_iStartIndex;
                    // 如果是目录，进入该目录
                    if(g_aptDirContents[iDirFileContentIndex]->eFileType == FILETYPE_DIR){
                        // 更改路径
                        snprintf(strTmp, 4096, "%s/%s", g_strCurDir, g_aptDirContents[iDirFileContentIndex]->strName);
                        strTmp[4096 - 1] = '\0';
                        // copy到当前目录下
                        strcpy(g_strCurDir, strTmp);
                        // 释放到之前生成的图标文件等信息
                        FreeDirContents(g_aptDirContents, g_iDirContentsNumber);

                        iError = GetDirContents(g_strCurDir, &g_aptDirContents, &g_iDirContentsNumber);
                        if (iError){
                            DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                            DBG_PRINTF("<3>GetDirContents error!\n");
                            return;
                        }
                        g_iStartIndex = 0;
                        iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptDevVideoMem);        
                    }else if(bUsedToSelectDir == 0){  // 当前不是用于选择目录的，故如果点击文件，单击显示该文件
                        // 获取文件路径
                        snprintf(tPageParams.strCurPicFile, 256, "%s/%s", g_strCurDir, g_aptDirContents[iDirFileContentIndex]->strName);
                        tPageParams.strCurPicFile[255] = '\0';
                        DBG_PRINTF("<3>FILETYPE_FILE!\n");
                        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                        if(isPictureFileSupported(tPageParams.strCurPicFile)){
                            DBG_PRINTF("<3>Enter manual page!\n");
                            tPageParams.iPageID = ID("browse");
                            // tPageParams中存放当前文件的路径和上一层的页面ID，显示文件
                            Page("manual")->Run(&tPageParams);
                            // 返回回来则继续显示当前浏览界面
                            ShowBrowsePage(&g_tBrowsePageMenuIconsLayout);
                        }

                    }

                }
            }
            
        }else{            
            if(iIndex !=-1){
                // 之前没有被按下
                if(!bIconPressed){
                    bIconPressed = 1;
                    // 使用iIndexPressed记录当前的索引值，松开的时候可能不是当前的索引值，也就是手指发生了移动
                    iIndexPressed = iIndex;
                    // 记录当前按下的状态
                    //tInputEventPrePress = tInputEvent;
                    // 菜单栏图标
                    if(iIndex < DIRFILE_ICON_INDEX_BASE){
                        DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                        PressButton(&g_tBrowsePageMenuIconsLayout.atLayout[iIndex]);
                    }else{
                        // 处理按下时候图标的变化
                        DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                        SelectDirFileIcon(iIndex - DIRFILE_ICON_INDEX_BASE);
                        DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    }
                }
                // 长按状态无需处理
            }

        }


    }
}

static T_PageAction g_tBrowsePageAction = {
    .name          = "browse",
    //.GetInputEvent = BrowsePageGetInputEvent,
    .Run           = BrowsePageRun,
    //.Prepare       = BrowsePagePrepare,
};

int BrowsePageInit(void){
    return RegisterPageAction(&g_tBrowsePageAction);
}
