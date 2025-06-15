#include<stdlib.h>
#include<stdio.h>
#include<pic_operation.h>
#include<string.h>

// 将小图片合并到大图片中  /*iX，iY为小图片在大图片中的位置*/
int PicMerge(int iX, int iY, PT_PixelDatas ptSmallPic, PT_PixelDatas ptBigPic){
    
    int i;
    unsigned char *pucSrc;
    unsigned char *pucDst;

    if((ptSmallPic->iWidth > ptBigPic->iWidth) ||
        (ptSmallPic->iHeight > ptBigPic->iHeight) ||
        (ptSmallPic->iBpp != ptBigPic->iBpp))
    {
        return -1;
    }

    pucSrc = ptSmallPic->aucPixelDatas;
    pucDst = ptBigPic->aucPixelDatas + iY * ptBigPic->iLineBytes + iX * ptBigPic->iBpp / 8;
    for(i = 0; i < ptSmallPic->iHeight; i++){
        memcpy(pucDst, pucSrc, ptSmallPic->iLineBytes);
        pucDst += ptBigPic->iLineBytes;
        pucSrc += ptSmallPic->iLineBytes;
    }

    return 0;
}


/**
 * @brief  把新图片的某部分, 合并入老图片的指定区域
 * 
 * @param  iStartXofNewPic, iStartYofNewPic : 从新图片的(iStartXofNewPic, iStartYofNewPic)座标处开始读出数据用于合并
 * @param  iStartXofOldPic, iStartYofOldPic : 合并到老图片的(iStartXofOldPic, iStartYofOldPic)座标去
 * @param  iWidth, iHeight                  : 合并区域的大小
 * @param  ptNewPic                         : 新图片
 * @param  ptOldPic                         : 老图片
 * @return 0 - 成功, 其他值 - 失败
 * 
 * @author  bia布
 * @date    2025/06/15
 * @version 1.0
 */
int PicMergeRegion(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int iWidth, int iHeight, PT_PixelDatas ptNewPic, PT_PixelDatas ptOldPic)
{
	int i;
	unsigned char *pucSrc;
	unsigned char *pucDst;
    int iLineBytesCpy = iWidth * ptNewPic->iBpp / 8;

    if ((iStartXofNewPic < 0 || iStartXofNewPic >= ptNewPic->iWidth) || \
        (iStartYofNewPic < 0 || iStartYofNewPic >= ptNewPic->iHeight) || \
        (iStartXofOldPic < 0 || iStartXofOldPic >= ptOldPic->iWidth) || \
        (iStartYofOldPic < 0 || iStartYofOldPic >= ptOldPic->iHeight))
    {
        return -1;
    }
	
	pucSrc = ptNewPic->aucPixelDatas + iStartYofNewPic * ptNewPic->iLineBytes + iStartXofNewPic * ptNewPic->iBpp / 8;
	pucDst = ptOldPic->aucPixelDatas + iStartYofOldPic * ptOldPic->iLineBytes + iStartXofOldPic * ptOldPic->iBpp / 8;
	for (i = 0; i < iHeight; i++)
	{
		memcpy(pucDst, pucSrc, iLineBytesCpy);
		pucSrc += ptNewPic->iLineBytes;
		pucDst += ptOldPic->iLineBytes;
	}
	return 0;
}