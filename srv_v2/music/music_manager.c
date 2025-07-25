/******************************************************
 * @brief  实现music界面设计
 * 
 * @author  bia布
 * @date    2025/07/21
 * @version 1.0
 ******************************************************/
#include <music_manager.h>
#include <file.h>
#include <debug_manager.h>
#include <common_st.h>

#include <string.h>
#include <stdio.h>


DECLARE_HEAD(g_tMusicParserHead);

int RegisterMusicParser(T_MusicParser *ptMusicParser){
    ListAddTail(&ptMusicParser->tMusicParser, &g_tMusicParserHead);
    return 0;
}

void UnregisterMusicParser(T_MusicParser *ptMusicParser){
    if(ptMusicParser != NULL){
        ListDelTail(&ptMusicParser->tMusicParser);
    }
}


T_MusicParser *GetMusicParser(char *pcName){
    struct list_head *ptLHTmpPos;
    struct MusicParser *ptMPTmpPos;
    LIST_FOR_EACH_ENTRY_H(ptLHTmpPos, &g_tMusicParserHead) {
        ptMPTmpPos = LIST_ENTRY(ptLHTmpPos, struct MusicParser, tMusicParser);

		if(0 == strcmp(pcName, ptMPTmpPos->name)){
			return ptMPTmpPos;
		} 

    }
    return NULL;
}


/**
 * @brief  获取文件支持的音频解释器
 * 
 * @param  ptFileDesc  文件映射结构体信息
 * @return T_MusicParser  音频格式结构体
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/22
 * @version 1.0
 */
T_MusicParser *GetSupportMusicParser(T_FileMap *ptFileDesc){
    struct list_head *ptLHTmpPos;	//LH = lis_head
	struct MusicParser *ptMPTmpPos;
	
	LIST_FOR_EACH_ENTRY_H(ptLHTmpPos, &g_tMusicParserHead){
		ptMPTmpPos = LIST_ENTRY(ptLHTmpPos, struct MusicParser, tMusicParser);

		if(ptMPTmpPos->isSupport(ptFileDesc)){
			return ptMPTmpPos;
		}
	}
    return NULL;
}


/**
 * @brief  判断是否是音频文件
 * 
 * @param  pcName  文件名
 * @return int  0--error  1--success
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/22
 * @version 1.0
 */
int isMusicSupport(char *pcName){
    DBG_PRINTF("MusicSupport\n");
    
    int iError = 0;
    T_FileMap tMusicFileDesc;

    snprintf(tMusicFileDesc.strFileName, 256, "%s", pcName);
    tMusicFileDesc.strFileName[255] = '\0';


    // 获取文件信息
    iError = MapFile(&tMusicFileDesc);
    if(iError){
        DBG_PRINTF(APP_ERR"Get file descriptor error.\n");
        return 0;
    }

    if(NULL == GetSupportMusicParser(&tMusicFileDesc)){
        UnMapFile(&tMusicFileDesc);
        return 0;
    }

    UnMapFile(&tMusicFileDesc);
    return 1;
}

/**
 * @brief  播放音频文件
 * 
 * @param  ptFileDesc  存储音频文件信息
 * @param  strFileName  音频文件路径
 * @return T_MusicParser*  音频文件结构体指针
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/22
 * @version 1.0
 */
T_MusicParser *PlayMusic(T_FileMap *ptFileDesc, char *strMusciPath){
    
    int iError;
    T_MusicParser *ptMusicParser;
    snprintf(ptFileDesc->strFileName, 256, "%s", strMusciPath);
    ptFileDesc->strFileName[255] = '\0';

    iError = MapFile(ptFileDesc);
    if(iError){
        DBG_PRINTF(APP_ERR"Get file descript error int isMusicSupported\n");
        return 0;
    }
    ptMusicParser = GetSupportMusicParser(ptFileDesc);

    if(NULL == ptMusicParser){
        UnMapFile(ptFileDesc);
        return NULL;
	}
    ptMusicParser->MusicPlay(ptFileDesc);
    return ptMusicParser;
}

/**
 * @brief  停止播放音频文件
 * 
 * @param  ptFileDesc  存储音频文件信息
 * @param  strFileName  音频文件路径
 * @return void
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/22
 * @version 1.0
 */
void StopMusic(T_FileMap *ptFileDesc, T_MusicParser *ptMusicParser){
	/* bug : 
	 * ReleaseFileDesc(ptFileDesc);
	 * ptMusicParser->MusicCtrl(MUSIC_CTRL_CODE_EXIT);
	 * 
	 * 由于先释放了文件内存，在线程里面可能会再次访问，所以导致段错误，互换位置就好了
	 */

	ptMusicParser->MusicCtrl(MUSIC_CTRL_CODE_EXIT);
	UnMapFile(ptFileDesc);
}

/**
 * @brief  音频播放控制接口函数
 * 
 * @param  T_MusicParser  音频解析器结构体
 * @param  eCtrlCode  控制字段
 * @return int
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/22
 * @version 1.0
 */
int CtrlMusic(T_MusicParser *ptMusicParser, E_MusicCtrlCode eCtrlCode)
{
	return ptMusicParser->MusicCtrl(eCtrlCode);
}

void ShowMusicParser(void){
    int iPTNum = 0;
    struct list_head *ptLHTmpPos;	//LH = lis_head
	struct MusicParser *ptMPTmpPos;
    LIST_FOR_EACH_ENTRY_H(ptLHTmpPos, &g_tMusicParserHead){
		ptMPTmpPos = LIST_ENTRY(ptLHTmpPos, struct MusicParser, tMusicParser);

		printf("%d <---> %s\n", iPTNum++, ptMPTmpPos->name);
	}
}


/**
 * @brief 初始化音乐解析模块，只要有一个初始化成功，就返回正确结果
 * 
 * @param  void
 * @return int
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/22
 * @version 1.0
 */
int MusicParserInit(void){
    int iError = 0;
    int iParserNum = 0;
    iError = MP3ParserInit();
    if(iError){
        DBG_PRINTF(APP_ERR"MP3ParserInit error\n");
        MP3ParserExit();
    }else{
        iParserNum++;
    }
    if(iParserNum){
        return 0;
    }

    return -1;
}
