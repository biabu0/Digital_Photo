#include<config.h>
#include<page_manager.h>

static T_PageAction g_tBrowsePageAction = {
    .name          = "browse",
    .GetInputEvent = BrowsePageGetInputEvent,
    .Run           = BrowsePageRun,
    .Prepare       = BrowsePagePrepare,
};


static int BrowsePageRun(){
    // 1.显示main_page界面

    // 2.创建Prepare线程

    // 调用GetInputEvent获取输入事件处理
    while(1){
        input = BrowsePageGetInputEvent();
        switch(input){
            // 返回则是从文件浏览状态返回回去，上一层的代码已经做了保存界面和恢复界面的处理，只需直接return 0即可
            case "返回":
            {
                return 0;
            }
            case "缩小":
            {
                // 显示缩小界面
                break;
            }
            case "放大":
            {
                // 显示放大界面
                break;
            }

            case "上一幅":
            {
                // 显示上一幅界面
                break;
            }
            case "下一幅":
            {
                // 显示下一幅界面
                break;
            }
            case "连播":
            {
                // 显示连播界面
                Page("auto")->Run();
                break;
            }
            case "按住不放"{
                // 显示移动的图片
            }
        }
    }
}

int BrowsePageInit(void){
    return RegisterPageAction(&g_tBrowsePageAction);
}
