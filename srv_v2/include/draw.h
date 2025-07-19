#ifndef _DRAW_H
#define _DRAW_H

#include <disp_manager.h>

int OpenTextFile(char *pcFileName);
int SetFontDetail(char *pcHZKFile, char *pcFileFreetype, unsigned int dwFontSize);
int SelectAndInitDisplay(char *pcName);
int ShowNextPage(void);
int ShowPrePage(void);
int IsTxtFileByExtension(const char *filename);
int DrawInit(void);
//int GetDispResolution(int *piXres, int *piYres, int *piBpp);
int ShowTextInReadingPage(PT_VideoMem ptVideoMem, char *strFileName);
#endif