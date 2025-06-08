#include<pic_operation.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

//缩放算法参考：http://blog.chinaunix.net/uid-22915173-id-2185545.html

int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic){
    unsigned long x;
    unsigned long y;
    unsigned long dwSrcY;
    unsigned long dwBytes = ptOriginPic->iBpp / 8;
    unsigned char *pucDest;
    unsigned char *pucSrc;
    unsigned long dwDestWidth = ptZoomPic->iWidth;
    unsigned long* pdwSrcXTable = malloc(sizeof(unsigned long) * dwDestWidth);

    if(ptOriginPic->iBpp != ptZoomPic->iBpp){
        printf("ptOriginPic->iBpp: %d\n",ptOriginPic->iBpp);
        printf("ptZoomPic->iBpp: %d\n",ptZoomPic->iBpp);
        return -1;
    }

    // 图片中每一列的x元素是一样的，先计算x元素存储起来，这样避免在双重for循环中重复计算x元素
    for (x = 0; x < dwDestWidth; x++){
        pdwSrcXTable[x]=(x * ptOriginPic->iWidth / ptZoomPic->iWidth);
    }

    for (y = 0; y < ptZoomPic->iHeight; y++){
        // 计算对应原图上的y值
        dwSrcY = y * ptOriginPic->iHeight / ptZoomPic->iHeight;
        // 将原图上对应点的像素拷贝到缩放后的图片中
        pucSrc = ptOriginPic->aucPixelDatas + dwSrcY * ptOriginPic->iLineBytes;
        pucDest = ptZoomPic->aucPixelDatas + y * ptZoomPic->iLineBytes;
        for (x = 0; x < dwDestWidth; x++){
            memcpy(pucDest + x * dwBytes, pucSrc + pdwSrcXTable[x] * dwBytes, dwBytes);
        }
    }

    free(pdwSrcXTable);

    return 0;
}
