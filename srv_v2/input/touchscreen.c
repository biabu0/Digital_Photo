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
        g_tTSDEV = ts_open(pcTSName, 1);
    }else{
        g_tTSDEV = ts_open("/dev/input/event1", 1);
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
    struct ts_sample tSamp;
    int iRet;
    
    while(1){
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

/**
 * @brief  获取滑动事件：对于自动播放的图片数据来说就是停止播放，返回；对于电子书来说就是翻页
 * 
 * @param  ptInputEvent 输入事件结构体指针
 * @return int  0- 成功
 *              其他值-失败
 * @note    
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 2.0
 */
int SlipGetInputEvent(PT_InputEvent ptInputEvent){
    // 记录压下的值
    struct ts_sample tSampPressed;
    // 记录松开时候的值
    struct ts_sample tSampReleased;
    struct ts_sample tSamp;
    int iRet;
    int iStart = 0;
    int iDelta;
    
    while(1){
        iRet = ts_read(g_tTSDEV, &tSamp, 1);/* 如果没有数据则休眠 */
        if(iRet == 1){
            // 压下
            if((tSamp.pressure > 0) && (iStart == 0)){
                //第一次按下，记录按下值 
                tSampPressed = tSamp;
                iStart = 1;
            }
            // 松开的时候，记录松开的位置
            if(tSamp.pressure <= 0){
                tSampReleased = tSamp;
                if(!iStart){
                    return -1;
                }else{
                    iDelta = tSampReleased.x - tSampPressed.x;
                    /* 输入事件赋值 */ 
                    ptInputEvent->tTime = tSampReleased.tv;
                    ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
                    if(iDelta > g_iXres / 5){
                        /* 上一页的事件 */
                        ptInputEvent->iVal = INPUT_VAL_UP;

                    }
                    else if(iDelta < 0 - g_iXres / 5){
                        /* 下一页的事件 */
                        ptInputEvent->iVal = INPUT_VAL_DOWN;
                    }
                    else{
                        /* 不是有效的输入事件 */
                        ptInputEvent->iVal = INPUT_VAL_UNKNOWN;
                    }
                    return 0;
                }   
            }
        }
        else{
            return -1;
        }
    }
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





