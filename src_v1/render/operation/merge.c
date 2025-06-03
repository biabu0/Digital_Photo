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