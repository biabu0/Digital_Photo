/******************************************************
 * @brief  实现music界面设计
 * 
 * @author  bia布
 * @date    2025/07/21
 * @version 1.0
 ******************************************************/
#ifndef _MUSIC_MANAGER_H_
#define _MUSIC_MANAGER_H_ 

#include <config.h>
#include <common_st.h>
#include <file.h>
#include <alsa/asoundlib.h>

typedef enum MusicCtrlCode
{
	MUSIC_CTRL_CODE_PLAY = 0,
	MUSIC_CTRL_CODE_START,
	MUSIC_CTRL_CODE_HALT,
	MUSIC_CTRL_CODE_EXIT,
	MUSIC_CTRL_CODE_ADD_VOL,
	MUSIC_CTRL_CODE_DEC_VOL,
	MUSIC_CTRL_CODE_GET_VOL,
	MUSIC_CTRL_CODE_GET_RUNTIME,
	MUSIC_CTRL_CODE_MAX,
}E_MusicCtrlCode;


// Pcm硬件参数结构体
struct PcmHardwareParams{
	snd_pcm_access_t eAccessMod;
	unsigned int eFmtMod;
	unsigned int dwSampRate;
	unsigned int dwChannels;
};




// 音频解析器结构体
typedef struct MusicParser {
    char *name;                         // 解析器名称标识（如"MP3"）
    int (*MusicDecoderInit)(void);      // 初始化解码器资源
    void (*MusicDecodeExit)(void);      // 释放解码器资源
    int (*isSupport)(T_FileMap*); // 检测文件格式是否支持
    int (*MusicPlay)(T_FileMap*); // 启动音乐播放
    int (*MusicCtrl)(enum MusicCtrlCode);// 播放控制接口
    struct list_head tMusicParser;      // 链表节点（用于注册管理）
}T_MusicParser, *PT_MusicParser;


int RegisterMusicParser(T_MusicParser *ptMusicParser);
void UnregisterMusicParser(T_MusicParser *ptMusicParser);
void ShowMusicParser(void);
T_MusicParser *GetMusicParser(char *pcName);
T_MusicParser *GetSupportMusicParser(T_FileMap *ptFileDesc);
int isMusicSupport(char *pcName);
T_MusicParser *PlayMusic(T_FileMap *ptFileDesc, char *strMusciPath);
void StopMusic(T_FileMap *ptFileDesc, T_MusicParser *ptMusicParser);
int CtrlMusic(T_MusicParser *ptMusicParser, E_MusicCtrlCode eCtrlCode);
int MusicParserInit(void);
int MP3ParserInit(void);
void MP3ParserExit(void);


#endif