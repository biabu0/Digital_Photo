#include <config.h>
#include <page_manager.h>
#include <string.h>
#include <stdlib.h>
#include <render.h>

static PT_PageAction g_ptPageActionHead;	//�ṹ������ͷ��

/*		ע��ṹ��			���ṹ����뵽������ȡ*/
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


/* �������ֻ�ȡҳ��ṹ�� */
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
 * @brief  ��ͼ���ļ��н�����ͼ�����ݲ�����ָ������,�Ӷ�����ҳ������
 * 
 * @param   ptPageLayout - �ں����ͼ����ļ�������ʾ����
 * @param   ptVideoMem   - �����VideoMem�ﹹ��ҳ������
 * 
 * @return int   0       - �ɹ�
 *               ����ֵ   - ʧ��
 * 
 * @note   ��BMP�ļ�����ȡ������Ϣ���������ŵ�ȷ���Ĵ�С�����������ݺϲ����ڴ����
 * 
 */

int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem){

	T_PixelDatas tIconPixelDatas;
	T_PixelDatas tOriginIconPixelDatas;
	PT_Layout atLayout = ptPageLayout->atLayout;
	int iError;

	// ҳ������û������
    if(ptVideoMem->ePicDataState != PDS_GENERATED){
		// ������VideoMem
		ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

        tIconPixelDatas.iBpp = ptPageLayout->iBpp;		
        tIconPixelDatas.aucPixelDatas = malloc(tIconPixelDatas.iTotalBytes);
        if(tIconPixelDatas.aucPixelDatas == NULL){
            DBG_PRINTF("<3>tIconPixelDatas.aucPixelDatas malloc error!\n");
            return -1;
        }
        while(atLayout->strIconName){

            // ��BMP�ļ��л��ԭʼ��������
            iError = GetPixelDatasFrmBMP(atLayout->strIconName, &tOriginIconPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>GetPixelDatasFrmBMP error!\n");
                return -1;
            }

 			tIconPixelDatas.iHeight = atLayout->iBotRightY - atLayout->iTopLeftY + 1;
			tIconPixelDatas.iWidth  = atLayout->iBotRightX - atLayout->iTopLeftX+ 1;
			tIconPixelDatas.iLineBytes  = tIconPixelDatas.iWidth * tIconPixelDatas.iBpp / 8;
			tIconPixelDatas.iTotalBytes = tIconPixelDatas.iLineBytes * tIconPixelDatas.iHeight;

            // ��������������һ��           
            iError = PicZoom(&tOriginIconPixelDatas, &tIconPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>PicZoom error!\n");
                return -1;
            }
            // ���������ݺϲ����ڴ����
            iError = PicMerge(atLayout->iTopLeftX, atLayout->iTopLeftY, &tIconPixelDatas, &ptVideoMem->tPixelDatas);
            if(iError != 0){
                DBG_PRINTF("<3>PicMerge error!\n");
                return -1;
            }
            // �����ڴ�״̬
            FreePixelDatasForIcon(&tOriginIconPixelDatas);
            atLayout++;
        }
        free(tIconPixelDatas.aucPixelDatas);
        // �����������
        ptVideoMem->ePicDataState = PDS_GENERATED;
    }

	return 0;
}


/**
 * @brief  ��ȡ��������,���ж���λ����һ��ͼ����
 * 
 * @param  ptPageLayout - �ں����ͼ�����ʾ���򣬶����򲼾�
 * @param  ptInputEvent - �ں��õ�����������
 * @return int   -1     - �������ݲ�λ���κ�һ��ͼ��֮��
 *            ����ֵ - �������������ڵ�ͼ��(PageLayout->atLayout�������һ��)
 * @author  biabu
 * @date    2025/06-06
 * @version 1.0
 */
int GenericPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent){

    T_InputEvent tInputEvent;
    int iRet;
    int i = 0;
	PT_Layout aptLayout = ptPageLayout->atLayout;

    // ��ȡԭʼ����������:����input_manager.c�к�����ʹ�õ�ǰ�̴߳�������״̬���д��������ݵ�����ʱ���Ѹ��߳�
    // ����ȡ���Ĵ��������ݴ�ŵ�tInputEvent�У������������Ĵ�����λ���Ѿ�ѹ��״̬��
    iRet = GetInputEvent(&tInputEvent);
    if(iRet != 0){
        DBG_PRINTF("<3>GetInputEvent error!\n");
        return -1;
    }
	// ���Ǵ������������Ȳ����д���
    if(tInputEvent.iType != INPUT_TYPE_TOUCHSCREEN){
        //DBG_PRINTF("<3>tInputEvent.iType is not INPUT_TYPE_TOUCHSCREEN!\n");
        return -1;
    }

    // ���õ��������¼����أ����۸��¼��Ƿ��ǵ���˰�ť��Ҫ���з��أ�������ǵ���˰�ť�������Ƕ�ͼƬ��С�������ŵȲ���
    *ptInputEvent = tInputEvent;

    // ��������
    // 1.ȷ������λ����һ����ť��
	DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    while(aptLayout[i].strIconName){
		printf("aptLayout[i].strIconName: %s\n", aptLayout[i].strIconName);
        if((tInputEvent.iX >= aptLayout[i].iTopLeftX) && (tInputEvent.iX <= aptLayout[i].iBotRightX) \
            && (tInputEvent.iY >= aptLayout[i].iTopLeftY) && (tInputEvent.iY <= aptLayout[i].iBotRightY)){
            // �ҵ������еİ�ť�����ذ�ť���±�
            return i;
        }else{
            i++;
        }
    }
	DBG_PRINTF("<6>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

    // ���λ�ò�λ�ڰ�ť�ڲ�
    return -1;
}

int PagesInit(void){
	int iError;

	iError = MainPageInit();
	iError = BrowsePageInit();
	iError = ExplorePageInit();
	return iError;
}



