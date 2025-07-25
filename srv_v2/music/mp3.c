/******************************************************
 * @brief  mp3解释器
 * 
 * @author  bia布
 * @date    2025/07/21
 * @version 1.0
 ******************************************************/

#include <music_manager.h>
#include <mad.h>
#include <alsa/asoundlib.h>
#include <debug_manager.h>

#include <stdlib.h>
#include <string.h>
#include <pthread.h>


//MP3文件大体上分为三个部分：ID3V2 + 音频数据 + ID3V1

// IDxVx 的头部结构
struct IDxVxHeader
{
    unsigned char aucIDx[3];     /* 保存的值比如为"ID3"表示是ID3V2 */
    unsigned char ucVersion;     /* 如果是ID3V2.3则保存3,如果是ID3V2.4则保存4 */
    unsigned char ucRevision;    /* 副版本号 */
    unsigned char ucFlag;        /* 存放标志的字节 */// 标志字节，只使用高三位，其它位为0
    unsigned char aucIDxSize[4]; /* 整个 IDxVx 的大小，除去本结构体的 10 个字节 */    /* 只有后面 7 位有用 */
};


// 音频数据帧的帧头
#pragma pack(push, 1)
struct DataFrameHeader
{
	unsigned int bzFrameSyncFlag1:8;   /* 全为 1 */
	unsigned int bzProtectBit:1;       /* CRC */
	unsigned int bzVersionInfo:4;      /* 包括 mpeg 版本，layer 版本 */
	unsigned int bzFrameSyncFlag2:3;   /* 全为 1 */
	unsigned int bzPrivateBit:1;       /* 私有 */
	unsigned int bzPaddingBit:1;      /* 是否填充，1 填充，0 不填充
	layer1 是 4 字节，其余的都是 1 字节 */
	unsigned int bzSampleIndex:2;     /* 采样率索引 */
	unsigned int bzBitRateIndex:4;    /* bit 率索引 */
	unsigned int bzExternBits:6;      /* 版权等，不关心 */	
	unsigned int bzCahnnelMod:2;      /* 通道
	* 00 - Stereo 01 - Joint Stereo
	* 10 - Dual   11 - Single
	*/
};
#pragma pack(pop)


//管理PCM音频数据的结构体，存放的是MP3的音频数据
struct PcmFmtParams
{
    unsigned char *pucDataStart;
    unsigned long dwDataLength;
	/* 要统计歌曲播放长度，但是 dwDataLength
	 * 被解码函数的 input 置 0，只能在下面备份一下
	 */
	unsigned long dwDataLengthBak;
	unsigned char *pucDataEnd;    
};


// 比特率索引表，4位，15个索引
static int g_aiMP3BitRateIndex[] = {
	0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0
};
// 采样率索引表，3位，4个索引
static int g_aiMP3SampleRateIndex[] = {
	44100, 48000, 32000, 0,
};

#define VOL_CHANGE_NUM 2
#define DEFAULT_VOL_VAL "10"
static char g_cVolValue = 10;    /* 默认的声音值 */
static int g_iRuntimeRatio = 0;
static char g_bMusicHalt = 0;

static char g_bMP3TimeThreadExit = 1;
static char g_bMP3PlayThreadExit = 1;

static snd_pcm_t *g_ptMP3PlaybackHandle;
static unsigned char g_bMp3DecoderExit = 0;
static int g_iMP3PlayTime;
static struct PcmFmtParams g_tMP3PcmFmtParams;
static pthread_once_t g_PthreadOnce = PTHREAD_ONCE_INIT;
static pthread_t g_MP3TimeThreadId;
static pthread_t g_MP3PlayThreadId;
static pthread_mutex_t g_tMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tConVar = PTHREAD_COND_INITIALIZER;



static int MP3MusicDecoderInit(void){
	return 0;
}

static void MP3MusicDecodeExit(void){	
	
}

// 由于 mp3 格式不是很固定，所以凭借后缀名来判断是不是 mp3 格式文件
static int isSupportMP3(T_FileMap *ptFileDesc){ 
    char *pcName;
    int iError = 0;

    pcName = strrchr(ptFileDesc->strFileName, '.');
    if(pcName == NULL){
        return 0;
    }
	printf("mp3 name is %s\n", pcName);
    iError = strcmp(pcName+1, "mp3");
    return !iError;
}


/**
 * @brief  调用 ALSA 库函数打开 PCM 设备
 * 
 * @param  strDevName  设备名称（如 "default" 表示默认声卡）。
 * @param  eSndPcmStream 流类型（SND_PCM_STREAM_PLAYBACK 播放，SND_PCM_STREAM_CAPTURE 录音）。
 * @return 成功返回PCM设备的句柄（snd_pcm_t*），失败返回 NULL。
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static snd_pcm_t *OpenPcmDev(char *strDevName, snd_pcm_stream_t eSndPcmStream)
{
	snd_pcm_t *SndPcmHandle;
	int iError = 0;
    // 调用 ALSA 库函数打开 PCM 设备
    // ALSA 库：基于 Linux 的音频设备驱动库（libasound），用于管理声卡硬件。
	iError = snd_pcm_open (&SndPcmHandle, strDevName, eSndPcmStream, 0);
	if (iError < 0) {
		return NULL;
	}

	return SndPcmHandle;
}

/**
 * @brief  配置 PCM 设备的硬件参数（如格式、采样率、声道数等），确保音频数据能正确传输。
 * 
 * @param  strDevName  设备名称（如 "default" 表示默认声卡）。
 * @param  eSndPcmStream 流类型（SND_PCM_STREAM_PLAYBACK 播放，SND_PCM_STREAM_CAPTURE 录音）。
 * @return int 0 - 成功，-1 - 失败
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static int SetHardwareParams(snd_pcm_t *ptPcmHandle, struct PcmHardwareParams *ptPcmHWParams){
	int iError = 0;
	// 1.首先给参数配置相应的空间，并且根据当前设备的具体情况进行初始化
	snd_pcm_hw_params_t *HardWareParams;

	snd_pcm_format_t ePcmFmt = SND_PCM_FORMAT_S16_LE; /* 默认值 */
	unsigned int dwSampRate = 44100; /* 默认值 */
	

	switch(ptPcmHWParams->eFmtMod){
		case 8: {
			ePcmFmt = SND_PCM_FORMAT_S8;			
			break;
		}
		case 16: {
			ePcmFmt = SND_PCM_FORMAT_S16_LE;
			break;
		}
		case 24: {
			ePcmFmt = SND_PCM_FORMAT_S24_LE;
			break;
		}
		case 32: {
			ePcmFmt = SND_PCM_FORMAT_S32_LE;
			break;
		}
		default : {
			DBG_PRINTF(APP_ERR"Unsupported format bits : %d\n", ptPcmHWParams->eFmtMod);
			return -1;
			break;
		}
	}
	dwSampRate = ptPcmHWParams->dwSampRate;    /* 采样率 */

	iError = snd_pcm_hw_params_malloc (&HardWareParams);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Cannot allocate memory for PCM hardware params\n");
		return -1;
	}
	// 1.1 将硬件参数设置为默认值
	iError = snd_pcm_hw_params_any(ptPcmHandle, HardWareParams);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Cannot set default PCM hardware params\n");
		free(HardWareParams);
		HardWareParams = NULL;
		return -1;
	}

	/* 2-5的设置函数都是通过 snd_pcm_hw_param_set 来完成，
	不同设置使用不同的参数来指定，通过这些参数将采样率、声道数等信息配置到参数结构体HardWareParams中*/

	// 2.设置访问模式(交错模式，帧连续)
	iError = snd_pcm_hw_params_set_access(ptPcmHandle, HardWareParams, ptPcmHWParams->eAccessMod);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Cannot set PCM hardware params access mode\n");
		free(HardWareParams);
		HardWareParams = NULL;
		return -1;
	}

	// 3.设置量化参数，默认是signed 16 bit
	iError = snd_pcm_hw_params_set_format(ptPcmHandle, HardWareParams, ePcmFmt);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Set format mod error\n");
		free(HardWareParams);
		HardWareParams = NULL;
		return -1;
	}

	// 5.设置声道数
	iError = snd_pcm_hw_params_set_channels(ptPcmHandle, HardWareParams, ptPcmHWParams->dwChannels);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Set channel mod error\n");
		free(HardWareParams);
		HardWareParams = NULL;
		return -1;
	}
	
	// 4.设置采样率44100
	iError = snd_pcm_hw_params_set_rate_near(ptPcmHandle, HardWareParams, &dwSampRate, NULL);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Set rate mod error\n");
		free(HardWareParams);
		HardWareParams = NULL;
		return -1;
	}

	// 6. 将配置好的硬件参数写入到设备句柄中
	iError = snd_pcm_hw_params(ptPcmHandle, HardWareParams);
	if(iError < 0){
		printf("iError = %d", iError);
		DBG_PRINTF(APP_ERR"Write hardware parameters error\n");
		free(HardWareParams);
		HardWareParams = NULL;
		return -1;
	}

	// 7. 释放参数结构体
	snd_pcm_hw_params_free(HardWareParams);

	// 8. 将PCM设备设置为准备状态，等待数据写入或读取
	iError = snd_pcm_prepare(ptPcmHandle);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Cannot prepare PCM device\n");
		return -1;
	}

	return 0;
}



/**
 * @brief  设置音量的值，获取或者写入。
 * 
 * @param  iFlag  为1时表示只读模式（获取音量），为0时表示写入模式（设置音量）
 * @param  strVol: 音量值的字符串表示（如 "50%"）
 * @return int 
 * @note    
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static int SetVol(char *strVol, int iFlag){ 
	int iError;
	snd_ctl_t *handle = NULL;		// ALSA 控制接口的句柄。
	snd_ctl_elem_info_t *info;		// ALSA 控制接口的参数信息结构体。存储音频元素（如音量控件）的元信息。
	snd_ctl_elem_id_t *id;			// ALSA 控制接口的参数标识符结构体。标识音频元素的唯一 ID。
	snd_ctl_elem_value_t *control;	// ALSA 控制接口的参数值结构体。存储音频元素的当前值。
	// 在栈上动态分配 ALSA 结构体内存。使用 alloca 分配的内存会在函数返回时自动释放。
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);

	// 1.将字符串 "numid=45" 解析为音频元素的 ID。
	if(snd_ctl_ascii_elem_id_parse(id, "numid=45")){
		return -1;
	}

	// 2.以非阻塞模式打开默认音频控制设备(声卡)。
	if((iError = snd_ctl_open(&handle, "default", SND_CTL_NONBLOCK)) < 0){
		DBG_PRINTF(APP_ERR"Cannot open PCM control interface\n");
		return iError;
	}

	// 3. 获取音频元素的元信息:通过句柄 handle 获取指定音频元素的元信息。
	snd_ctl_elem_info_set_id(info, id);
	// 调用 snd_ctl_elem_info 查询元信息。
	if((iError = snd_ctl_elem_info(handle, info)) < 0){
		DBG_PRINTF(APP_ERR"Cannot get PCM element info\n");
		snd_ctl_close(handle);
		handle = NULL;
		return iError;
	}

	// 4.绑定控件值与ID：将control与音频元素ID绑定，用于后续读写操作
	snd_ctl_elem_value_set_id(control, id);

	// 5.如果是只读模式
	if(iFlag){
		return (int)snd_ctl_elem_value_get_integer(control, 1);
	}

	// 6.写入模式
	// 6.1. 读取控件值，从音频设备读取当前控件值到 control 结构体，在写入新值前必须读取当前状态（ALSA 要求先读后写）。
	if((iError = snd_ctl_elem_read(handle, control)) < 0){
		DBG_PRINTF(APP_ERR"Cannot read the given element from control\n");
		snd_ctl_close(handle);
		handle = NULL;
		return iError;
	}
	// 6.2. 将字符串解析为设备可识别的格式
	iError = snd_ctl_ascii_value_parse(handle, control, info, strVol);
	if(iError < 0){
		DBG_PRINTF(APP_ERR"Control parse error\n");
		snd_ctl_close(handle);
		handle = NULL;
		return iError;
	}

	// 6.3. 将修改后的control写入音频设备
	iError = snd_ctl_elem_write(handle, control);
	if(iError < 0){
		DBG_PRINTF("Control element write error.\n");
		snd_ctl_close(handle);
		handle = NULL;
		return iError;
	}

	snd_ctl_close(handle);
	handle = NULL;

	return 0;
}


// 初始化音量线程，只执行一次，避免重复设置
static void SetVolThreadOnce(void)
{
	SetVol(DEFAULT_VOL_VAL, 0);
	g_cVolValue = atoi(DEFAULT_VOL_VAL);		
}


static int SetMP3OutPcmFmt(void){
	int iError = 0;
	// PCM硬件参数结构体设置
	struct PcmHardwareParams tPcmHWParams;
	tPcmHWParams.eAccessMod = SND_PCM_ACCESS_RW_INTERLEAVED;	// 设置为交错读写模式
	tPcmHWParams.dwSampRate = 44100;							// CD音质标准采样频率
	tPcmHWParams.dwChannels = 2;								// 立体声
	tPcmHWParams.eFmtMod 	= 16;								// 16位量化标准

	// 打开一个音频设备，获取PCM设备句柄，设置为播放
	g_ptMP3PlaybackHandle = OpenPcmDev("default", SND_PCM_STREAM_PLAYBACK);
	if(g_ptMP3PlaybackHandle == NULL){
		DBG_PRINTF(APP_ERR"Cannot open PCM device");
		return -1;
	}
	// 设置PCM硬件参数
	iError = SetHardwareParams(g_ptMP3PlaybackHandle, &tPcmHWParams);
	if(iError){
		DBG_PRINTF(APP_ERR"Cannot set PCM hardware params\n");
		// 关闭设备句柄
		snd_pcm_close(g_ptMP3PlaybackHandle);
		return -1;
	}

	// 线程安全的单次初始化：无论多少线程调用 SetMp3OutPcmFmt，SetVolThreadOnce 音量初始化函数 只会被执行一次。
	pthread_once(&g_PthreadOnce, SetVolThreadOnce);
	return 0;
}


// 检测并计算需要跳过的ID3标签内容长度
static int SkipIDxVxContents(struct IDxVxHeader *ptIDxVxHeader)
{
    int iSkipNum;

    if(!strncmp("ID3", (char *)ptIDxVxHeader->aucIDx, 3)){
        iSkipNum = (ptIDxVxHeader->aucIDxSize[0] & 0x7f) * 0x200000
			+ (ptIDxVxHeader->aucIDxSize[1] & 0x7f) * 0x4000
			+ (ptIDxVxHeader->aucIDxSize[2] & 0x7f) * 0x80
			+ (ptIDxVxHeader->aucIDxSize[3] & 0x7f); 

		return iSkipNum + 10;    /* 结构体本身 10 个字节 */
    }

	return 0;
}



/**
 * @brief  计算MP3音频文件的播放时长:通过解析文件头中的比特率（iBitRate）和文件大小（iFileSize）进行估算。
 * 
 * @param  iFlag  为真时表示只读模式（获取音量），为假时表示写入模式（设置音量）
 * @param  strVol: 音量值的字符串表示（如 "50%"）
 * @return int 
 * @note    代码仅适用于固定比特率（CBR）文件
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static int GetPlayTimeForFile(unsigned char *ptDataFrameIndex, int iFileSize){
	unsigned char *pucDataFrame = ptDataFrameIndex;
	int iBitRate = 0;
	int iSampleRate = 0;
	int iPlayTime = 0;
	unsigned char ucChannels = 0;
	struct DataFrameHeader *ptDataFrameHeader;

	// 简单处理，只检测第一个字节是否是0xFF, 找到帧头
	// while(*pucDataFrame != 0xFF){
	// 	if(pucDataFrame >= ptDataFrameIndex + iFileSize){
	// 		// 文件可能已经损坏
	// 		return -1;
	// 	}
	// 	pucDataFrame++;
	// }

	while(!((pucDataFrame[0] == 0xFF) && (pucDataFrame[1] & 0xE0) == 0xE0)){
		if(pucDataFrame >= ptDataFrameIndex + iFileSize - 4){
			// 文件可能已经损坏
			DBG_PRINTF(APP_ERR"File is broken\n");
			return -1;
		}
		pucDataFrame++;
	}


	// 将帧头数据转换成数据帧结构体
	ptDataFrameHeader = (struct DataFrameHeader *)pucDataFrame;

	iBitRate   = g_aiMP3BitRateIndex[ptDataFrameHeader->bzBitRateIndex];
	iSampleRate = g_aiMP3SampleRateIndex[ptDataFrameHeader->bzSampleIndex];
	ucChannels  = ptDataFrameHeader->bzCahnnelMod;
	// 字节总数*比特位/比特率/1000转换成秒
	iPlayTime = iFileSize * 8 / iBitRate / 1000;
	return iPlayTime;
}



// MAD库中用于将解码后的PCM数据转换为标准16位整数
static inline signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}



/*
 * 该回调函数用于输出解码的音频数据. 它在每一帧的数据解码完毕之后被回调
 * 目的是为了输出或者播放 PCM 音频数据
 * 将MAD库解码后的PCM数据转换为16位小端格式，并通过ALSA接口（snd_pcm_writei）输出到音频设备。
 * 支持单声道/立体声自动适配，单声道数据会被复制到左右声道。
 * x86和ARM等主流处理器默认采用小端模式
 */

/**
 * @brief  将MAD库解码后的数据转换为标准PCM格式并播放
 * 
 * @param  data：用户自定义数据（未使用）。
 * @param  header：MP3帧头信息（如采样率、比特率）。
 * @param  pcm：解码后的PCM音频数据。
 * @return MAD_FLOW_CONTINUE（继续解码）或 MAD_FLOW_STOP（终止解码）。
 * @note   
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static enum mad_flow Mp3DataOutput(void *data, struct mad_header const *header, struct mad_pcm *pcm){

    unsigned int nchannels, nsamples;
    mad_fixed_t const *left_ch, *right_ch;		//指向左右声道PCM数据的指针。
    unsigned char aucFrameData[1152*4];			// 存储转换后的16位小端PCM数据（缓冲区大小：1152采样点 × 4字节/采样点）。
	int iFrameDataNum = 0;
	int iError = 0;
	
    /* pcm->samplerate contains the sampling frequency */
	// 从 mad_pcm 结构体中提取声道数、采样点数和左右声道数据指针。
    nchannels = pcm->channels;
    nsamples  = pcm->length;
    left_ch   = pcm->samples[0];
    right_ch  = pcm->samples[1];
	
	// PCM数据处理循环:将MAD定点数转换为16位小端格式，并填充到缓冲区。
	// 调用 scale() 函数将MAD定点数转换为16位有符号整数。将低8位和高8位分别存入缓冲区（小端格式）。
	// 对于单声道数据，将左声道数据拷贝到左右声道数据，并填充到缓冲区。
    while (nsamples--) {
        signed int sample;
     
        /* output sample(s) in 16-bit signed little-endian PCM */
     
        sample = scale(*left_ch++);
        aucFrameData[iFrameDataNum ++] = ((sample >> 0) & 0xff);
        aucFrameData[iFrameDataNum ++] = ((sample >> 8) & 0xff);
   		           
   	    if(nchannels == 2){
            sample = scale(*right_ch++);
            aucFrameData[iFrameDataNum ++] = ((sample >> 0) & 0xff);
            aucFrameData[iFrameDataNum ++] = ((sample >> 8) & 0xff);
        }else if(nchannels == 1){
            /* 如果是单声道就扩充数据 */
            aucFrameData[iFrameDataNum ++] = ((sample >> 0) & 0xff);
   		    aucFrameData[iFrameDataNum ++] = ((sample >> 8) & 0xff); 
   	    }

    }

    /* 传送数据,四个字节为单位 */
	// 通过ALSA接口将PCM数据写入音频设备。
	iError = snd_pcm_writei(g_ptMP3PlaybackHandle, aucFrameData, iFrameDataNum / 4);

	if(g_bMp3DecoderExit){
		return MAD_FLOW_STOP;
	}
  
    return MAD_FLOW_CONTINUE;
}



/**
 * @brief  定义一个静态函数 Mp3GetInput，用于向 MAD 解码库提供 MP3 输入数据流。
 * 
 * @param  data：MP3 数据（实际类型为 struct PcmFmtParams *）。
 * @param  stream：MAD 库的流对象指针，用于管理解码过程中的数据流。
 * @return MAD_FLOW_CONTINUE（继续解码）或 MAD_FLOW_STOP（终止解码）。
 * @note   
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static enum mad_flow Mp3GetInput(void *data, struct mad_stream *stream)
{
	struct PcmFmtParams *ptPcmFmtParams = data;

	// 文件读取完毕或数据已全部加载到解码器。
	if (!ptPcmFmtParams->dwDataLength){
		return MAD_FLOW_STOP;		
	}

	/* 要解码的数据流,因为之前已经映射数据到内存了，所以直接使用此函数即可，可能有别的
	 *  函数用于没有提前映射的数据解码
	 */
	// 将用户提供的 MP3 数据绑定到 MAD 的流对象。
	mad_stream_buffer(stream, ptPcmFmtParams->pucDataStart, ptPcmFmtParams->dwDataLength);

	ptPcmFmtParams->dwDataLength = 0;

	return MAD_FLOW_CONTINUE;
}


/*
 * 错误处理函数 MAD_ERROR_* 错误在 mad.h (or stream.h) 中定义
 	MAD解码库的错误回调函数，用于处理解码过程中的错误。
 */
static enum mad_flow Mp3DecoderError(void *data, struct mad_stream *stream, struct mad_frame *frame){
  struct PcmFmtParams *ptPcmFmtParams = data;

  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - ptPcmFmtParams->pucDataStart);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}



static void MP3Thread1Clean(void *data)
{
	g_iRuntimeRatio = 0;
	g_bMusicHalt = 0;
	pthread_mutex_unlock(&g_tMutex);
	g_bMP3TimeThreadExit = 1;
}

static void *MP3TimeThread(void *data){
	int iTotalPlayTimeUsec = g_iMP3PlayTime * 1000; // 总播放时间（毫秒转微秒
	int iHasPlayTimeUsec = 0;						// 已播放时间（初始为0）
	
	// 设置线程取消状态，允许异步取消请求
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	// 注册清理函数：线程被取消时自动调用 MP3Thread1Clean 释放资源。必须与 pthread_cleanup_pop 配对使用。
	pthread_cleanup_push(MP3Thread1Clean, NULL);
	g_bMP3TimeThreadExit = 0;

	while(iHasPlayTimeUsec < iTotalPlayTimeUsec){
		pthread_testcancel();    /* 线程取消点 */
		// 获取锁并检查线程取消状态
		pthread_mutex_lock(&g_tMutex);
		if(g_bMusicHalt){
			// 等待条件变量，自动释放锁
			pthread_cond_wait(&g_tConVar, &g_tMutex);
		}
		pthread_mutex_unlock(&g_tMutex);

		// 更新播放进度，每50毫秒更新一次。
		g_iRuntimeRatio = (float)iHasPlayTimeUsec / iTotalPlayTimeUsec * 1000;
		usleep(50 * 1000);	//休眠50ms
		iHasPlayTimeUsec += 50;	//累计已播放时间
	}

	
	pthread_exit(NULL);
	pthread_cleanup_pop(0);
}


// 使用 MAD库（MPEG Audio Decoder）进行同步解码（MAD_DECODER_MODE_SYNC）
// 通过回调函数链处理数据流：
// Mp3GetInput：从内存或文件读取MP3数据。
// Mp3DataOutput：将解码后的PCM数据写入音频设备（如ALSA）。
// Mp3DecoderError：处理解码错误（如损坏帧）。
static void *MP3PlayThread(void *data){
	struct mad_decoder decoder;
	int result;

	g_bMp3DecoderExit = 0;    // 0
	g_bMP3PlayThreadExit = 0;

	// 初始化MAD解码器，设置输入、输出、错误回调函数。
	mad_decoder_init(&decoder, data,
		   Mp3GetInput, 0, 0, Mp3DataOutput,
		   Mp3DecoderError, 0);

	// 启动解码， MAD_DECODER_MODE_SYNC表示同步解码（阻塞直到完成）。
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	// 解码结束
	mad_decoder_finish(&decoder);

	// 关闭音频设备
	snd_pcm_close(g_ptMP3PlaybackHandle);
	// 设置线程退出标志
	g_bMP3PlayThreadExit = 1;

	pthread_exit(NULL);
}



/**
 * @brief  启动MP3音乐播放的主函数。
 * 
 * @param  ptFileDesc 包含MP3文件内存映射地址和文件大小。
 * @return int 0 成功   -1 失败
 * @note   
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static int MP3MusicPlay(T_FileMap *ptFileDesc){
	// pucFileMem文件内存起始地址，MP3开头是ID3V2。
	struct IDxVxHeader *ptIDxVxHeader = (struct IDxVxHeader *)ptFileDesc->pucFileMapMem;
	int iError = 0;
	int iSkipSize = 0;
	unsigned char *pucMemStart = ptFileDesc->pucFileMapMem;
	int iRealPcmDataSize = 0;


	// 设置PCM输出格式 初始化音频设备的PCM参数（如采样率、位深、声道数）。
	iError = SetMP3OutPcmFmt();
	if(iError){
		DebugPrint("SetMp3OutPcmFmt error \n");
		return -1;
	}

	/* 有很长一段是歌词等等信息，要跳过 */
	// 跳过文件头元数据
    ptIDxVxHeader = (struct IDxVxHeader *)pucMemStart;
	iSkipSize = SkipIDxVxContents(ptIDxVxHeader);

    /* 结尾有 ID3V1, 长 128 字节 */
	// 计算实际音频数据大小：iSkipSize是跳过的头数据，128是结尾ID3V1
    iRealPcmDataSize = ptFileDesc->iFileSize - iSkipSize - 128;
	// 音频数据开始地址
    pucMemStart = pucMemStart + iSkipSize;

	/* 初始化私有信息结构体，包括要解码的其起始地址以及长度 */
	// 将音频数据地址和长度存入全局变量，供解码线程使用。
	g_tMP3PcmFmtParams.pucDataStart = pucMemStart;
	g_tMP3PcmFmtParams.dwDataLength = iRealPcmDataSize;
	g_tMP3PcmFmtParams.dwDataLengthBak = iRealPcmDataSize;
	g_tMP3PcmFmtParams.pucDataEnd  = g_tMP3PcmFmtParams.pucDataStart + iRealPcmDataSize;
	// 计算播放的总时长
	g_iMP3PlayTime = GetPlayTimeForFile(g_tMP3PcmFmtParams.pucDataStart, iRealPcmDataSize);

	// 创建播放控制线程
	pthread_create(&g_MP3TimeThreadId, NULL, MP3TimeThread, NULL);	//更新播放进度、处理暂停/恢复（通过全局变量 g_bMusicHalt）。
	pthread_create(&g_MP3PlayThreadId, NULL, MP3PlayThread, &g_tMP3PcmFmtParams);	//调用MAD库解码MP3数据，输出PCM到音频设备。

	return 0;
}



/**
 * @brief 音乐播放控制函数，通过不同的控制码（eCtrlCode）执行播放、暂停、退出等操作。
 * 
 * @param  eCtrlCode 控制码
 * @return int 0 成功
 * @note   通过多线程和ALSA库实现音乐播放控制，支持暂停、恢复、退出、音量调节和进度查询。
 * 
 * @author  bia布
 * @date    2025/07/23
 * @version 1.0
 */
static int MP3MusicCtrl(enum MusicCtrlCode eCtrlCode)
{
	char acVolStr[3];	// 用于临时存储音量值的字符串（最大支持两位数）

	switch(eCtrlCode){
		case MUSIC_CTRL_CODE_HALT : {
			snd_pcm_pause(g_ptMP3PlaybackHandle, 1);		// 调用ALSA库的 snd_pcm_pause 暂停音频输出。
			pthread_mutex_lock(&g_tMutex);					// 通过互斥锁保护共享变量 g_bMusicHalt
			g_bMusicHalt = 1;
			pthread_mutex_unlock(&g_tMutex);	

			break;
		}
		case MUSIC_CTRL_CODE_PLAY : {
			snd_pcm_pause(g_ptMP3PlaybackHandle, 0);		// 恢复音频设备
			/* Get lock */
			pthread_mutex_lock(&g_tMutex);
			g_bMusicHalt = 0;
			pthread_cond_signal(&g_tConVar);	// 唤醒等待的线程（MP3TimeThread，处理暂停的进度更新进程）
			pthread_mutex_unlock(&g_tMutex);	

			break;
		}
		case MUSIC_CTRL_CODE_EXIT : {
			/* 有可能线程取消的时候音乐正在暂停，这个时候先恢复播放，确保线程能退出阻塞状态，再通过标志位通知安全退出
			 * 不管有没有暂停歌曲，都先运行歌曲，然后再取消线程
			 * 防止由于阻塞导致程序不响应
			 */
			if(!g_bMP3PlayThreadExit){
				snd_pcm_pause(g_ptMP3PlaybackHandle, 0);	// 确保音频设备未暂停
				g_bMp3DecoderExit = 1;
				pthread_join(g_MP3PlayThreadId, NULL);		// 等待播放线程结束
				snd_pcm_drop(g_ptMP3PlaybackHandle);		// 丢弃音频设备缓存
			}

			if(!g_bMP3TimeThreadExit){
				pthread_cancel(g_MP3TimeThreadId);			// 强制取消时间线程
				pthread_join(g_MP3TimeThreadId, NULL);		// 等待线程终止		
			}
			break;
		}
		case MUSIC_CTRL_CODE_ADD_VOL : {
			g_cVolValue = g_cVolValue > 63 - VOL_CHANGE_NUM ? 63 : g_cVolValue + VOL_CHANGE_NUM;
			sprintf(acVolStr, "%d", g_cVolValue);
			SetVol(acVolStr, 0);
			break;
		}
		case MUSIC_CTRL_CODE_DEC_VOL : {
			g_cVolValue = g_cVolValue < VOL_CHANGE_NUM ? 0 : g_cVolValue - VOL_CHANGE_NUM;
			sprintf(acVolStr, "%d", g_cVolValue);
			SetVol(acVolStr, 0);
			break;
		}
		case MUSIC_CTRL_CODE_GET_VOL : {
			return g_cVolValue;
			break;
		}
		case MUSIC_CTRL_CODE_GET_RUNTIME : {
			return g_iRuntimeRatio;	// 返回全局进度百分比（0-1000）
			break;
		}
		
		default : break;
	}

	return 0;
}


// 定义MP3音频解码结构体，用于向上层注册
T_MusicParser g_tMP3MusicParser = {
	.name = "mp3",
	.MusicDecoderInit = MP3MusicDecoderInit,
	.MusicDecodeExit  = MP3MusicDecodeExit,
	.isSupport = isSupportMP3,
	.MusicPlay = MP3MusicPlay,
	.MusicCtrl = MP3MusicCtrl,
};

int MP3ParserInit(void){
	int iError = 0;
	iError = RegisterMusicParser(&g_tMP3MusicParser);
	iError |= MP3MusicDecoderInit();

	return iError;
}


void MP3ParserExit(void){
	UnregisterMusicParser(&g_tMP3MusicParser);
	MP3MusicDecodeExit();
}
