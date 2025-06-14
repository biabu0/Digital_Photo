#include <config.h>
#include <pic_operation.h>
#include <picfmt_manager.h>
#include <string.h>
#include<file.h>

static PT_PicFileParser g_ptPicFileParserHead;

/**
 * @brief  注册"图片文件解析模块", "图片文件解析模块"就是怎么从BMP/JPG等图片文件中解析出象素数据
 * 
 * @param  ptPicFileParser - 一个结构体,内含"图片文件解析模块"的操作函数
 * @return int  0 - 成功, 其他值 - 失败

 * @author  bia布
 * @date    2025/06/08
 * @version 1.0
 */
int RegisterPicFileParser(PT_PicFileParser ptPicFileParser){
    PT_PicFileParser ptTmp;
    if(!g_ptPicFileParserHead){
        g_ptPicFileParserHead = ptPicFileParser;
        ptPicFileParser->ptNext = NULL;
    }else{
        ptTmp = g_ptPicFileParserHead;
        while(ptTmp->ptNext){
            ptTmp = ptTmp->ptNext;
        }
        ptTmp->ptNext = ptPicFileParser;
        ptPicFileParser->ptNext = NULL;
    }
    return 0;
}

/**
 * @brief  显示本程序能支持的"图片文件解析模块"
 * 
 * @author  bia布
 * @date    2025/06/08
 * @version 1.0
 */
void ShowPicFmts(void){
	int i = 0;
	PT_PicFileParser ptTmp = g_ptPicFileParserHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_PicFileParser Parser(char *pcName)
{
	PT_PicFileParser ptTmp = g_ptPicFileParserHead;
	
	while (ptTmp)
	{
        DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		if (strcmp(ptTmp->name, pcName) == 0)
		{
            DBG_PRINTF("<7>%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            DBG_PRINTF("<7>ptTmp: %s\n", ptTmp->name);
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

/**
 * @brief   找到能支持指定文件的"图片文件解析模块"
 * 
 * @param   ptFileMap - 内含文件信息
 * @return  NULL   - 失败,没有指定的模块, 
 *            非NULL - 支持该文件的"图片文件解析模块"的PT_PicFileParser结构体指针

 * @author  bia布
 * @date    2025/06/08
 * @version 1.0
 */
PT_PicFileParser GetParser(PT_FileMap ptFileMap){
    PT_PicFileParser ptTmp;
    ptTmp = g_ptPicFileParserHead;
    while(ptTmp){
        if(ptTmp->isSupport(ptFileMap)){
            return ptTmp;
        }
        ptTmp = ptTmp->ptNext;
    }
    return NULL;
}

int PicFmtsInit(void){
	int iError;

	iError = BMP_Init();
    iError = JPG_Init();
		
	return iError;
}