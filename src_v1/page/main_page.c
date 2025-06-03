#include<config.h>
#include<page_manager.h>

static T_PageAction g_tMainPageAction = {
    .name          = "main",
    .GetInputEvent = MainPageGetInputEvent,
    .Run           = MainPageRun,
    .Prepare       = MainPagePrepare,
};


static void ShowMainPage(){
    // 1. 获得显存

    // 2. 描画数据

    // 3. 刷新到数据上

}

static int MainPageRun(){
    // 1.显示main_page界面

    ShowMainPage();

    // 2.创建Prepare线程：用户可能会停顿部分时间，在这段时间将可能得下一个页面准备好，便于快速切换，流畅

    // 调用GetInputEvent获取输入事件处理
    while(1){
        input = MainPageGetInputEvent();
        switch(input){
            case "浏览":
            {
                /* 保存当前页面 */
                StorePage();
                Page("explore")->Run();
                /* 恢复之前页面 */
                RestorePage();
                break;
            }
            case "设置":
            {
               /* 保存当前页面 */
                StorePage();
                Page("setting")->Run();
                /* 恢复之前页面 */
                RestorePage();
                break;
            }
            case "连播":
            {
                /* 保存当前页面 */
                StorePage();
                Page("auto")->Run();
                /* 恢复之前页面 */
                RestorePage();
                break;
            }
        }
    }
}

int MainPageGetInputEvent(void){
    // 获取原始触摸屏数据:调用input_manager.c中函数，使得当前线程处于休眠状态，有触摸屏数据到来的时候唤醒该线程
    GetInputEvent();

    // 处理数据
}

int MainPageInit(void){
    return RegisterPageAction(&g_tMainPageAction);
}
