#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>
// 文件结构体（图片文件)
typedef struct FileMap{
    char strFileName[128];
    //int iFd;
    FILE * tFp;              /* 文件句柄 */
    int iFileSize;
    unsigned char *pucFileMapMem;
}T_FileMap, *PT_FileMap;

/* 文件类别 */
typedef enum {
	FILETYPE_DIR = 0,  /* 目录 */
	FILETYPE_FILE,     /* 文件 */
}E_FileType;

/* 目录里的内容 */
typedef struct DirContent {
	char strName[256];     /* 名字 */
	E_FileType eFileType;  /* 类别 */	
}T_DirContent, *PT_DirContent;


void UnMapFile(PT_FileMap ptFileMap);
int MapFile(PT_FileMap ptFileMap);
int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *piNumber);
void FreeDirContents(PT_DirContent *aptDirContents, int iNumber);
#endif