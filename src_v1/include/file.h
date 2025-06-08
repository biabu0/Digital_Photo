#ifndef _FILE_H
#define _FILE_H


// 文件结构体（图片文件）
typedef struct FileMap{
    char strFileName[128];
    int iFd;
    int iFileSize;
    unsigned char *pucFileMapMem;
}T_FileMap, *PT_FileMap;

void UnMapFile(PT_FileMap ptFileMap);
int MapFile(PT_FileMap ptFileMap);

#endif