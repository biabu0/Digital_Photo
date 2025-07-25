
#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>
#include<debug_manager.h>

#define FB_DEVICE_NAME "/dev/fb0"

#define COLOR_BACKGROUND   0xE7DBB5  /* 泛黄纸张 */
#define COLOR_FOREGROUND   0x514438  /* 褐色字体 */



#define BRONZES_COLOR    0xA62AA2
#define DEEP_BROWN_COLOR 0x5A4033
#define PALE_TURQUOISE_2 0xAEEEEE
#define PALE_TURQUOISE_3 0x96CDCD
#define PALE_TURQUOISE_4 0x668B8B
#define CADET_BLUE_1     0x98F5FF
#define CADET_BLUE_2     0x8EE5EE
#define CADET_BLUE_3     0x7AC5CD
#define LIGHT_GRAY       0xD3D3D3
#define MISTYROSE        0xFFE4E1
#define TAN4             0x8B5A2B
#define BURLYWOOD4       0x8B7355
#define DARKSLATEGRAY    0x2F4F4F

#define CONFIG_FONT_COLOR        0x6495ED
#define CONFIG_BACKGROUND_COLOR  DEEP_BROWN_COLOR
#define CONFIG_MUSIC_BG_COLOR    DARKSLATEGRAY
#define CONFIG_PROGRESS_BG_COLOR PALE_TURQUOISE_4
#define CONFIG_PROGRESS_COLOR    CADET_BLUE_2



// 文件名称长度
#define FILE_NAME_SIZE 256

#define DEFAULT_DEBUGLEVEL 7

#define APP_EMERG "<0>"
#define APP_ALERT "<1>"
#define APP_CRIT "<2>"
#define APP_ERR "<3>"
#define APP_WARNING "<4>"
#define APP_NOTICE "<5>"
#define APP_INFO "<6>"
#define APP_DEBUG "<7>"


//#define DBG_PRINTF(...)  
#define DBG_PRINTF DebugPrint
//#define DBG_PRINTF printf

#define ICON_PATH "/etc/digitpic/pic"

#define DEFAULT_DIR "/"

#define DEFAULT_DIR_PIC "/etc/picture/big_pic"


#endif /* _CONFIG_H */
