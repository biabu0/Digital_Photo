#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include <sys/time.h>
#include <pthread.h>

#define INPUT_TYPE_STDIN 0  
#define INPUT_TYPE_TOUCHSCREEN 1

#define INPUT_VAL_UP 0
#define INPUT_VAL_DOWN 1
#define INPUT_VAL_EXIT 2
#define INPUT_VAL_UNKNOWN -1

// ���������¼��Ľṹ��
typedef struct InputEvent{
    struct timeval tTime;
    // ��𣺰����໹�Ǵ��������       0����1
    int iType;
    // val ֵ��ʾ����
    int iVal;  

} T_InputEvent, *PT_InputEvent;

typedef struct InputOpr{
    char * name;
    pthread_t pid;
    // ��ʼ�����˳�
    int (*DeviceInit)(void);
    int (*DeviceExit)(void);
    // ��ȡ�����¼�
    int (*GetInputEnvent)(PT_InputEvent ptInputEvent);
    // ����
    struct InputOpr *ptNext;
}T_InputOpr, *PT_InputOpr;

int AllInputDeviceInit(void);
int InputInit(void);
int RegisterInputOpr(PT_InputOpr ptInputOpr);
int ShowInputOpr(void);
int GetInputEvent(PT_InputEvent ptInputEvent);
int TouchScreenInit(void);
int StdinInit(void);

#endif