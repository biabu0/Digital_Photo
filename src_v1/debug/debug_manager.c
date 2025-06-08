#include <config.h>
#include <debug_manager.h>
#include <string.h>
#include<stdio.h>
#include <stdarg.h>

static PT_DebugOpr g_ptDebugOprHead = NULL;
// ��ʼ��������Ϊ4
static int g_iDebugLevelLimit = DEFAULT_DEBUGLEVEL;
int RegisterDebugOpr(PT_DebugOpr ptDebugOpr){
    PT_DebugOpr ptTmp;

    if(!g_ptDebugOprHead){
        g_ptDebugOprHead = ptDebugOpr;
        ptDebugOpr->ptNext = NULL;
    }else{
        ptTmp = g_ptDebugOprHead;
        while(ptTmp->ptNext){
            ptTmp = ptTmp->ptNext;
        }
        ptTmp->ptNext = ptDebugOpr;
        ptDebugOpr->ptNext = NULL;
    }
    return 0;
}

int ShowDebugOpr(void){

    PT_DebugOpr ptTmp = g_ptDebugOprHead;
    int i = 0;

    while(ptTmp){
        printf("%02d %s\n", i++, ptTmp->name);
        ptTmp = ptTmp->ptNext;
    }
    return 0;
}

PT_DebugOpr GetDebugOpr(char *pcName){

	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while(ptTmp){
		if(strcmp(ptTmp->name, pcName) == 0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}


// dbglevel=0,1,2,3...7
int SetDebugLevel(char *strBuf){
    g_iDebugLevelLimit = strBuf[9] - '0';
    return 0;
}


// * stdout = 0/1           :�ر�/�򿪱�׼�����ӡ
// * netprint = 0/1         :�ر�/�������ӡ
int SetDebugChannel(char *strBuf){
    char *pStrTmp;
    char strName[100];
    PT_DebugOpr ptDebugOprTmp;
    /* ��strBuf�е�һ�γ���"="��λ�� */
    pStrTmp = strchr(strBuf, (int)'=');
    if(pStrTmp == NULL){
        return -1;
    }else{
        strncpy(strName, strBuf, pStrTmp - strBuf);
        strName[pStrTmp - strBuf] = '\0';
        ptDebugOprTmp = GetDebugOpr(strName);
        if(!ptDebugOprTmp){
            return -1;
        }
        if(pStrTmp[1] == '0')
            ptDebugOprTmp->canUsed = 0;
        else
            ptDebugOprTmp->canUsed = 1;
        
        return 0;
    }
}

int DebugPrint(const char *pcFormat, ...){
    char strTmpBuf[1000];
    char *pcTmp = strTmpBuf;
    int dbglevel = DEFAULT_DEBUGLEVEL;
    PT_DebugOpr ptTmp = g_ptDebugOprHead;

    va_list tArgs;
    int iNum;
    

    va_start(tArgs, pcFormat);
    iNum = vsprintf(strTmpBuf, pcFormat, tArgs);
    strTmpBuf[iNum] = '\0';
    va_end(tArgs);

    /* ���ݴ�ӡ�����ж��Ƿ��ӡ */
    if((strTmpBuf[0] == '<') && strTmpBuf[2] == '>'){
        //��ǰ��Ϣ�Ĵ�ӡ����
        dbglevel = strTmpBuf[1] - '0';
        if(dbglevel >= 0 && dbglevel <= 9){
            /* ��ǰ��ӡ��������Ч�ģ���ӡ��ʱ�򲻴�ӡ��������ַ� */
            pcTmp = strTmpBuf + 3;
        }else{
            dbglevel = DEFAULT_DEBUGLEVEL;
        }
    }
    if(dbglevel > g_iDebugLevelLimit){
        return -1;
    }
    while(ptTmp){
        if(ptTmp->canUsed){
            ptTmp->DebugPrint(pcTmp);
        }
        ptTmp = ptTmp->ptNext;
    }

    return 0;
}

int DebugInit(void){
    int iError;
    iError = StdoutInit();
    iError |= NetPrintInit();
    return iError;
}

int InitDebugChannel(void){ 
    PT_DebugOpr ptTmp = g_ptDebugOprHead;
    while(ptTmp){
        if(ptTmp->canUsed && ptTmp->DebugInit){
            ptTmp->DebugInit();
        }
        ptTmp = ptTmp->ptNext;
    }
    return 0;
}