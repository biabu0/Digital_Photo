#ifndef _PIC_OPERATION_H_
#define _PIC_OPERATION_H_ 


typedef struct PixelDatas{
    int iWidth;
    int iHeight;
    int iBpp;
    int iLineBytes;
    // 指针指向像素数据，需要分配内存
    unsigned char *aucPixelDatas;
}T_PixelDatas, *PT_PixelDatas;



typedef struct PicFileParser{
    char *name;
    int (*isSupport)(unsigned char *aFileHead); //根据文件头部判断是否支持该格式的解析
    int (*GetPixelDatas) (unsigned char *aFileHead, PT_PixelDatas tPixelDatas);
    int (*FreePixelDatas)(PT_PixelDatas tPixelDatas);        //释放内存
}T_PicFileParser, *PT_PicFileParser;


#endif