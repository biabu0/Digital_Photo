#include <config.h>
#include <render.h>
#include <stdlib.h>
#include <file.h>
#include <string.h>
#include <unistd.h>

static T_PageCfg g_tPageCfg;
static pthread_t g_tAutoPlayThreadID;
static pthread_mutex_t g_tAutoPlayThreadMutex  = PTHREAD_MUTEX_INITIALIZER; /* 互斥量 */
static int g_bAutoPlayThreadShouldExit = 0;

/* 以深度优先的方式获得目录下的文件 
 * 即: 先获得顶层目录下的文件, 再进入一级子目录A
 *     先获得一级子目录A下的文件, 再进入二级子目录AA, ...
 *     处理完一级子目录A后, 再进入一级子目录B
 *
 * "连播模式"下调用该函数获得要显示的文件
 * 有两种方法获得这些文件:
 * 1. 事先只需要调用一次函数,把所有文件的名字保存到某个缓冲区中
 * 2. 要使用文件时再调用函数,只保存当前要使用的文件的名字
 * 第1种方法比较简单,但是当文件很多时有可能导致内存不足.
 * 我们使用第2种方法:
 * 假设某目录(包括所有子目录)下所有的文件都给它编一个号
 * g_iStartNumberToRecord : 从第几个文件开始取出它们的名字
 * g_iCurFileNumber       : 本次函数执行时读到的第1个文件的编号
 * g_iFileCountHaveGet    : 已经得到了多少个文件的名字
 * g_iFileCountTotal      : 每一次总共要取出多少个文件的名字
 * g_iNextProcessFileIndex: 在g_apstrFileNames数组中即将要显示在LCD上的文件
 *
 */
static int g_iStartNumberToRecord = 0;
static int g_iCurFileNumber = 0;
static int g_iFileCountHaveGet = 0;
static int g_iFileCountTotal = 0;
static int g_iNextProcessFileIndex = 0;


// 自动播放模式下最多显示10张图片
#define FILE_COUNT 10
static char g_apstrFileNames[FILE_COUNT][256];


/**
 * @brief  每次使用"连播"功能时,都调用此函数,它使得从第1个文件开始"连播"
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 1.0
 */
static void ResetAutoPlayFile(void)
{
    g_iStartNumberToRecord = 0;
    g_iCurFileNumber = 0;
    g_iFileCountHaveGet = 0;
    g_iFileCountTotal = 0;
    g_iNextProcessFileIndex = 0;
}

/**
 * @brief  获得下一个要播放的图片的路径
 * 
 * @param  strFileName - 里面存有下一个要播放的图片的名字(含绝对路径)
 * @return int   0 - 成功, 其他值 - 失败
 * @note   一个递归遍历目录并收集文件路径的功能，它从指定目录开始，按顺序记录文件路径到提供的数组中，
 * 			支持从指定位置开始记录、限制收集总数，并能递归处理子目录，使用静态变量跟踪目录深度防止栈溢出。
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 1.0
 */
static int GetNextAutoPlayFile(char *strFileName){
	int iError;
	if(g_iNextProcessFileIndex < g_iFileCountHaveGet){
		strncpy(strFileName, g_apstrFileNames[g_iNextProcessFileIndex], 256);
        g_iNextProcessFileIndex++;
		return 0;
	}else{
		g_iCurFileNumber = 0;
		g_iFileCountHaveGet = 0;
		g_iFileCountTotal = FILE_COUNT;
		g_iNextProcessFileIndex = 0;

		iError = GetFilesIndir(g_tPageCfg.strSeletedDir, &g_iStartNumberToRecord, &g_iCurFileNumber, &g_iFileCountHaveGet, g_iFileCountTotal, g_apstrFileNames);

		if (iError || (g_iNextProcessFileIndex >= g_iFileCountHaveGet))
        {
            /* 再次从头读起(连播模式下循环显示) */
            g_iStartNumberToRecord = 0;
            g_iCurFileNumber    = 0;
            g_iFileCountHaveGet = 0;
            g_iFileCountTotal = FILE_COUNT;
            g_iNextProcessFileIndex = 0;
            
            iError = GetFilesIndir(g_tPageCfg.strSeletedDir, &g_iStartNumberToRecord, &g_iCurFileNumber, &g_iFileCountHaveGet, g_iFileCountTotal, g_apstrFileNames);
        }
		if (iError == 0)
        {   
            if (g_iNextProcessFileIndex < g_iFileCountHaveGet)
            {
                strncpy(strFileName, g_apstrFileNames[g_iNextProcessFileIndex], 256);
                g_iNextProcessFileIndex++;
                return 0;
            }
        }

	}
	return -1;
}

/**
 * @brief  准备显示下一图片: 取出下图片的数据,存入VideoMem中
 * 
 * @param  Cur : 0 - 表示这是做准备用的, 有可能无法获得videomem
 *                   1 - 表示必须获得videomem, 因为这是马上就要在LCD上显示出来的
 * @return PT_VideoMem
 * @note   通过获取下一张图片文件、调整图片尺寸并居中显示在内存中
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 1.0
 */
static PT_VideoMem PrepareNextPicture(int bCur){
	int iXres, iYres, iBpp;
	int iError;
	int iTopLeftX, iTopLeftY;
    float k;

	char strFileName[256];
	T_PixelDatas tOriginIconPixelDatas;
	T_PixelDatas tPicPixelDatas;
	PT_VideoMem ptVideoMem;

	GetDispResolution(&iXres, &iYres, &iBpp);
    ptVideoMem = GetVideoMem(-1, bCur);
	if (ptVideoMem == NULL)
	{
		DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		DBG_PRINTF("<3>Can't get video mem for browse page!\n");
		return NULL;
	}
	//ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);
	ClearRectangleInVideoMem(0, 0, iXres, iYres, ptVideoMem, COLOR_BACKGROUND);

	while(1){
		iError = GetNextAutoPlayFile(strFileName);
		if (iError)
        {
			DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            DBG_PRINTF("<3>GetNextAutoPlayFile error\n");
            PutVideoMem(ptVideoMem);
            return NULL;
        }
		DBG_PRINTF("<6>strFileName: %s\n", strFileName);
		// 获取图片数据，结束循环
		iError = GetPixelDatasFrmFile(strFileName, &tOriginIconPixelDatas);
        if (0 == iError)
        {
            break;
        }
	}

	k = (float)tOriginIconPixelDatas.iHeight / tOriginIconPixelDatas.iWidth;
    tPicPixelDatas.iWidth  = iXres;
    tPicPixelDatas.iHeight = iXres * k;
    if (tPicPixelDatas.iHeight > iYres)
    {
        tPicPixelDatas.iWidth  = iYres / k;
        tPicPixelDatas.iHeight = iYres;
    }
    tPicPixelDatas.iBpp        = iBpp;
    tPicPixelDatas.iLineBytes  = tPicPixelDatas.iWidth * tPicPixelDatas.iBpp / 8;
    tPicPixelDatas.iTotalBytes = tPicPixelDatas.iLineBytes * tPicPixelDatas.iHeight;
    tPicPixelDatas.aucPixelDatas = malloc(tPicPixelDatas.iTotalBytes);
	if (tPicPixelDatas.aucPixelDatas == NULL)
    {
        PutVideoMem(ptVideoMem);
        return NULL;
    }

	PicZoom(&tOriginIconPixelDatas, &tPicPixelDatas);
	iTopLeftX = (iXres - tPicPixelDatas.iWidth) / 2;
    iTopLeftY = (iYres - tPicPixelDatas.iHeight) / 2;

    PicMerge(iTopLeftX, iTopLeftY, &tPicPixelDatas, &ptVideoMem->tPixelDatas);

	FreePixelDatasFrmFile(&tOriginIconPixelDatas);
	free(tPicPixelDatas.aucPixelDatas);

	return ptVideoMem;
}

/**
 * @brief  连播页面"的子线程函数:用于显示  (主线程用于读取输入数据)
 * 
 * @param  pVoid - 未用
 * @return void *
 * @note   获取要显示的图片文件，非首次执行时通过sleep实现固定间隔轮播，将图片数据刷新到显存上去进行显示。
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 1.0
 */
static void *AutoPlayThreadFunction(void *pVoid){
	int bExit;
	int bFirst = 1;

	PT_VideoMem ptVideoMem;
	ResetAutoPlayFile();
	while(1){

		// 1. 判断是否退出
		pthread_mutex_lock(&g_tAutoPlayThreadMutex);
		//使用bEixt可以将全局变量值拷贝到局部变量后立即释放锁，使临界区仅包含最简单的赋值操作
		bExit = g_bAutoPlayThreadShouldExit;
		pthread_mutex_unlock(&g_tAutoPlayThreadMutex);
		if(bExit){
			return NULL;
		}
		// 2. 准备要显示的图片
		ptVideoMem = PrepareNextPicture(0);
		// 3. 显示图片
		if(!bFirst){
			sleep(g_tPageCfg.iIntervalSecond);
		}
		bFirst = 0;
		if (ptVideoMem == NULL)
        {
            ptVideoMem = PrepareNextPicture(1);
        }
    	FlushVideoMemToDev(ptVideoMem);
    	PutVideoMem(ptVideoMem);  
	}
	return NULL;
}

// static void *AutoPlayThreadFunction(void *pVoid) {
//     pthread_mutex_lock(&g_tAutoPlayThreadMutex);
//     while(!g_bAutoPlayThreadShouldExit) {
//         // 使用条件变量替代sleep实现可中断等待
//         struct timespec ts;
//         clock_gettime(CLOCK_REALTIME, &ts);
//         ts.tv_sec += g_tPageCfg.iIntervalSecond;
//         pthread_cond_timedwait(&g_tAutoPlayCond, &g_tAutoPlayThreadMutex, &ts);
        
//         PT_VideoMem ptVideoMem = PrepareNextPicture(0);
//         if(!ptVideoMem) ptVideoMem = PrepareNextPicture(1);
        
//         if(ptVideoMem) {
//             FlushVideoMemToDev(ptVideoMem);
//             if(PutVideoMem(ptVideoMem) < 0) {
//                 pthread_mutex_unlock(&g_tAutoPlayThreadMutex);
//                 return NULL; // 显存操作失败时主动退出
//             }
//         }
//     }
//     pthread_mutex_unlock(&g_tAutoPlayThreadMutex);
//     return NULL;
// }



/**
 * @brief  "连播页面"的主线程函数: 用于读取输入数据
 * 
 * @param  ptParentPageParams - 内含上一个页面(父页面)的参数
 * @return void
 * @note   通过线程分离实现了UI响应与后台播放的解耦, 实现多线程图片轮播与触摸屏中断控制，包含路径解析、线程同步等核心机制
 * 
 * @author  bia布
 * @date    2025/06/20
 * @version 1.0
 */
static void AutoPageRun(PT_PageParams ptParentParams){
	
	int iRet;
	int bIconPressed = 0;
	T_InputEvent tInputEvent;
	char *pcTmp;
	// 每次重新调用的时候将其初始化为0
	g_bAutoPlayThreadShouldExit = 0;

	// 获取连播模式配置
	GetPageCfg(&g_tPageCfg);
	// 首先确定要显示的图片文件目录是否是默认的路径
	if(ptParentParams->strCurPicFile[0] != '\0'){
		strcpy(g_tPageCfg.strSeletedDir, ptParentParams->strCurPicFile);
		pcTmp = strrchr(g_tPageCfg.strSeletedDir, '/');
		if(pcTmp != g_tPageCfg.strSeletedDir){
			*pcTmp = '\0';
		}
	}

	// 1. 创建一个线程用于连续显示图片。
	pthread_create(&g_tAutoPlayThreadID, NULL, AutoPlayThreadFunction, NULL);
	sleep(1);
	// 2. 当前线程等待触摸屏数据，如果有触摸屏数据则退出
	while(1){
		iRet = GetInputEvent(&tInputEvent);
		if(iRet == 0 && tInputEvent.iType == INPUT_TYPE_TOUCHSCREEN){
			if(tInputEvent.iPressure == 0){
				if(bIconPressed){
					bIconPressed = 0;
					pthread_mutex_lock(&g_tAutoPlayThreadMutex);
					g_bAutoPlayThreadShouldExit = 1;   /* AutoPlayThreadFunction线程检测到这个变量为1后会退出 */
					pthread_mutex_unlock(&g_tAutoPlayThreadMutex);

					pthread_join(g_tAutoPlayThreadID, NULL);  //实现主线程对子线程g_tAutoPlayThreadID的同步等待与资源回收					
					return ;
				}
			}else{
				if(!bIconPressed){
					bIconPressed = 1;
				}
			}
		}
	}

}

static T_PageAction g_tAutoPageAction = {
	.name          = "auto",
	.Run           = AutoPageRun,
};



int AutoPageInit(void){
	return RegisterPageAction(&g_tAutoPageAction);
}


