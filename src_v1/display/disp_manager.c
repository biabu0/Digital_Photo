
#include <config.h>
#include <disp_manager.h>
#include <string.h>
#include <stdlib.h>



static PT_DispOpr g_ptDispOprHead;	//结构体链表头部
static PT_DispOpr g_ptDefaultDispOpr;
static PT_VideoMem g_ptVideoMemHead;

/*		注册结构体			将结构体加入到链表中取*/
int RegisterDispOpr(PT_DispOpr ptDispOpr){

	PT_DispOpr ptTmp;
	
	if(!g_ptDispOprHead){
		g_ptDispOprHead = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}else{
		ptTmp = g_ptDispOprHead;
		while(ptTmp->ptNext){
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}
	
	return 0;
}

void ShowDispOpr(void){

	PT_DispOpr ptTmp = g_ptDispOprHead;
	int i = 0;
	
	while(ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_DispOpr GetDispOpr(char *pcName){

	PT_DispOpr ptTmp = g_ptDispOprHead;

	while(ptTmp){
		if(strcmp(ptTmp->name, pcName) == 0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}

void SelectAndInitDefaultDispDev(char *name){
	g_ptDefaultDispOpr = GetDispOpr(name);
	if (g_ptDefaultDispOpr)
	{
		g_ptDefaultDispOpr->DeviceInit();
		g_ptDefaultDispOpr->CleanScreen(0);
	}
}

// 不建议直接使用全局变量，通过函数获得比较好，防止命名冲突
PT_DispOpr GetDefaultDispDev(void){
	return g_ptDefaultDispOpr;
}

int GetDispResolution(int *piXres, int *piYres, int *piBpp){
	if (g_ptDefaultDispOpr)
	{
		*piXres = g_ptDefaultDispOpr->iXres;
		*piYres = g_ptDefaultDispOpr->iYres;
		*piBpp  = g_ptDefaultDispOpr->iBpp;
		return 0;
	}
	else
	{
		return -1;
	}
}

// 分配iNum块内存
int AllocVideoMem(int iNum){
	
	int iXres = 0;
	int iYres = 0;
	int iBpp = 0;
	int iVideoMemSize;
	int iLineBytes;

	PT_VideoMem ptNew;

	// 获取显示器的分辨率和BPP，得到显存的大小
	GetDispResolution(&iXres, &iYres, &iBpp);
	iVideoMemSize = iXres * iYres * iBpp / 8;
	iLineBytes = iXres * iBpp / 8;
	

	// 先把设备本身的framebuffer放入到链表中；framebuffer在驱动程序中已经分配好了，只需要定义一个结构体来描述就可以
	// 如果iNum为0也就是直接将数据存放到该内存中
	ptNew = malloc(sizeof(T_VideoMem));
	if(ptNew == NULL){
		return -1;
		DBG_PRINTF("PT_VideoMem malloc error!\n");
	}
	// 设备显存
	ptNew->tPixelDatas.aucPixelDatas = g_ptDefaultDispOpr->pucDispMem;
	ptNew->iID = 0;
	ptNew->ePicDataState = PDS_BLANK;
	ptNew->eVideoMemState = VMS_FREE;
	ptNew->iDevFrameBuffer = 1;							//是设备的framebuffer
	ptNew->tPixelDatas.iBpp = iBpp;
	ptNew->tPixelDatas.iHeight = iYres;
	ptNew->tPixelDatas.iWidth = iXres;
	ptNew->tPixelDatas.iLineBytes = iLineBytes;
	ptNew->tPixelDatas.iTotalBytes = iVideoMemSize;

	if(iNum != 0){
		// 将显存的内存状态初始化为VMS_USED_FOR_CUR，避免GetVideoMem的时候取到
		ptNew->eVideoMemState = VMS_USED_FOR_CUR;
	}

	// 初始化结束，放入链表头
	// ptNew->ptNext = g_ptVideoMemHead;
	// g_ptVideoMemHead->ptNext = ptNew;
	if (!g_ptVideoMemHead) {
		g_ptVideoMemHead = ptNew;
		ptNew->ptNext = NULL;
	} else {
		ptNew->ptNext = g_ptVideoMemHead;
		g_ptVideoMemHead = ptNew;  // 更新头指针
	}
	for(int i = 0; i < iNum; i++){
		// T_VideoMem中的T_PixelDatas的aucPixelDatas指向内存iVideoMemSize
		ptNew = malloc(sizeof(T_VideoMem) + iVideoMemSize);
		if(ptNew == NULL){
			return -1;
			DBG_PRINTF("PT_VideoMem malloc error!\n");
		}
		//ptNew里面的显存指向内存iVideoMemSize
		ptNew->tPixelDatas.aucPixelDatas = (unsigned char *)(ptNew + 1);
		ptNew->iID = 0;
		ptNew->ePicDataState = PDS_BLANK;
		ptNew->eVideoMemState = VMS_FREE;
		ptNew->iDevFrameBuffer = 0;
		ptNew->tPixelDatas.iBpp = iBpp;
		ptNew->tPixelDatas.iHeight = iYres;
		ptNew->tPixelDatas.iWidth = iXres;
		ptNew->tPixelDatas.iLineBytes = iLineBytes;
		ptNew->tPixelDatas.iTotalBytes = iVideoMemSize;

		// // 初始化结束，放入链表头
		// ptNew->ptNext = g_ptVideoMemHead;
		// g_ptVideoMemHead->ptNext = ptNew;

		ptNew->ptNext = NULL;  // 确保新节点next为NULL:ml-citation{ref="3" data="citationList"}
		if(!g_ptVideoMemHead){
			g_ptVideoMemHead = ptNew;  // 空链表时直接作为头节点:ml-citation{ref="4" data="citationList"}
		}else{
			PT_VideoMem ptTmp = g_ptVideoMemHead;
			while(ptTmp->ptNext){  // 找到当前尾节点:ml-citation{ref="1" data="citationList"}
				ptTmp = ptTmp->ptNext;
			}
			ptTmp->ptNext = ptNew;  // 追加到链表尾部:ml-citation{ref="1" data="citationList"}
		}
	}

	return 0;
}


void PutVideoMem(PT_VideoMem ptVideoMem){
	ptVideoMem->eVideoMemState = VMS_FREE;
}

// 根据ID值从链表中将响应内存块取出
PT_VideoMem GetVideoMem(int iID, int iCur){

	PT_VideoMem ptTmp;

	// 1.优先取出空闲的、ID相同的videomem
	ptTmp = g_ptVideoMemHead;
	while(ptTmp){
		if((ptTmp->iID == iID) && (ptTmp->eVideoMemState == VMS_FREE)){
			ptTmp->eVideoMemState = iCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	// 2.如果没有ID相同的，说明当前id还没有进行分配，优先取出空闲的
	ptTmp = g_ptVideoMemHead;
	while(ptTmp){
		if(ptTmp->eVideoMemState == VMS_FREE){
			ptTmp->iID = iID;
			ptTmp->ePicDataState = PDS_BLANK;			// 当前数据还没写入
			ptTmp->eVideoMemState = iCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

	return NULL;
}


int DisplayInit(void){
	int iError;

	iError = FBInit();

	return iError;
}