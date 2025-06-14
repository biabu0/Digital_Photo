#include <config.h>
#include <pic_operation.h>
#include <picfmt_manager.h>
#include <file.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "jpeglib.h"
#include <setjmp.h>



static int JPGisSupport(PT_FileMap ptFileMap); 
static int JPGGetPixelDatas (PT_FileMap ptFileMap, PT_PixelDatas tPixelDatas);
static int JPGFreePixelDatas(PT_PixelDatas ptPixelDatas); 


// JPEG解码过程可以安全地从错误中恢复，而不是直接终止程序
typedef struct MyErrorMgr
{
    // 标准错误管理结构体
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
}T_MyErrorMgr, *PT_MyErrorMgr;


T_PicFileParser g_tJPGParser = {
    .name           = "jpg",
    .isSupport      = JPGisSupport,
    .GetPixelDatas  = JPGGetPixelDatas,
    .FreePixelDatas = JPGFreePixelDatas
};


/**
 * @brief  自定义的libjpeg库出错处理函数
 * 
 * @param  ptCInfo - libjpeg库抽象出来的通用结构体
 * @return void
 * 
 * @note    默认的错误处理函数是让程序退出，重定义该处理函数，JPEG解码过程可以安全地从错误中恢复，而不是直接终止程序
 * @author  bia布
 * @date    2025/06/13
 * @version 1.0
 */
static void MyErrorExit(j_common_ptr ptCInfo)
{
    static char errStr[JMSG_LENGTH_MAX];
    
	PT_MyErrorMgr ptMyErr = (PT_MyErrorMgr)ptCInfo->err;

    /* Create the message */
    (*ptCInfo->err->format_message) (ptCInfo, errStr);
    DBG_PRINTF("%s\n", errStr);

	longjmp(ptMyErr->setjmp_buffer, 1);
}

static int JPGisSupport(PT_FileMap ptFileMap){
    // 分配和初始化一个decompression结构体
    struct jpeg_decompress_struct tDInfo;

	//struct jpeg_error_mgr tJErr;   
	T_MyErrorMgr tJerr;
    int iRet;
    
    fseek(ptFileMap->tFp, 0, SEEK_SET);

	// 分配和初始化一个decompression结构体
	// tDInfo.err = jpeg_std_error(&tJErr);
	tDInfo.err               = jpeg_std_error(&tJerr.pub);
	tJerr.pub.error_exit     = MyErrorExit;

	if(setjmp(tJerr.setjmp_buffer))
	{
		/* 如果程序能运行到这里, 表示JPEG解码出错 */
        jpeg_destroy_decompress(&tDInfo);
		return 0;;
	}
	
	jpeg_create_decompress(&tDInfo);

	// 用jpeg_read_header获得jpg信息
	jpeg_stdio_src(&tDInfo, ptFileMap->tFp);

    iRet = jpeg_read_header(&tDInfo, TRUE);
	jpeg_abort_decompress(&tDInfo);
    return (iRet == JPEG_HEADER_OK);
}



/**
 * @brief  把已经从JPG文件取出的一行象素数据,转换为能在显示设备上使用的格式
 * 
 * @param  iWidth      - 宽度,即多少个象素
 *@param   iSrcBpp     - 已经从JPG文件取出的一行象素数据里面,一个象素用多少位来表示
 *@param   iDstBpp     - 显示设备上一个象素用多少位来表示
 *@param   pudSrcDatas - 已经从JPG文件取出的一行象素数所存储的位置
 *@param   pudDstDatas - 转换所得数据存储的位置
 * @return 0 - 成功, 其他值 - 失败
 * 
 * @author  bia布
 * @date    2025/06/13
 * @version 1.0
 */
static int CovertOneLine(int iWidth, int iSrcBpp, int iDstBpp, unsigned char *pudSrcDatas, unsigned char *pudDstDatas)
{
	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	unsigned int dwColor;

	unsigned short *pwDstDatas16bpp = (unsigned short *)pudDstDatas;
	unsigned int   *pwDstDatas32bpp = (unsigned int *)pudDstDatas;

	int i;
	int pos = 0;

	if (iSrcBpp != 24)
	{
		return -1;
	}

	if (iDstBpp == 24)
	{
		memcpy(pudDstDatas, pudSrcDatas, iWidth*3);
	}
	else
	{
		for (i = 0; i < iWidth; i++)
		{
			dwRed   = pudSrcDatas[pos++];
			dwGreen = pudSrcDatas[pos++];
			dwBlue  = pudSrcDatas[pos++];
			if (iDstBpp == 32)
			{
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;
				pwDstDatas32bpp++;
			}
			else if (iDstBpp == 16)
			{
				/* 565 */
				dwRed   = dwRed >> 3;
				dwGreen = dwGreen >> 2;
				dwBlue  = dwBlue >> 3;
				dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue);
				*pwDstDatas16bpp = dwColor;
				pwDstDatas16bpp++;
			}
		}
	}
	return 0;
}



static int JPGGetPixelDatas (PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas){
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    struct jpeg_decompress_struct tDInfo;
	//struct jpeg_error_mgr tJErr;
    int iRet;
    int iRowStride;
    unsigned char *aucLineBuffer = NULL;
    unsigned char *pucDest;
	T_MyErrorMgr tJerr;

    fseek(ptFileMap->tFp, 0, SEEK_SET);

	// 分配和初始化一个decompression结构体
	//tDInfo.err = jpeg_std_error(&tJErr);

	tDInfo.err               = jpeg_std_error(&tJerr.pub);
	tJerr.pub.error_exit     = MyErrorExit;

	if(setjmp(tJerr.setjmp_buffer))
	{
		/* 如果程序能运行到这里, 表示JPEG解码出错 */
        jpeg_destroy_decompress(&tDInfo);
        if (aucLineBuffer)
        {
            free(aucLineBuffer);
        }
        if (ptPixelDatas->aucPixelDatas)
        {
            free(ptPixelDatas->aucPixelDatas);
        }
		return -1;
	}

	jpeg_create_decompress(&tDInfo);

	// 用jpeg_read_header获得jpg信息
	jpeg_stdio_src(&tDInfo, ptFileMap->tFp);

    iRet = jpeg_read_header(&tDInfo, TRUE);
    (void)iRet;

	// 设置解压参数,比如放大、缩小
    tDInfo.scale_num = tDInfo.scale_denom = 1;
    
	// 启动解压：jpeg_start_decompress	
	jpeg_start_decompress(&tDInfo);
    
	// 一行的数据长度
	iRowStride = tDInfo.output_width * tDInfo.output_components;
	aucLineBuffer = malloc(iRowStride);

    if (NULL == aucLineBuffer)
    {
        return -1;
    }
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	ptPixelDatas->iWidth  = tDInfo.output_width;
	ptPixelDatas->iHeight = tDInfo.output_height;
	//ptPixelDatas->iBpp    = iBpp;
	ptPixelDatas->iLineBytes    = ptPixelDatas->iWidth * ptPixelDatas->iBpp / 8;
    ptPixelDatas->iTotalBytes   = ptPixelDatas->iHeight * ptPixelDatas->iLineBytes;
	ptPixelDatas->aucPixelDatas = malloc(ptPixelDatas->iTotalBytes);
	if (NULL == ptPixelDatas->aucPixelDatas)
	{
		return -1;
	}

    pucDest = ptPixelDatas->aucPixelDatas;

	// 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
	while (tDInfo.output_scanline < tDInfo.output_height) 
	{
        /* 得到一行数据,里面的颜色格式为0xRR, 0xGG, 0xBB */
		(void) jpeg_read_scanlines(&tDInfo, &aucLineBuffer, 1);

		// 转到ptPixelDatas去
		CovertOneLine(ptPixelDatas->iWidth, 24, ptPixelDatas->iBpp, aucLineBuffer, pucDest);
		pucDest += ptPixelDatas->iLineBytes;
	}
	
	free(aucLineBuffer);
	jpeg_finish_decompress(&tDInfo);
	jpeg_destroy_decompress(&tDInfo);

    return 0;
}
static int JPGFreePixelDatas(PT_PixelDatas ptPixelDatas){
    free(ptPixelDatas->aucPixelDatas);
	return 0;
}

int JPG_Init(void){
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return RegisterPicFileParser(&g_tJPGParser);
}

