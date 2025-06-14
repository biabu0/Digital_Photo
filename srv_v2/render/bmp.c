
#include<stdlib.h>
#include<stdio.h>
#include <string.h>
#include<pic_operation.h>
#include <picfmt_manager.h>
#include<config.h>
#include<file.h>

#pragma pack(push)      // 将当前pack设置压栈保存
#pragma pack(1)         // 设置pack为1字节对齐

// bmp格式分析参考 ：https://yushi.blog.csdn.net/article/details/128646165?fromshare=blogdetail&sharetype=blogdetail&sharerId=128646165&sharerefer=PC&sharesource=2201_75766594&sharefrom=from_link
// 结构体要对齐

typedef struct tagBITMAPFILEHEADER {
        unsigned short    bfType; //图片种类，BMP图片固定为BM，表示为十六进制就是0x4d42
        unsigned long     bfSize; //该图片文件的大小
        unsigned short    bfReserved1; //保留字，不用管
        unsigned short    bfReserved2;//保留字，不用管
        unsigned long     bfOffBits; //实际图片数据的偏移量，即`DIB`的偏移量，也即前三个结构体的大小
} BITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER{
        unsigned long      biSize; //指定此结构体的长度
        unsigned long       biWidth; //bmp图片的宽度
        unsigned long       biHeight; //bmp图片的高度
        unsigned short       biPlanes; //平面数，显示器只有一个平面，所以一般为1
        unsigned short       biBitCount; //颜色位数，目前一般用24位或32位
        unsigned long      biCompression; //压缩方式，可以是0，1，2，0表示不压缩，BMP为不压缩，所以为0
        unsigned long      biSizeImage; //实际位图数据占用的字节数.由于上面不压缩，所以这里填0即可
        unsigned long       biXPelsPerMeter; //X方向分辨率，即每米有多少个像素，可以省略
        unsigned long       biYPelsPerMeter; //Y方向分辨率，即每米有多少个像素，可以省略
        unsigned long      biClrUsed;  //使用的颜色数，如果为0，则表示默认值(2^颜色位数)
        unsigned long      biClrImportant; //重要颜色数，如果为0，则表示所有颜色都是重要的
} BITMAPINFOHEADER;

#pragma pack(pop)       //恢复之前的pack设置
//#pragma pack()       //恢复之前的pack设置

static int BMPisSupport(PT_FileMap ptFileMap); //根据文件头部判断是否支持该格式的解析
static int BMPGetPixelDatas (PT_FileMap ptFileMap, PT_PixelDatas tPixelDatas);
static int BMPFreePixelDatas(PT_PixelDatas ptPixelDatas);        //释放内存


T_PicFileParser g_tBMPParser = {
    .name           = "bmp",
    .isSupport      = BMPisSupport,
    .GetPixelDatas  = BMPGetPixelDatas,
    .FreePixelDatas = BMPFreePixelDatas
};


static int BMPisSupport(PT_FileMap ptFileMap){
    unsigned char *aFileHead = ptFileMap->pucFileMapMem;
    DBG_PRINTF("aFileHead[0]:%s\n",aFileHead[0]);
    DBG_PRINTF("aFileHead[1]:%s\n",aFileHead[1]);
    if(aFileHead[0] != 0x42 || aFileHead[1] != 0x4d)
        return 0;
    else
        return 1;
}

static int CovertOneLineFrmBMP(int iWidth, int iSrcBpp, int iDstBpp, unsigned char *pucSrcDatas, unsigned char *pucDstDatas){
    unsigned int dwRed;
    unsigned int dwGreen;
    unsigned int dwBlue;
    unsigned int dwColor;

    unsigned short *pwDstDatas16Bpp = (unsigned short *)pucDstDatas;
    unsigned int *pwDstDatas32Bpp = (unsigned int *)pucDstDatas;
    int i;
    int pos = 0;

    if(iSrcBpp != 24){
        return -1;
    }
    // Bpp一样则不需要处理
    if(iDstBpp == 24){
        memcpy(pucDstDatas, pucSrcDatas, iWidth * 3);
    }else{
        for(i = 0; i < iWidth; i++){
            dwBlue = pucSrcDatas[pos++];
            dwGreen = pucSrcDatas[pos++];
            dwRed = pucSrcDatas[pos++];

            if(iDstBpp == 32){
                dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
                *pwDstDatas32Bpp = dwColor; 
                pwDstDatas32Bpp++;
            }else if(iDstBpp == 16){
                // 565
                dwRed = dwRed >> 3;
                dwGreen = dwGreen >> 2;
                dwBlue = dwBlue >> 3;
                dwColor = (dwRed << 11) | (dwGreen << 5) | dwBlue;
                *pwDstDatas16Bpp = dwColor;
                pwDstDatas16Bpp++;
            }
        }
    }
    return 0;
}

/**
 * @brief  把BMP文件中的象素数据,取出并转换为能在显示设备上使用的格式
 * 
 * @param  ptFileMap    - 内含文件信息
 * @param  ptPixelDatas - 内含象素数据
 * @return int  0 - 成功, 其他值 - 失败
 * 
 * @note   修改传入参数，从aFileHead修改为ptFileMap
 * @author  bia布
 * @date    2025/06/10
 * @version 2.0
 */

static int BMPGetPixelDatas (PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas){
    BITMAPFILEHEADER *ptBITMAPFILEHEADER;
    BITMAPINFOHEADER *ptBITMAPINFOHEADER;
    int iWidth, iHeight, iBMPBpp;
    unsigned char *pucSrc;
    unsigned char *pucDest;
    int iLineWidthReal;
    int iLineWidthAlign;
    int y;
    unsigned char *aFileHead = ptFileMap->pucFileMapMem;

    ptBITMAPFILEHEADER = (BITMAPFILEHEADER *)aFileHead;
    ptBITMAPINFOHEADER = (BITMAPINFOHEADER *)(aFileHead + sizeof(BITMAPFILEHEADER));
    iBMPBpp = ptBITMAPINFOHEADER->biBitCount;
    iWidth = ptBITMAPINFOHEADER->biWidth;
    iHeight = ptBITMAPINFOHEADER->biHeight;

    // bmp格式的Bpp为24；Bpp‌（Bits Per Pixel）指每像素所占用的存储位数
    if(iBMPBpp != 24){
        DBG_PRINTF("iBMPBpp = %d\n", iBMPBpp);
        DBG_PRINTF("sizeof(BITMAPFILEHEADER) = %d\n", sizeof(BITMAPFILEHEADER));
        return -1;
    }
    ptPixelDatas->iWidth = iWidth;
    ptPixelDatas->iHeight = iHeight;
    //ptPixelDatas->iBpp = iBpp;
    ptPixelDatas->aucPixelDatas = malloc(iWidth * iHeight * ptPixelDatas->iBpp / 8);
    ptPixelDatas->iLineBytes = iWidth * ptPixelDatas->iBpp / 8;
    if(NULL == ptPixelDatas->aucPixelDatas){
        return -1;
    }
    iLineWidthReal = iWidth * iBMPBpp / 8;
    // 每一行要是4的整数倍
    iLineWidthAlign = (iLineWidthReal + 3) & ~0x3;

    // 原文件像素数据位置
    pucSrc = aFileHead + ptBITMAPFILEHEADER->bfOffBits;
    // 原文件数据存放的位置是先从图片左下角的数据开始逐行存放，而LCDframebuffer数据存放的是从图片左上角开始逐行存放
    // 故先将文件数据位置定位到图片左上角
    pucSrc = pucSrc + iLineWidthAlign * (iHeight - 1);

    pucDest = ptPixelDatas->aucPixelDatas;

    for(y = 0; y < iHeight; y++){
        //memcpy(pucDest, pucSrc, iLineWidthReal);
        // 转换一行数据
        CovertOneLineFrmBMP(iWidth, iBMPBpp, ptPixelDatas->iBpp, pucSrc, pucDest);
        pucSrc -= iLineWidthAlign;
        pucDest += ptPixelDatas->iLineBytes;
    }
    return 0;
}
static int BMPFreePixelDatas(PT_PixelDatas ptPixelDatas){
    if (!ptPixelDatas || !ptPixelDatas->aucPixelDatas) {
        printf("Invalid pointer detected\n");
        return -1;
    }
    free(ptPixelDatas->aucPixelDatas);

    return 0;
}
// 2025/06/08
int BMP_Init(void){
    return RegisterPicFileParser(&g_tBMPParser);
}