#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

// 每一个页面的结构体
typedef struct PageAction{
    char *name;
    int (*Run)(void);
    int (*GetInputEvent)();
    int (*Prepare)();       //准备可能得下一步操作的界面，确保用户在点击的时候能够快速响应
    struct PageAcction *ptNext;
}T_PageAction, *PT_PageAction;


#endif