#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

// ÿһ��ҳ��Ľṹ��
typedef struct PageAction{
    char *name;
    int (*Run)(void);
    int (*GetInputEvent)();
    int (*Prepare)();       //׼�����ܵ���һ�������Ľ��棬ȷ���û��ڵ����ʱ���ܹ�������Ӧ
    struct PageAcction *ptNext;
}T_PageAction, *PT_PageAction;


#endif