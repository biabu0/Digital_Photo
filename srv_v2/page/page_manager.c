#include <config.h>
#include <page_manager.h>
#include <string.h>
#include <stdlib.h>
#include <render.h>

static PT_PageAction g_ptPageActionHead;	//结构体链表头部

/*		注册结构体			将结构体加入到链表中取*/
int RegisterPageAction(PT_PageAction ptPageAction){

	PT_PageAction ptTmp;
	
	if(!g_ptPageActionHead){
		g_ptPageActionHead = ptPageAction;
		ptPageAction->ptNext = NULL;
	}else{
		ptTmp = g_ptPageActionHead;
		while(ptTmp->ptNext){
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptPageAction;
		ptPageAction->ptNext = NULL;
	}
	
	return 0;
}

void ShowPages(void){

	PT_PageAction ptTmp = g_ptPageActionHead;
	int i = 0;
	
	while(ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


/* 根据名字获取页面结构体 */
PT_PageAction Page(char *pcName){
	PT_PageAction ptTmp = g_ptPageActionHead;
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	while(ptTmp){
		if(strcmp(ptTmp->name, pcName) == 0){
			DBG_PRINTF("<3>ptTmp.name: %s", ptTmp->name);
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	return NULL;
}
int ID(char *strName){
	return (int)strName[0] + (int)strName[1] + (int)strName[2] + (int)strName[3];
}


void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor){
	unsigned char *pucVM;
	unsigned short *pwVM16bpp;
	unsigned int *pdwVM32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int i = 0;

	pucVM = ptVideoMem->tPixelDatas.aucPixelDatas;
	pwVM16bpp  = (unsigned short *)pucVM;
	pdwVM32bpp = (unsigned int *)pucVM;

	switch (ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
			memset(pucVM, dwColor, ptVideoMem->tPixelDatas.iTotalBytes);
			break;
		}
		case 16:
		{
			/* 565 */
			iRed   	 = (dwColor >> 16) & 0xff;
			iGreen 	 = (dwColor >> 8) & 0xff;
			iBlue  	 = (dwColor >> 0) & 0xff;
			wColor16bpp  = ((iRed >> 3) << 11) | ((iGreen >> 2) << 5) | (iBlue >> 3);

			while(i < ptVideoMem->tPixelDatas.iTotalBytes){
				*pwVM16bpp = wColor16bpp;
				pwVM16bpp++;
				i += 2;
			}
			break;
		}
		case 32:
		{
			while(i < ptVideoMem->tPixelDatas.iTotalBytes){
				*pdwVM32bpp = dwColor;
				pdwVM32bpp++;
				i += 4;
			}
			break;
		}
		default:
		{
			DBG_PRINTF("can't surport %d bpp\n", ptVideoMem->tPixelDatas.iBpp);
			break;
		}
	}
}

/**
 * @brief  从图标文件中解析出图像数据并放在指定区域,从而生成页面数据
 * 
 * @param   ptPageLayout - 内含多个图标的文件名和显示区域
 * @param   ptVideoMem   - 在这个VideoMem里构造页面数据
 * 
 * @return int   0       - 成功
 *               其他值   - 失败
 * 
 * @note   从BMP文件中提取数据信息，将其缩放到确定的大小，将像素数据合并到内存块中
 * 
 */

int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem){

	T_PixelDatas tIconPixelDatas;
	T_PixelDatas tOriginIconPixelDatas;
	PT_Layout atLayout = ptPageLayout->atLayout;
	int iError;

	// 页面数据没有生成
    if(ptVideoMem->ePicDataState != PDS_GENERATED){
		// 先清理VideoMem
		ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

        tIconPixelDatas.iBpp = ptPageLayout->iBpp;		
        tIconPixelDatas.aucPixelDatas = malloc(tIconPixelDatas.iTotalBytes);
        if(tIconPixelDatas.aucPixelDatas == NULL){
            DBG_PRINTF("<3>tIconPixelDatas.aucPixelDatas malloc error!\n");
            return -1;
        }
        while(atLayout->strIconName){

            // 从BMP文件中获得原始像素数据
            iError = GetPixelDatasFrmBMP(atLayout->strIconName, &tOriginIconPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>GetPixelDatasFrmBMP error!\n");
                return -1;
            }

 			tIconPixelDatas.iHeight = atLayout->iBotRightY - atLayout->iTopLeftY + 1;
			tIconPixelDatas.iWidth  = atLayout->iBotRightX - atLayout->iTopLeftX+ 1;
			tIconPixelDatas.iLineBytes  = tIconPixelDatas.iWidth * tIconPixelDatas.iBpp / 8;
			tIconPixelDatas.iTotalBytes = tIconPixelDatas.iLineBytes * tIconPixelDatas.iHeight;

            // 将像素数据缩放一下           
            iError = PicZoom(&tOriginIconPixelDatas, &tIconPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>PicZoom error!\n");
                return -1;
            }
            // 将像素数据合并到内存块中
            iError = PicMerge(atLayout->iTopLeftX, atLayout->iTopLeftY, &tIconPixelDatas, &ptVideoMem->tPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>PicMerge error!\n");
                return -1;
            }
            // 更新内存状态
            FreePixelDatasForIcon(&tOriginIconPixelDatas);
            atLayout++;
        }
        free(tIconPixelDatas.aucPixelDatas);
        // 数据描述完成
        ptVideoMem->ePicDataState = PDS_GENERATED;
    }

	return 0;
}


/**
 * @brief  读取输入数据,并判断它位于哪一个图标上
 * 
 * @param  ptPageLayout - 内含多个图标的显示区域，多区域布局
 * @param  ptInputEvent - 内含得到的输入数据
 * @return int   -1     - 输入数据不位于任何一个图标之上
 *            其他值 - 输入数据所落在的图标(PageLayout->atLayout数组的哪一项)
 * @author  biabu
 * @date    2025/06-06
 * @version 1.0
 */
int GenericPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent){

    T_InputEvent tInputEvent;
    int iRet;
    int i = 0;
	PT_Layout aptLayout = ptPageLayout->atLayout;

    // 获取原始触摸屏数据:调用input_manager.c中函数，使得当前线程处于休眠状态，有触摸屏数据到来的时候唤醒该线程
    // 将获取到的触摸屏数据存放到tInputEvent中，包含触摸屏的触摸点位置已经压力状态等
    iRet = GetInputEvent(&tInputEvent);
    if(iRet != 0){
        DBG_PRINTF("<3>GetInputEvent error!\n");
        return -1;
    }
	// 不是触摸屏的数据先不进行处理
    if(tInputEvent.iType != INPUT_TYPE_TOUCHSCREEN){
        //DBG_PRINTF("<3>tInputEvent.iType is not INPUT_TYPE_TOUCHSCREEN!\n");
        return -1;
    }

    // 将得到的输入事件返回，无论该事件是否是点击了按钮都要进行返回，如果不是点击了按钮，可能是对图片大小进行缩放等操作
    *ptInputEvent = tInputEvent;

    // 处理数据
    // 1.确定触点位于哪一个按钮上
	DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    while(aptLayout[i].strIconName){
		printf("aptLayout[i].strIconName: %s\n", aptLayout[i].strIconName);
        if((tInputEvent.iX >= aptLayout[i].iTopLeftX) && (tInputEvent.iX <= aptLayout[i].iBotRightX) \
            && (tInputEvent.iY >= aptLayout[i].iTopLeftY) && (tInputEvent.iY <= aptLayout[i].iBotRightY)){
            // 找到被点中的按钮，返回按钮的下标
            return i;
        }else{
            i++;
        }
    }
	DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

    // 点击位置不位于按钮内部
    return -1;
}

int PagesInit(void){
	int iError;

	iError = MainPageInit();
	iError = BrowsePageInit();
	iError = ExplorePageInit();
	return iError;
}



