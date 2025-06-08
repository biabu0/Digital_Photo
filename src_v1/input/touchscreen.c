#include<input_manager.h>
#include<sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <tslib.h>
#include <config.h>
#include<disp_manager.h>

static int g_iXres;
static int g_iYres;
static int g_iBpp;
static int TouchScreenDeviceExit();
static int TouchScreenDeviceInit();
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent);


static struct tsdev *g_tTSDEV;


static int TouchScreenDeviceExit(){
    return 0;
}

/* 由于调用了LCD的分辨率，要在初始化显示屏之后调用 */
static int TouchScreenDeviceInit(){
    
    char *pcTSName = NULL;

    if((pcTSName = getenv("TSLIB_TSDEVICE")) != NULL){
        // 设置为0，以阻塞的方式打开
        g_tTSDEV = ts_open(pcTSName, 0);
    }else{
        g_tTSDEV = ts_open("/dev/input/event1", 0);
    }

    if(!g_tTSDEV){
        DBG_PRINTF("ts_open error!\n");
        return -1;
    }

    if(ts_config(g_tTSDEV)){
        DBG_PRINTF("ts_config error!\n");
        return -1;
    }
    if(GetDispResolution(&g_iXres, &g_iYres, &g_iBpp)){
        DBG_PRINTF("GetDispResolution error!\n");
        return -1;
    }
    return 0;
}
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent){
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    struct ts_sample tSamp;
    int iRet;
    
    while(1){
        DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        iRet = ts_read(g_tTSDEV, &tSamp, 1);/* 如果没有数据则休眠 */
        if(iRet == 1){
            // 压下
            /* 输入事件赋值 */ 
            ptInputEvent->tTime     = tSamp.tv;
            ptInputEvent->iType     = INPUT_TYPE_TOUCHSCREEN;
            ptInputEvent->iX        = tSamp.x;
            ptInputEvent->iY        = tSamp.y;
            ptInputEvent->iPressure = tSamp.pressure;
            
            return 0;            
        }
        else{
            return -1;
        }
    }
    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;
}

static T_InputOpr g_tTouchScreenOpr = {
    .name = "TouchScreen",
    .DeviceExit = TouchScreenDeviceExit,
    .DeviceInit = TouchScreenDeviceInit,
    .GetInputEnvent =TouchScreenGetInputEvent,
};

int TouchScreenInit(void){
    return RegisterInputOpr(&g_tTouchScreenOpr);
}





