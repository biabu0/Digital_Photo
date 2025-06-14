#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <config.h>
#include <input_manager.h>
#include <disp_manager.h>
#include <pic_operation.h>
#include <picfmt_manager.h>
#include <render.h>
#include <debug_manager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>




/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */
#define ArraySize 128

// ./digitalpic <bmp_file>
int main(int argc, char ** argv){
	int iError;
	// 初始化调试

	DebugInit();

	InitDebugChannel();
	if (argc != 2)
	{
		DBG_PRINTF("Usage:\n");
		DBG_PRINTF("%s <freetype_file>\n", argv[0]);
		return 0;
	}


	DisplayInit();

	SelectAndInitDefaultDispDev("fb");

	AllocVideoMem(5);
	InputInit();

	AllInputDeviceInit();
	EncodingInit();
	DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	iError = FontsInit();
	DBG_PRINTF("<3>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	if (iError)
	{
		DBG_PRINTF("FontsInit error!\n");
	}

	/* 设置freetype字库所用的文件和字体尺寸 */
	iError = SetFontsDetail("freetype", argv[1], 12);
	if (iError)
	{
		DBG_PRINTF("SetFontsDetail error!\n");
	}
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	PicFmtsInit();
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	PagesInit();
	DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	Page("main")->Run(NULL);
	return 0;
}