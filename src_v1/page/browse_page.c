#include<config.h>
#include<page_manager.h>

static T_PageAction g_tBrowsePageAction = {
    .name          = "browse",
    .GetInputEvent = BrowsePageGetInputEvent,
    .Run           = BrowsePageRun,
    .Prepare       = BrowsePagePrepare,
};


static int BrowsePageRun(){
    // 1.��ʾmain_page����

    // 2.����Prepare�߳�

    // ����GetInputEvent��ȡ�����¼�����
    while(1){
        input = BrowsePageGetInputEvent();
        switch(input){
            // �������Ǵ��ļ����״̬���ػ�ȥ����һ��Ĵ����Ѿ����˱������ͻָ�����Ĵ���ֻ��ֱ��return 0����
            case "����":
            {
                return 0;
            }
            case "��С":
            {
                // ��ʾ��С����
                break;
            }
            case "�Ŵ�":
            {
                // ��ʾ�Ŵ����
                break;
            }

            case "��һ��":
            {
                // ��ʾ��һ������
                break;
            }
            case "��һ��":
            {
                // ��ʾ��һ������
                break;
            }
            case "����":
            {
                // ��ʾ��������
                Page("auto")->Run();
                break;
            }
            case "��ס����"{
                // ��ʾ�ƶ���ͼƬ
            }
        }
    }
}

int BrowsePageInit(void){
    return RegisterPageAction(&g_tBrowsePageAction);
}
