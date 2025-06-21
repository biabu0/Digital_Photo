
#ifndef _FONTS_MANAGER_H
#define _FONTS_MANAGER_H

typedef struct FontBitMap {
	int iXLeft;
	int iYTop;
	int iXMax;
	int iYMax;
	int iBpp;		// 婵炶揪绲界粔鏉懨瑰Ο鑽も枖妞ゆ挾濮甸悾閬嶆煕瀹ュ懐绠崇紒鈧畝鈧幃浼村Ω瑜嬫禒娑㈡煛閸屾繍娼愮痪顓炵埣閺佸秶浠﹂悡搴樻瀼闂佺儵鍋撻崝瀣博鐎涙ɑ濯寸€广儱鐗勯埀顒€鍟扮划鍫熸姜閹殿噮浼囨繛鎴炴惄娴滄粓宕曞杈潟闁绘ǹ娅ｇ粻鐑芥煛閸曢潧鐏熺紒鏂跨摠缁嬪顢旈崟顓熸喕闂佺厧鎼崐浠嬪Υ閸愵亞鐭嗛弶鐐村椤忓崬鈽夐幙鍐х盎闁稿孩宀搁弫鎾绘晸閿燂拷?
	int iPitch;   	// 闁荤姵浜介崝宀€鈧濞婇弫宥囦沪缁涘娈告繛瀛樼矊妤犵ǹ鐣烽悢鍏煎殞闂婎偒鍘剧粔鎾煕閵夛附瀚曠紒杈ㄧ箖缁嬪濡堕崼顐ｆ杸闂佺ǹ绉寸换鎺旂矆鐏炲墽鈻曢悗锝庡枟閿涚喖鏌ｉ妸銉ヮ伂婵炲娲熼弫鎾绘晸閿燂拷?,闁荤偞绋忛崝搴ㄥΦ濮樿泛鐏抽悘鐐舵缁€瀣煠绾懎绱︾紓宥呮嚇瀹曞爼鏌ㄩ妤€浜鹃悘鐐跺閸橆剟鏌ｉ埡鍌涘€愰柛瀣堕檮缁嬪濡堕崼顐ｆ杸闂佺ǹ绉寸换鎺旂矆瀹€鍕瀬闁绘鐗嗙粊锕傛煕閿斿搫濡奸柛鐐差嚟閳ь剚绋掔湁闁煎灚鍨块幆鍐礋閵娿垹浜鹃悘鐐垫櫕閹界喖鏌ら崫鍕偓鍛婄閸濄儳鐭撳┑鐘宠壘濞咃拷
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
	void (*SetFontSize) (unsigned int dwFontSize);
	struct FontOpr *ptNext;
}T_FontOpr, *PT_FontOpr;


int RegisterFontOpr(PT_FontOpr ptFontOpr);
void ShowFontOpr(void);
int FontsInit(void);
int FreeTypeInit(void);
PT_FontOpr GetFontOpr(char *pcName);
int GetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
int SetFontsDetail(char *pcFontsName, char *pcFontsFile, unsigned int dwFontSize);
void SetFontSize(unsigned int dwFontSize);
#endif /* _FONTS_MANAGER_H */

