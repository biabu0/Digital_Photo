#include<file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <config.h>

/* 对文件的操作 */

int MapFile(PT_FileMap ptFileMap){

    int iFd;
    struct stat tStat;

    iFd = open(ptFileMap->strFileName, O_RDWR);
	if(iFd == -1){
		DBG_PRINTF("can't open %s\n",  ptFileMap->strFileName);
        return -1;
	}

	fstat(iFd, &tStat);
    ptFileMap->iFd = iFd;
    ptFileMap->iFileSize = tStat.st_size;
	ptFileMap->pucFileMapMem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
	if(ptFileMap->pucFileMapMem == (void *)-1){
		DBG_PRINTF("mmap error\n");
        return -1;
	}

    return 0;
}

void UnMapFile(PT_FileMap ptFileMap){
    munmap(ptFileMap->pucFileMapMem, ptFileMap->iFileSize);
    close(ptFileMap->iFd);
}