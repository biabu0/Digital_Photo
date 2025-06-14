#include <config.h>
#include <debug_manager.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// ����һ���˿�ֵ
#define SERVER_PORT 8888
#define PRINT_BUF_SIZE  (16*1024)

// ȫ�ֱ�������Ҫ�ڶ��������ʹ��
static int g_iSocketServer;
static struct sockaddr_in g_tSocketServerAddr;
static struct sockaddr_in g_tSocketClientAddr;

static int g_iHaveConnected = 0;

// ��һ��ʹ�������ӡ���ʲ������������ʽ��ֻ�е�ʹ�������ӡ��ʱ����Ϊ�����ռ�
static char *g_pcNetPrintBuf;
// ���λ�������дλ��
static int g_iReadPos = 0;
static int g_iWritePos = 0;
// �߳�ID
static pthread_t g_tSendThreadID;
static pthread_t g_tRecvThreadID;

// ���̷߳��ʹ�����Դ��Ҫ����
static pthread_mutex_t g_tSendMutex = PTHREAD_MUTEX_INITIALIZER;
// ����������һ��ͬ�����ƣ��뻥����һ��ʹ�������߳����޾����ķ�ʽ�ȴ��ض��¼��ķ���
static pthread_cond_t g_tSendConVar = PTHREAD_COND_INITIALIZER;


// дλ�ü�һģ���ȵ��ڶ�λ�ü�Ϊ��
static int isFull(void){
    return (g_iWritePos + 1) % PRINT_BUF_SIZE == g_iReadPos;
}
// ��λ�õ���дλ�ü�Ϊ��
static int isEmpty(void){ 
    return g_iReadPos == g_iWritePos;
}
static int PutData(char cVal){
    if(isFull()){
        // ����������������д��
        return -1;
    }else{
        g_pcNetPrintBuf[g_iWritePos] = cVal;
        g_iWritePos = (g_iWritePos + 1) % PRINT_BUF_SIZE;
        return 0;
    }
}


static int GetData(char *pcVal){ 
    if(isEmpty()){
        return -1;
    }else{
        *pcVal = g_pcNetPrintBuf[g_iReadPos];
        g_iReadPos = (g_iReadPos + 1) % PRINT_BUF_SIZE;
        return 0;
    }
}

static void *NetDebugSendThread(void *pVoid){

    char strTmpBuf[512];
    int i;
    char cVal;

    while(1){
        /* ����״̬ �����ͻ������ӳɹ����һ��λ�����������ʱ�������߳� */
        pthread_mutex_lock(&g_tSendMutex);
        pthread_cond_wait(&g_tSendConVar, &g_tSendMutex);
        pthread_mutex_unlock(&g_tSendMutex);
        /* �пͻ������Ӳ��һ������������� */
        while(g_iHaveConnected && !isEmpty()){
            i = 0;
            /* �����ݴӻ��λ�������ȡ�����ݣ����뵽Ҫ���͵Ļ������� */
            while((i < 512) && (0 == GetData(&cVal))){
                strTmpBuf[i] = cVal;
                i++;
            }
            /* ʹ��sendto�������ʹ�ӡ��Ϣ���ͻ��� */
            sendto(g_iSocketServer, strTmpBuf, i, 0, (const struct sockaddr *)&g_tSocketClientAddr, sizeof(struct sockaddr));
        }
        
    }
    return NULL;
}

static void *NetDebugRecvThread(void *pVoid){
    int iAddrLen;
    int iRecvLen;
    char cRecvBuf[1000];
    /* �ͻ��˵�ַ */
    struct sockaddr_in tSocketClientAddr;
    while(1){
        iAddrLen = sizeof(struct sockaddr_in);
        iRecvLen = recvfrom(g_iSocketServer, cRecvBuf, 999, 0, (struct sockaddr *)&tSocketClientAddr, (socklen_t *)&iAddrLen);
        if(iRecvLen > 0){
            /* ��������� */
            cRecvBuf[iRecvLen] = '\0';
            /* �������ݣ�
            * dbglevel=0,1,2,3...    :�޸Ĵ�ӡ����
            * stdout = 0/1           :�ر�/�򿪱�׼�����ӡ
            * netprint = 0/1         :�ر�/�������ӡ
            * setclient              :���ÿͻ��˵�ַ
            **/
           
           if(strcmp(cRecvBuf, "setclient") == 0){
            /* ���ÿͻ��˵�ַ ��ȫ�ֱ������������̣߳����ڷ����߳���õ�ַ�������� */
                g_tSocketClientAddr = tSocketClientAddr;
                g_iHaveConnected = 1;
           }else if(strncmp(cRecvBuf, "dbglevel=", 9)  == 0){
            /* ���õ��Եȼ� ��������Ϣ��������Ϣ����ʾ��Ϣ��*/
                SetDebugLevel(cRecvBuf);
           }else{
            /* ���õ���ͨ�� ��*/
                SetDebugChannel(cRecvBuf);
           }

        }

    }

    return NULL;
}


static int NetDebugInit(void){
    /* �����̳�ʼ����socket��ʼ�� ѡ��˿ڣ�ip*/
    /* ʹ��udpЭ�� */
    int iRet;
    g_iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);
    if(g_iSocketServer == -1){
        printf("server: socket error!\n");
        return -1;
    }
    g_tSocketServerAddr.sin_port = htons(SERVER_PORT);
    g_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
    g_tSocketServerAddr.sin_family = AF_INET;
    memset(g_tSocketServerAddr.sin_zero, 0, 8);
    iRet = bind(g_iSocketServer, (struct sockaddr *)&g_tSocketServerAddr, sizeof(g_tSocketServerAddr));
    if(iRet == -1){
        printf("server: bind error!\n");
        return -1;
    }

    // ��ʼ����ʱ���ʾҪʹ�������ӡ�����ʱ����Է��仺�����ڴ�
    g_pcNetPrintBuf = malloc(PRINT_BUF_SIZE);
    // �������ڴ������󣬹ر�socket
    if(g_pcNetPrintBuf == NULL){
        close(g_iSocketServer);
        return -1;
    }

    /* ���������̣߳����ڷ��ʹ�ӡ��Ϣ���ͻ��� */
    pthread_create(&g_tSendThreadID, NULL, NetDebugSendThread, NULL);
    /* ���������̣߳����ڽ��տ�����Ϣ���޸Ĵ�ӡ���𣬴�/�رմ�ӡ��*/
    pthread_create(&g_tRecvThreadID, NULL, NetDebugRecvThread, NULL);

    return 0;

}

static int NetDebugExit(void){
    /* �ر�socket,�������� ... */
    close(g_iSocketServer);
    free(g_pcNetPrintBuf);
    return 0;
}



static int NetDebugPrint(char *strData){
    /* �����ݷ��뵽���λ�������*/
    int i;
    /* ����ʱ�������е����ݷ��뵽�����ӡ�������� */
    for(i = 0; i < strlen(strData); i++){
        if(0 != PutData(strData[i])){
            break;
        }
    }

    /* ����пͻ������ӣ�������ͨ�����緢�͸��ͻ��� : ����ʹ��sendto���߻����߳� */
    /* ����netprint�ķ����߳� */
    pthread_mutex_lock(&g_tSendMutex);
    pthread_cond_signal(&g_tSendConVar);
    pthread_mutex_unlock(&g_tSendMutex);

    return i;
}

static T_DebugOpr g_tNetDebugOpr = {
    .canUsed = 1,
    .name = "netprint",
    .DebugPrint = NetDebugPrint,
    .DebugExit = NetDebugExit,
    .DebugInit = NetDebugInit,
};


int NetPrintInit(void)
{
    return RegisterDebugOpr(&g_tNetDebugOpr);
}


