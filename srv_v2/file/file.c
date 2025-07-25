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


//为避免访问的目录互相嵌套, 设置能访问的目录深度为10
#define MAX_DIR_DEEPNESS 10

/**
 * @brief  使用mmap函数映射一个文件到内存,以后就可以直接通过内存来访问文件
 * 
 * @param  ptFileMap - 内含文件名strFileName
 * @return int  0      - 成功
 *            其他值 - 失败
 * 
 * @note   使用open获得文件句柄，根据文件句柄使用fstat获取文件信息，并使用mmap将文件内存映射，完善ptFileMap
 * 
 *              ptFileMap - tFp           : 所打开的文件句柄
 *                          iFileSize     : 文件大小
 *                          pucFileMapMem : 映射内存的首地址
 * @author  bia布
 * @date    2025/06/05->06/13
 * @version v2.0：open函数改用fopen
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
 * @brief  使用unmmap函数解除映射，关闭文件句柄
 * 
 * @param  ptFileMap - 内含映射地址和文件句柄
 * @return void
 * 
 * @note   使用munmap解除内存映射
 * @author  bia布
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
    
    // 先检查指针再操作
    if (ptFileMap->pucFileMapMem != NULL) {
        munmap(ptFileMap->pucFileMapMem, ptFileMap->iFileSize);
        ptFileMap->pucFileMapMem = NULL; // 防止野指针
    }
    
    // 单次关闭文件指针
    // if (ptFileMap->tFp != NULL) {
    //     if (fclose(ptFileMap->tFp) == EOF) {
    //         perror("fclose failed");
    //     }
    //     ptFileMap->tFp = NULL; // 标记为已释放
    // }
}


/**********************************************************************
 * 函数名称： isDir
 * 功能描述： 判断一个文件是否为目录：使用stat获取文件属性，文件类型信息包含在stat结构的st_mode中，使用宏即可确定文件类型
 * 输入参数： strFilePath - 文件的路径
 *            strFileName - 文件的名字
 * 输出参数： 无
 * 返 回 值： 0 - 不是目录
 *            1 - 是目录
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2025/06/06	     V1.0	  baibu	      创建
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


/**
 * @brief  判断一个目录是否常规的目录,在本程序中把sbin等目录当作特殊目录来对待
 * 
 * @param  strDirPath    - 目录的路径
 * @param  strSubDirName - 目录的名字
 * @return int  0 - 不是常规目录
 *            1 - 是常规目录
 * @note   当父目录是根目录的时候进行检查，用于跳过系统目录，可以防止程序误操作关键的系统目录
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 1.0
 */
static int isRegDir(char *strDirPath, char *strSubDirName)
{
    static const char *astrSpecailDirs[] = {"sbin", "bin", "usr", "lib", "proc", "tmp", "dev", "sys", NULL};
    int i = 0;
    
    /* 如果目录名含有"astrSpecailDirs"中的任意一个, 则返回0 */
    if (0 == strcmp(strDirPath, "/"))
    {
        while (astrSpecailDirs[i])
        {
            if (0 == strcmp(strSubDirName, astrSpecailDirs[i]))
                return 0;
            i++;
        }
    }
    return 1;    
}

/**
 * @brief  判断一个文件是否常规的文件,设备节点/链接文件/FIFO文件等是特殊文件
 * 
 * @param  strFilePath - 文件的路径
 * @param  strFileName - 文件的名字
 * @return int  0 - 不是常规文件
 *            1 - 是常规文件
 * 
 * @author  bia布
 * @date    2025/06/6
 * @version 1.0
 */
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
 * @brief  把某目录下所含的顶层子目录、顶层文件都记录下来,并且按名字排序
 * 
 * @param  strDirName - 目录名(含绝对路径)
 * @param  pptDirContents - (*pptDirContents)指向一个PT_DirContent数组,
 *                             (*pptDirContents)[0,1,...]指向T_DirContent结构体,
 *                             T_DirContent中含有"目录/文件"的名字等信息
 * @param  piNumber       - strDirName下含有多少个"顶层子目录/顶层文件",
 *                             即数组(*pptDirContents)[0,1,...]有多少项
 * @return    0 - 成功
 *            1 - 失败
 * 
 * @note   调用scandir函数获取当前目录下的所有文件，然后isDir等判断是否是目录或者文件，
 *         将结果和文件名称存储在pptDirContents中
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

   
    /* 1. 扫描目录
     * 该函数扫描目录下的所有子目录和文件，存在aptNameList[0],aptNameList[1],... 
     * alphasort：表示按字母顺序排列
     * */
    iNum = scandir(strDirName, &aptNameList, 0, alphasort);
    if(iNum < 0){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>scandir error : %s!\n", strDirName);
        return -1;
    }

    // DBG_PRINTF(APP_INFO, "scandir : %s\n", strDirName);
    // DBG_PRINTF(APP_INFO, "scandir number : %d\n", iNum);
    // for(int i = 0; i < iNum; i++){
    //     DBG_PRINTF("aptNameList[i]->d_name:%s\n",aptNameList[i]->d_name);
    // }
    

    /* 2. 分配空间
     * scandir 的扫描结果默认会包含当前目录（.）和父目录（..）两项，去掉这两项，生成iNum-2个空间
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
            // 资源回收
            while(i-- > 0) free(aptDirContents[i]);
            free(aptDirContents);
            DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            DBG_PRINTF("<3>malloc error!\n");
            return -1;
        }
    }

    /* 3. 优先取出目录放入到ptDirContents */
    //DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    for(i = 0, j = 0; i < iNum; i++){
        // 不处理目录.和..
        if(0 == strcmp(aptNameList[i]->d_name, ".") || (0 == strcmp(aptNameList[i]->d_name, ".."))){
            continue;
        }
        if(isDir(strDirName, aptNameList[i]->d_name)){
            strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, 256);
            aptDirContents[j]->strName[255] = '\0';
            aptDirContents[j]->eFileType = FILETYPE_DIR;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
            j++;
        }
    }

    /* 4. 处理常规文件放入到ptDirContents */ 
    for(int i = 0; i < iNum; i++){
        if(aptNameList[i] == NULL){
            continue;
        }
        // 不处理目录.和..
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

    /* 5. 释放未使用的aptDirContents项：有写文件与目录可能不是常规的文件和目录，统计出来了iNum但并不放入aptDirContents中*/
    for(i = j; i < iNum - 2; i++){
        free(aptDirContents[i]);
    }

    /* 6. 释放aptNameList中还未释放的项：同5*/

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

/**
 * @brief  以深度优先的方式获得目录下的文件 
 *         即: 先获得顶层目录下的文件, 再进入一级子目录A
 *         再获得一级子目录A下的文件, 再进入二级子目录AA, ...
 *         处理完一级子目录A后, 再进入一级子目录B
 *
 * "连播模式"下调用该函数获得要显示的文件
 * 有两种方法获得这些文件:
 * 1. 事先把所有文件的名字保存到某个缓冲区中
 * 2. 用到时再去搜索取出若干个文件名
 * 第1种方法比较简单,但是当文件很多时有可能导致内存不足.
 * 我们使用第2种方法:
 * 
 * @param    strDirName            : 要获得哪个目录下的内容 
 *           piStartNumberToRecord : 从第几个文件开始取出它们的名字
 *           iFileCountTotal       : 总共要取出多少个文件的名字
 *           piFileCountHaveGet    : 已经得到了多少个文件的名字
 *           apstrFileNames[][256] : 用来存储搜索到的文件名
 *           piCurFileNumber       : 当前搜索到的文件编号
 * @return 0 - 成功
 *           1 - 失败
 * @note    一个递归遍历目录并收集文件路径的功能，它从指定目录开始，按顺序记录文件路径到提供的数组中，
 *          支持从指定位置开始记录、限制收集总数，并能递归处理子目录，使用静态变量跟踪目录深度防止栈溢出
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 1.0
 */
int GetFilesIndir(char *strDirName, int *piStartNumberToRecord, int *piCurFileNumber, int *piFileCountHaveGet, int iFileCountTotal, char apstrFileNames[][256]){
    
    PT_DirContent *aptDirContents;  /* 数组:存有目录下"顶层子目录","文件"的名字 */
    
    static int iDirDeepness = 0;
    int iDirCountNumber;
    int iError;
    char strSubDirName[256];

    if (iDirDeepness > MAX_DIR_DEEPNESS){
        return -1;
    }

    iDirDeepness++; 

    iError = GetDirContents(strDirName, &aptDirContents, &iDirCountNumber);
    if (iError){
        DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        DBG_PRINTF("<3>GetDirContents error!\n");
        iDirDeepness--;
        return -1;
    }

    for(int i = 0; i < iDirCountNumber; i++){
        if(aptDirContents[i]->eFileType == FILETYPE_FILE){
            if(*piCurFileNumber >= *piStartNumberToRecord){
                snprintf(apstrFileNames[*piFileCountHaveGet], 256, "%s/%s", strDirName, aptDirContents[i]->strName);
                (*piFileCountHaveGet)++;
                (*piCurFileNumber)++;
                (*piStartNumberToRecord)++;
                // 得到文件名后，判断是否已经达到文件总数，是则返回
                if(*piFileCountHaveGet >= iFileCountTotal){
                    FreeDirContents(aptDirContents, iDirCountNumber);
                    iDirDeepness--;
                    return 0;
                }
            }else{
                (*piCurFileNumber)++;
            }
        }
    }

    for(int i = 0; i < iDirCountNumber; i++){
        if((aptDirContents[i]->eFileType == FILETYPE_DIR) && isRegDir(strDirName, aptDirContents[i]->strName)){
            snprintf(strSubDirName, 256, "%s/%s", strDirName, aptDirContents[i]->strName);
            GetFilesIndir(strSubDirName, piStartNumberToRecord, piCurFileNumber, piFileCountHaveGet, iFileCountTotal, apstrFileNames);
            if(*piFileCountHaveGet >= iFileCountTotal){
                FreeDirContents(aptDirContents, iDirCountNumber);
                iDirDeepness--;
                return 0;
            }
        }
    }

    FreeDirContents(aptDirContents, iDirCountNumber);
    iDirDeepness--;
    return 0;
}

