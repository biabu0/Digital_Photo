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

	// 初始化调试

	DebugInit();

	InitDebugChannel();

	DisplayInit();

	SelectAndInitDefaultDispDev("fb");

	AllocVideoMem(1);
	InputInit();
	AllInputDeviceInit();

	PagesInit();

	Page("main")->Run();
	return 0;
}