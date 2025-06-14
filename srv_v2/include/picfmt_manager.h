#ifndef _PICFMT_MANAGER_H
#define _PICFMT_MANAGER_H



int RegisterPicFileParser(PT_PicFileParser ptPicFileParser);
void ShowPicFmts(void);
PT_PicFileParser GetParser(PT_FileMap ptFileMap);
int BMP_Init(void);
int JPG_Init(void);
int PicFmtsInit(void);
PT_PicFileParser Parser(char *pcName);
#endif
