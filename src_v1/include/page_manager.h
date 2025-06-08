#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include<input_manager.h>

typedef struct Layout{
    int iTopLeftX;
    int iTopLeftY;
    int iBotRightX;
    int iBotRightY;
    char *strIconName;
}T_Layout, *PT_Layout;

// 每一个页面的结构体
typedef struct PageAction{
    char *name;
    void (*Run)(void);
    int (*GetInputEvent)(PT_Layout aptLayout, PT_InputEvent ptInputEvent);
    int (*Prepare)(void);       //准备可能得下一步操作的界面，确保用户在点击的时候能够快速响应
    struct PageAction *ptNext;
}T_PageAction, *PT_PageAction;

// 通过字符串确定ID
//#define ID(name) (int(name[0]) + int(name[1]) + int(name[2]) + int(name[3]))

int ID(char *strName);
int RegisterPageAction(PT_PageAction ptPageAction);
int MainPageInit(void);
void ShowPages(void);
int PagesInit(void);
PT_PageAction Page(char *pcName);

#endif