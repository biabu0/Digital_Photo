
#ifndef _FONTS_MANAGER_H
#define _FONTS_MANAGER_H

typedef struct FontBitMap {
	int iXLeft;
	int iYTop;
	int iXMax;
	int iYMax;
	int iBpp;		// 娴ｅ秴娴樻稉顓犳畱閸嶅繒绀岀悰銊с仛閺傝纭堕敍灞煎▏閻€劋绔存担宥堛€冪粈杞扮娑擃亜鍎氱槐鐘虹箷閺勵垯绔存稉顏勭摟閼哄倽銆冪粈杞扮娑擃亜鍎氶敓锟�?
	int iPitch;   	// 鐠恒劌瀹抽敍灞筋嚠娴滃骸宕熼懝韫秴閸ユ拝绱濇稉銈堫攽閸嶅繒绀屾稊瀣？閻ㄥ嫯娉曢敓锟�?,鐞涖劎銇氶垾灞藉礋閼硅弓缍呴崶閿偓灞艰厬閻╂悂鍋︽稉銈堫攽閸嶅繒绀岄弫鐗堝祦閸︺劌鍞寸€涙ü鑵戦惃鍕ㄢ偓灞界摟閼哄倸浜哥粔濠氬櫤
	int iCurOriginX;
	int iCurOriginY;
	int iNextOriginX;
	int iNextOriginY;
	unsigned char *pucBuffer;
}T_FontBitMap, *PT_FontBitMap;

typedef struct FontOpr {
	char *name;
	int (*FontInit)(char *pcFontFile, unsigned int dwFontSize);
	int (*GetFontBitmap)(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
	struct FontOpr *ptNext;
}T_FontOpr, *PT_FontOpr;


int RegisterFontOpr(PT_FontOpr ptFontOpr);
void ShowFontOpr(void);
int FontsInit(void);
int FreeTypeInit(void);
PT_FontOpr GetFontOpr(char *pcName);
int GetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
int SetFontsDetail(char *pcFontsName, char *pcFontsFile, unsigned int dwFontSize);

#endif /* _FONTS_MANAGER_H */

