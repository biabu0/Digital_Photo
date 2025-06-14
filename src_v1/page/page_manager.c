#include <config.h>
#include <page_manager.h>
#include <string.h>

static PT_PageAction g_ptPageActionHead;	//结构体链表头部

/*		注册结构体			将结构体加入到链表中取*/
int RegisterPageAction(PT_PageAction ptPageAction){

	PT_PageAction ptTmp;
	
	if(!g_ptPageActionHead){
		g_ptPageActionHead = ptPageAction;
		ptPageAction->ptNext = NULL;
	}else{
		ptTmp = g_ptPageActionHead;
		while(ptTmp->ptNext){
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptPageAction;
		ptPageAction->ptNext = NULL;
	}
	
	return 0;
}

void ShowPages(void){

	PT_PageAction ptTmp = g_ptPageActionHead;
	int i = 0;
	
	while(ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


/* 根据名字获取页面结构体 */
PT_PageAction Page(char *pcName){
	PT_PageAction ptTmp = g_ptPageActionHead;

	while(ptTmp){
		if(strcmp(ptTmp->name, pcName) == 0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}
int ID(char *strName){
	return (int)strName[0] + (int)strName[1] + (int)strName[2] + (int)strName[3];
}

int PagesInit(void){
	int iError;

	iError = MainPageInit();

	return iError;
}



