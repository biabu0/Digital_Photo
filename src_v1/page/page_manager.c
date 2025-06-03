#include <config.h>
#include <page_manager.h>
#include <string.h>

static PT_PageAction g_ptPageActionrHead;	//结构体链表头部

/*		注册结构体			将结构体加入到链表中取*/
int RegisterPageAction(PT_PageAction ptPageAction){

	PT_PageAction ptTmp;
	
	if(!g_ptPageActionrHead){
		g_ptPageActionrHead = ptPageAction;
		ptPageAction->ptNext = NULL;
	}else{
		ptTmp = g_ptPageActionrHead;
		while(ptTmp->ptNext){
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptPageAction;
		ptPageAction->ptNext = NULL;
	}
	
	return 0;
}

void ShowPages(void){

	PT_PageAction ptTmp = g_ptPageActionrHead;
	int i = 0;
	
	while(ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


/* 根据名字获取页面结构体 */
PT_PageAcction Page(char *pcName){

	PT_PageAction ptTmp = g_ptPageActionrHead;

	while(ptTmp){
		if(strcmp(ptTmp->name, pcName) == 0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}


int PagesInit(void){
	int iError;



	return iError;
}



