#include<file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <config.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/**
 * @brief  ʹ��mmap����ӳ��һ���ļ����ڴ�,�Ժ�Ϳ���ֱ��ͨ���ڴ��������ļ�
 * 
 * @param  ptFileMap - �ں��ļ���strFileName
 * @return int  0      - �ɹ�
 *            ����ֵ - ʧ��
 * 
 * @note   ʹ��open����ļ�����������ļ����ʹ��fstat��ȡ�ļ���Ϣ����ʹ��mmap���ļ��ڴ�ӳ�䣬����ptFileMap
 * 
 *              ptFileMap - tFp           : ���򿪵��ļ����
 *                          iFileSize     : �ļ���С
 *                          pucFileMapMem : ӳ���ڴ���׵�ַ
 * @author  bia��
 * @date    2025/06/05->06/13
 * @version v2.0��open��������fopen
 */
int MapFile(PT_FileMap ptFileMap){

    int iFd;
    FILE *tFp;
    struct stat tStat;

    tFp = fopen(ptFileMap->strFileName, "r+");
    if (tFp == NULL)
	{
		DBG_PRINTF("can't open %s\n", ptFileMap->strFileName);
		return -1;
	}
    ptFileMap->tFp = tFp;
    iFd = fileno(tFp);

	fstat(iFd, &tStat);
    ptFileMap->iFileSize = tStat.st_size;
	ptFileMap->pucFileMapMem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
	if(ptFileMap->pucFileMapMem == (void *)-1){
		DBG_PRINTF("mmap error\n");
        return -1;
	}

    return 0;
}

/**
 * @brief  ʹ��unmmap�������ӳ�䣬�ر��ļ����
 * 
 * @param  ptFileMap - �ں�ӳ���ַ���ļ����
 * @return void
 * 
 * @note   ʹ��munmap����ڴ�ӳ��
 * @author  bia��
 * @date    2025/06/05
 * @version 1.0
 */
void UnMapFile(PT_FileMap ptFileMap){
    // munmap(ptFileMap->pucFileMapMem, ptFileMap->iFileSize);
    
    // if (ptFileMap == NULL || ptFileMap->tFp == NULL) {
    //     DBG_PRINTF("Invalid file pointer\n");
    //     return;
    // }
    // fclose(ptFileMap->tFp);
    // if(fclose(ptFileMap->tFp) == EOF){
    //     perror("fclose failed");
    // }

    if (ptFileMap == NULL) {
        DBG_PRINTF("Invalid PT_FileMap pointer\n");
        return;
    }

    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    
    // �ȼ��ָ���ٲ���
    if (ptFileMap->pucFileMapMem != NULL) {
        munmap(ptFileMap->pucFileMapMem, ptFileMap->iFileSize);
        ptFileMap->pucFileMapMem = NULL; // ��ֹҰָ��
    }

    DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    
    // ���ιر��ļ�ָ��
    // if (ptFileMap->tFp != NULL) {
    //     if (fclose(ptFileMap->tFp) == EOF) {
    //         perror("fclose failed");
    //     }
    //     ptFileMap->tFp = NULL; // ���Ϊ���ͷ�
    // }
}


/**********************************************************************
 * �������ƣ� isDir
 * ���������� �ж�һ���ļ��Ƿ�ΪĿ¼��ʹ��stat��ȡ�ļ����ԣ��ļ�������Ϣ������stat�ṹ��st_mode�У�ʹ�ú꼴��ȷ���ļ�����
 * ��������� strFilePath - �ļ���·��
 *            strFileName - �ļ�������
 * ��������� ��
 * �� �� ֵ�� 0 - ����Ŀ¼
 *            1 - ��Ŀ¼
 * �޸�����        �汾��     �޸���	      �޸�����
 * -----------------------------------------------
 * 2025/06/06	     V1.0	  baibu	      ����
 ***********************************************************************/

static int isDir(char *strFilePath, char *strFileName){
    int iError;
    struct stat tbuf;

    char strTmp[FILE_NAME_SIZE];

    snprintf(strTmp, FILE_NAME_SIZE, "%s/%s", strFilePath, strFileName);
    strTmp[FILE_NAME_SIZE - 1] = '\0';    
    iError = stat(strTmp, &tbuf);
    if(iError != 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>stat error! Path : %s\n", strTmp);
        return 0;
    }
    //DBG_PRINTF("<3>Path : %s\n", strTmp);

    if(S_ISDIR(tbuf.st_mode)){
        return 1;
    }else{
        return 0;
    }
}


/**********************************************************************
 * �������ƣ� isDir
 * ���������� �ж�һ���ļ��Ƿ�ΪĿ¼��ʹ��stat��ȡ�ļ����ԣ��ļ�������Ϣ������stat�ṹ��st_mode�У�ʹ�ú꼴��ȷ���ļ�����
 * ��������� strFilePath - �ļ���·��
 *            strFileName - �ļ�������
 * ��������� ��
 * �� �� ֵ�� 0 - ����Ŀ¼
 *            1 - ��Ŀ¼
 * �޸�����        �汾��     �޸���	      �޸�����
 * -----------------------------------------------
 * 2025/06/06	     V1.0	  baibu	      ����
 ***********************************************************************/
static int isRegFile(char *strFilePath, char *strFileName)
{
    char strTmp[FILE_NAME_SIZE];
    struct stat tStat;

    snprintf(strTmp, FILE_NAME_SIZE, "%s/%s", strFilePath, strFileName);
    strTmp[FILE_NAME_SIZE - 1] = '\0';

    if ((stat(strTmp, &tStat) == 0) && S_ISREG(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/**
 * @brief  ��ĳĿ¼�������Ķ�����Ŀ¼�������ļ�����¼����,���Ұ���������
 * 
 * @param  strDirName - Ŀ¼��(������·��)
 * @param  pptDirContents - (*pptDirContents)ָ��һ��PT_DirContent����,
 *                             (*pptDirContents)[0,1,...]ָ��T_DirContent�ṹ��,
 *                             T_DirContent�к���"Ŀ¼/�ļ�"�����ֵ���Ϣ
 * @param  piNumber       - strDirName�º��ж��ٸ�"������Ŀ¼/�����ļ�",
 *                             ������(*pptDirContents)[0,1,...]�ж�����
 * @return    0 - �ɹ�
 *            1 - ʧ��
 * 
 * @note   ����scandir������ȡ��ǰĿ¼�µ������ļ���Ȼ��isDir���ж��Ƿ���Ŀ¼�����ļ���
 *         ��������ļ����ƴ洢��pptDirContents��
 * 
 * @author  biabu
 * @date    2025/06-05
 * @version 1.0
 */
int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *piNumber){
    
    int iNum;
    int i = 0;
    int j = 0;
    struct dirent **aptNameList;
    PT_DirContent *aptDirContents;

   
    /* 1. ɨ��Ŀ¼
     * �ú���ɨ��Ŀ¼�µ�������Ŀ¼���ļ�������aptNameList[0],aptNameList[1],... 
     * alphasort����ʾ����ĸ˳������
     * */
    iNum = scandir(strDirName, &aptNameList, 0, alphasort);
    if(iNum < 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>scandir error : %s!\n", strDirName);
        return -1;
    }

    /* 2. ����ռ�
     * scandir ��ɨ����Ĭ�ϻ������ǰĿ¼��.���͸�Ŀ¼��..�����ȥ�����������iNum-2���ռ�
     */ 
    aptDirContents = malloc(sizeof(PT_DirContent) * (iNum - 2));
    if(NULL == aptDirContents){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>malloc error!\n");
        return -1;
    }

    *pptDirContents = aptDirContents;
    for(i = 0; i < iNum - 2; i++){
        aptDirContents[i] = malloc(sizeof(T_DirContent));
        if(NULL == aptDirContents[i]){
            // ��Դ����
            while(i-- > 0) free(aptDirContents[i]);
            free(aptDirContents);
            DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            DBG_PRINTF("<3>malloc error!\n");
            return -1;
        }
    }

    /* 3. ����ȡ��Ŀ¼���뵽ptDirContents */
    DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    for(i = 0, j = 0; i < iNum; i++){
        // ������Ŀ¼.��..
        if(0 == strcmp(aptNameList[i]->d_name, ".") || (0 == strcmp(aptNameList[i]->d_name, ".."))){
            continue;
        }
        if(isDir(strDirName, aptNameList[i]->d_name)){
            strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, FILE_NAME_SIZE);
            aptDirContents[j]->strName[FILE_NAME_SIZE - 1] = '\0';
            aptDirContents[j]->eFileType = FILETYPE_DIR;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
            j++;
        }
    }

    /* 4. �������ļ����뵽ptDirContents */ 
    for(int i = 0; i < iNum; i++){
        if(aptNameList[i] == NULL){
            continue;
        }
        // ������Ŀ¼.��..
        if(0 == strcmp(aptNameList[i]->d_name, ".") || (0 == strcmp(aptNameList[i]->d_name, ".."))){
            continue;
        }
        if(isRegFile(strDirName, aptNameList[i]->d_name)){
            strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, FILE_NAME_SIZE);
            aptDirContents[j]->strName[FILE_NAME_SIZE - 1] = '\0';
            aptDirContents[j]->eFileType = FILETYPE_FILE;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
            j++;
        }
    }

    /* 5. �ͷ�δʹ�õ�aptDirContents���д�ļ���Ŀ¼���ܲ��ǳ�����ļ���Ŀ¼��ͳ�Ƴ�����iNum����������aptDirContents��*/
    for(i = j; i < iNum - 2; i++){
        free(aptDirContents[i]);
    }

    /* 6. �ͷ�aptNameList�л�δ�ͷŵ��ͬ5*/

    for(i = 0; i < iNum; i++){
        if(aptNameList[i])
            free(aptNameList[i]);
    }

    free(aptNameList);

    *piNumber = j;

    return 0;
}


void FreeDirContents(PT_DirContent *aptDirContents, int iNumber){
	int i;
	for (i = 0; i < iNumber; i++)
	{
		free(aptDirContents[i]);
	}
	free(aptDirContents);
}