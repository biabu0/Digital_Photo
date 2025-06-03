#include<config.h>
#include<page_manager.h>

static T_PageAction g_tExplorePageAction = {
    .name          = "explpre",
    .GetInputEvent = ExplorePageGetInputEvent,
    .Run           = ExplorePageRun,
    .Prepare       = ExplorePagePrepare,
};


static int ExplorePageRun(){
    // 1.��ʾmain_page����

    // 2.����Prepare�߳�

    // ����GetInputEvent��ȡ�����¼�����
    while(1){
        input = ExplorePageGetInputEvent();
        switch(input){
            // ��һ��
            case "����":
            {
                // �ж��Ƿ�λ�ڶ���
                if(isTopLevel){
                    return 0;   //���ص�main_page�е�Restore�ָ��������
                }else{
                    // ��ʾ��һ��Ŀ¼ҳ��
                }
                break;
            }
            case "ѡ��":
            {
                // ѡ����Ƿ�����һ��Ŀ¼��������Ϊ�ļ����ļ���ʹ��browse��Ԥ��
                if(isSelectDir){

                }else{
                    /* ���浱ǰҳ�� */
                    StorePage();
                    Page("browse")->Run();
                    /* �ָ�֮ǰҳ�� */
                    RestorePage();
                }
                break;
            }
            case "��һҳ":
            {
                //  ���浱ǰҳ��
                StorePage();
                // ��ʾ��һҳ

                // �ָ�֮ǰҳ��
                RestorePage();
                break;
            }

            case "��һҳ":
            {
                //  ���浱ǰҳ��
                StorePage();
                // ��ʾ��һҳ

                // �ָ�֮ǰҳ��
                RestorePage();
                break;
            }
        }
    }
}

int ExplorePageInit(void){
    return RegisterPageAction(&g_tExplorePageAction);
}
