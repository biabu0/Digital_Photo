#include<stdio.h>
#include<stdlib.h>
#include "jpeglib.h"
#include <setjmp.h>

/*
    代码流程
	Allocate and initialize a JPEG decompression object    // 分配和初始化一个decompression结构体
	Specify the source of the compressed data (eg, a file) // 提定源文件
	Call jpeg_read_header() to obtain image info           // 用jpeg_read_header获得jpg信息
	Set parameters for decompression                       // 设置解压参数,比如放大、缩小
	jpeg_start_decompress(...);                            // 启动解压：jpeg_start_decompress
	while (scan lines remain to be read)
		jpeg_read_scanlines(...);                          // 循环调用jpeg_read_scanlines
	jpeg_finish_decompress(...);                           // jpeg_finish_decompress
	Release the JPEG decompression object                  // 释放decompression结构体
*/

/*Usage: jog2rgb <jpgfile>
*/
int main(int argc, char **argv){

    // 分配和初始化一个decompression结构体
    struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
    FILE * infile;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

    // 提定源文件
	if ((infile = fopen(argv[1], "rb")) == NULL) {
	    fprintf(stderr, "can't open %s\n", argv[1]);
	    return -1;
	}
	//
	jpeg_stdio_src(&cinfo, infile);

    // 用jpeg_read_header获得jpg信息
    jpeg_read_header(&cinfo, TRUE);

    // 打印jpg信息
    printf("image_width = %d, image_height = %d\n", cinfo.image_width, cinfo.image_height);
    printf("num_components = %d\n", cinfo.num_components);
    // 输出图像信息
    printf("output_width = %d, output_height = %d\n", cinfo.output_width, cinfo.output_height);

	// 设置解压参数,比如放大、缩小

	// 启动解压：jpeg_start_decompress
    jpeg_start_decompress(&cinfo);
	
	// 循环调用jpeg_read_scanlines


	// jpeg_finish_decompress
    jpeg_finish_decompress(&cinfo);
	// 释放decompression结构体
    jpeg_destroy_decompress(&cinfo);

    return 0;
}

