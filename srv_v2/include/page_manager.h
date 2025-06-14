#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include<input_manager.h>
#include<disp_manager.h>


// 描述整个页面的布局
typedef struct PageLayout{
	int iTopLeftX;        /* 这个区域的左上角、右下角坐标 */
	int iTopLeftY;
	int iBotRightX;
	int iBotRightY;
	int iBpp;             /* 一个象素用多少位来表示 */
	int iMaxTotalBytes;     
    // 描述页面中各个功能子区域
    PT_Layout atLayout;
}T_PageLayout, *PT_PageLayout;

typedef struct PageParams{
    int iPageID;                    // 页面ID
    char strCurPicFile[256];        //要处理的第一个图片文件 
}T_PageParams, *PT_PageParams;


// 每一个页面的结构体
typedef struct PageAction{
    char *name;
    void (*Run)(PT_PageParams ptParentParams);
    int (*GetInputEvent)(PT_Layout aptLayout, PT_InputEvent ptInputEvent);
    int (*Prepare)(void);       //准备可能得下一步操作的界面，确保用户在点击的时候能够快速响应
    struct PageAction *ptNext;
}T_PageAction, *PT_PageAction;

// 通过字符串确定ID
//#define ID(name) (int(name[0]) + int(name[1]) + int(name[2]) + int(name[3]))

int ID(char *strName);
int RegisterPageAction(PT_PageAction ptPageAction);
int MainPageInit(void);
int BrowsePageInit(void);
void ShowPages(void);
int PagesInit(void);
int ExplorePageInit(void);
PT_PageAction Page(char *pcName);
int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem);
int GenericPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent);

#endif