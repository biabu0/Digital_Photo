#include <config.h>
#include <debug_manager.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


static int StdoutDebugPrint(char *strData) {
    /* ��׼����� ֱ�ӽ������Ϣ��printf��ӡ���� */
    printf("%s", strData);

    return strlen(strData);// �����Ѿ��ɹ���ӡ���ַ���
}


// ����ע��һ���ṹ��
static T_DebugOpr g_tStdoutDebugOpr = {
    .canUsed = 1,
    .name = "stdout",
    .DebugPrint = StdoutDebugPrint,
    // ���ڱ�׼���������Ҫ��ʲô��ʼ�����˳�����
    // �������ֱ������Ϊ�ռ��ɣ���������Ҫ����������������ʱ����Ҫ�ж��Ƿ��������������
    // .DebugExit = StdoutDebugExit,
    // .DebugInit = StdoutDebugInit,
};

// ע��ṹ��
int StdoutInit(void){
    return RegisterDebugOpr(&g_tStdoutDebugOpr);
}


