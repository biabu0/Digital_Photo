#include<config.h>
#include<page_manager.h>

static T_PageAction g_tExplorePageAction = {
    .name          = "explpre",
    .GetInputEvent = ExplorePageGetInputEvent,
    .Run           = ExplorePageRun,
    .Prepare       = ExplorePagePrepare,
};


static int ExplorePageRun(){
    // 1.显示main_page界面

    // 2.创建Prepare线程

    // 调用GetInputEvent获取输入事件处理
    while(1){
        input = ExplorePageGetInputEvent();
        switch(input){
            // 上一层
            case "向上":
            {
                // 判断是否位于顶层
                if(isTopLevel){
                    return 0;   //返回到main_page中的Restore恢复顶层界面
                }else{
                    // 显示上一个目录页面
                }
                break;
            }
            case "选择":
            {
                // 选择的是否是下一级目录，不是则为文件，文件则使用browse打开预览
                if(isSelectDir){

                }else{
                    /* 保存当前页面 */
                    StorePage();
                    Page("browse")->Run();
                    /* 恢复之前页面 */
                    RestorePage();
                }
                break;
            }
            case "下一页":
            {
                //  保存当前页面
                StorePage();
                // 显示下一页

                // 恢复之前页面
                RestorePage();
                break;
            }

            case "上一页":
            {
                //  保存当前页面
                StorePage();
                // 显示上一页

                // 恢复之前页面
                RestorePage();
                break;
            }
        }
    }
}

int ExplorePageInit(void){
    return RegisterPageAction(&g_tExplorePageAction);
}
