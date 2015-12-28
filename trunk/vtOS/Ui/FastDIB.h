#ifndef _FASTDIB_H_
#define _FASTDIB_H_

#pragma once

#include "..\..\FacePad\stdafx.h"

#define E_UNSUPORTCOLOR             0x90000001
#define E_LOADIMAGEFAIL             0x90000002
#define E_GETOBJECTFAIL             0x90000003
#define E_FILENOTFOUND              0x90000004
#define E_TYPEMISMATCH              0x90000005
#define E_INVALIDSIZE               0x90000006
#define E_EMPTYOBJECT               0x90000007
#define E_DIBLOCKED                 0x90000008
#define E_NOTINIT                   0x90000009

#define DRAWMODE_IGNOREALPHA        0x0000007F
#define DRAWMODE_FLIPALPHA          0x0000003F
#define DRAWMODE_XADDSATURATION     0x0000001F
#define DRAWMODE_XSUBSATURATION     0x0000000F
#define DRAWMODE_ADDSATURATION      0x00000007
#define DRAWMODE_SUBSATURATION      0x00000003
#define DRAWMODE_NORMAL             0x00000001
#define DRAWMODE_COPY               0x00000000

#define DRAWMODE_FROMBACK           0x00010000
#define DRAWMODE_FLIPSRCCOLOR       0x00100000
#define DRAWMODE_FLIPDSTCOLOR       0x01000000
#define DRAWMODE_ALPHABLEND         0x10000000

#define FDIBTYPE_RGBA               0x00000001
#define FDIBTYPE_8BITBUF            0x00000003
#define FDIBTYPE_16BITBUF           0x00000007
#define FDIBTYPE_DOUBLEBUF          0x0000000F
#define FDIBTYPE_NULLFRONT          0x00010000
#define FDIBTYPE_NULLBACK           0x00020000

#define TM_NONE         0
#define TM_RGB          1
#define TM_ALPHA        2

#define TCM_EQUAL       0
#define TCM_BELONG      1

#define YUV11           0x000129E4
#define YUV12           0x000129E4
#define YUV13           0x000129E4
#define YUV21           0x000001D5
#define YUV22           0x000062EB
#define YUV23           0x000201DF
#define YUV31           0x00019589
#define YUV32           0x0000CF41
#define YUV33           0x0000012F

#define RGB11           0x000041FF
#define RGB12           0x000025FD
#define RGB13           0x000070FF
#define RGB21           0x000080FF
#define RGB22           0x00004B02
#define RGB23           0x00005F00
#define RGB31           0x00001903
#define RGB32           0x000070FF
#define RGB33           0x000011FF

#define YUV_BLACK       0x00108080
#define YUV_WHITE       0x00EB8080
#define YUV_LIGHTGRAY   0x00B58080
#define YUV_DARKGRAY    0x00478080
#define RGB_BLACK       0x00000000
#define RGB_WHITE       0x00FFFFFF

#define _CHECK(hr)                      if (hr != S_OK) AfxMessageBox(CFastDIB::GetErrorString(hr));
#define _SATURATION(min, max, value)    if (value < min) value = min; if (value > max) value = max;
#define _SATURATION255(color)           if (color<0) color = 0; if (color>255) color = 255;

#define _COLORREFTOARGB(color, nAlphaValue)     \
    DWORD XR = (color&0x000000FF)<<16;          \
    DWORD XB = (color&0x00FF0000)>>16;          \
    DWORD XA = nAlphaValue<<24;                 \
    color = (color&0x0000FF00)|XR|XB|XA;

#ifdef _DEBUG
#if 1
#define _CHECKTYPE(pDibInfo, uDibType, uTypeMode)       \
	{                                                   \
		if (uTypeMode == TCM_BELONG) {                  \
			if (pDibInfo->nType != uDibType)          \
				return E_TYPEMISMATCH;                  \
		} else {                                        \
			if ((pDibInfo->nType & uDibType) == 0)    \
				return E_TYPEMISMATCH;                  \
		}                                               \
	}
#else
	#define _CHECKTYPE(pDibInfo, uDibType, uTypeMode)
#endif
#else
#define _CHECKTYPE(pDibInfo, uDibType, uTypeMode)
#endif

typedef UINT FASTDIBTYPE;

typedef union _FASTDIB_COLOR
{
	DWORD dw;
	struct
	{
		unsigned char V;
		unsigned char U;
		unsigned char Y;
		unsigned char T;
	};
	struct
	{
		unsigned char B;
		unsigned char G;
		unsigned char R;
		unsigned char A;
	};
} FASTDIB_COLOR, *PFASTDIB_COLOR;

typedef union _FDIBMATRIX
{
	struct _STRUCTFDIBMATRIX
	{
		DWORD part0;
		DWORD part1;
		DWORD part2;
		DWORD part3;
		DWORD part4;
		DWORD part5;
		DWORD part6;
		DWORD part7;
		DWORD part8;
	} sPart;
	DWORD aPart[3][3];
} FDIBMATRIX, *PFDIBMATRIX;

typedef struct tagFASTDIB
{
	FASTDIBTYPE nType;
	LONG bmWidth;
	LONG bmHeight;
	void *pvData;
} FASTDIB, *PFASTDIB;

typedef struct tagDRAWPARAM
{
	int nXDst;
	int nYDst;
	int nWidth;
	int nHeight;
	int nXSrc;
	int nYSrc;
	int nDstWidth;
	int nDstHeight;
	int nSrcWidth;
	int nSrcHeight;
	DWORD *pDst;
	DWORD *pDst2;
	DWORD *pSrc;
	DWORD nBoardColor;
	BYTE nAlphaValue;
	BYTE nBoardWidth;
	int nShadowHorPos;
	int nShadowVerPos;
	int nShadowHorSize;
	int nShadowVerSize;
	DWORD nShadowColor;
} DRAWPARAM, *PDRAWPARAM;

typedef struct tagCOLORTABLE
{
	DWORD m_part0;
	DWORD m_part1;
	DWORD m_part2;
	DWORD m_part3;
} COLORTABLE, *PCOLORTABLE;

typedef struct tagANGLETABLE
{
	int m_sinpart;
	int m_cospart;
} ANGLETABLE, *PANGLETABLE;

typedef struct tagCOLORTRANSPARENCY
{
	BOOL m_bColorKey;
	BOOL m_bLightKey;
	WORD m_wHue;
	BYTE m_wHueRange;
	BYTE m_nSaturation;
	BYTE m_nBoardBlur;
	BYTE m_nShadowRange;
	BYTE m_nHighLevelDoor;
	BYTE m_nLowLevelDoor;
	BYTE m_nHighLevelBlur;
	BYTE m_nLowLevelBlur;

	BYTE m_nTransparentValue;
} COLORTRANSPARENCY, *PCOLORTRANSPARENCY;

typedef struct tagPAINTPARAM
{
	BYTE m_nMosaicHorizontal;//水平马赛克
	BYTE m_nMosaicVertical;  //垂直马赛克
	BOOL m_bFlipColor;       //色彩反转
	BOOL m_bFlipLight;       //亮度反转
	BOOL m_bReverse;         //反相
	BOOL m_bColorMask;       //是否蒙色
	DWORD m_dwColorMask;     //画面蒙色
	BYTE m_nLightPlus;       //亮度增益
	WORD m_nLightOffset;     //亮度偏移
	BYTE m_nColorPlus;       //色度增益
	WORD m_nColorOffset;     //色度偏移
	BYTE m_nLightRange;      //亮度分量
	BYTE m_nColorRange;      //色度分量
	BOOL m_bNoise;           //杂色
} PAINTPARAM, *PPAINTPARAM;

class CFastDIB
{
public:
	CFastDIB();
	virtual ~CFastDIB();

private:
	FASTDIB m_DibInfo;
	FASTDIB m_DibInfoBack;
    BOOL    m_bLocked;

private:
	static BOOL  m_bInited;
	static DWORD m_nMaxHeight;
	static DWORD m_nMaxWidth;
	static COLORTABLE m_ct[32][32];
	static ANGLETABLE m_at[3600];
	static DWORD m_hslTable[262144];
	static int *m_mapx;
	static int *m_offsetx;

public:
	static void Initialize(DWORD nMaxWidth, DWORD nMaxHeight);
	static void UnInitialize(void);
    static HRESULT Finalization(void);

	static FASTDIB_COLOR YUVTORGB(FASTDIB_COLOR paramYUV);
	static FASTDIB_COLOR RGBTOYUV(FASTDIB_COLOR paramRGB);
	static HRESULT InitPaintParam(PAINTPARAM *pppm);

	static HRESULT WaterFX(CFastDIB *pCurrentBuf, CFastDIB *pNextBuf);
	static HRESULT WaterFX16(CFastDIB *pCurrentBuf, CFastDIB *pNextBuf);
    static void memcpy2(void *lpvDst, void *lpvSrc, DWORD cpySize) {};
	static CString GetErrorString(HRESULT hErrorCode);

    BOOL IsInited(void) { return m_bInited; };
    BOOL IsValid(void)  { return (m_DibInfo.pvData != NULL); };
    void FreeDIB(void);

	HRESULT LoadFromDIB(CFastDIB *pSrcDib);
	HRESULT LoadFromDIBEx(CFastDIB *pSrcDib);
	HRESULT LoadFromHBitmap(HBITMAP hSrcBitmap);
	HRESULT LoadFromBuf(FASTDIB *lpDibInfo, FASTDIB *lpDibInfoBack);
	HRESULT LoadFromRes(UINT uIDResource);
	HRESULT LoadFromRes(LPCTSTR lpszResourceName);
	HRESULT LoadFromFile(LPCTSTR lpszFileName);

	HRESULT ActivateBuf(int x, int y, BYTE nEnerge);
	HRESULT ActivateBuf16(int x, int y, int r, WORD nEnerge);
	HRESULT ClearImage(void);
	HRESULT CreateDIB(DWORD dwWidth, DWORD dwHeight, FASTDIBTYPE fdt);
	HRESULT CreateCompatibleDIB(CFastDIB *pDIB);
	HRESULT DistortByDIB(int x, int y, CFastDIB *pDIB);
	HRESULT DistortByDIB16(int x, int y, CFastDIB *pDIB);

    HRESULT FlipBuffer(void);
	HRESULT Fade(BYTE nThreshold);

	HRESULT Stretch(double fxtime, double fytime, DWORD x, DWORD y, DWORD nWidth, DWORD nHeight, BOOL bBlur = TRUE);
	HRESULT Rotation(int nangle, int cx, int cy, DWORD nWidth, DWORD nHeight, BOOL bBlur = TRUE);
	HRESULT BitmapWipe(CFastDIB *pBufDIB, BYTE nCurrentTime, BYTE nBoardWidth = 0, BYTE nBoardWidthColor = 0,
		BOOL bColorBoard=FALSE, DWORD nBoardColor=RGB(0, 0, 0),
		DWORD x = 0, DWORD y = 0);
	HRESULT SyncDoubleBuffer(void);
	HRESULT ColorTransparency(PCOLORTRANSPARENCY pctr);
	HRESULT Paint(PPAINTPARAM pppm);
	HRESULT QuickPaint(PPAINTPARAM pppm);
	HRESULT FastBlt(HDC hdc, int nXDst = 0, int nYDst = 0, int Width = 0, int Height = 0, int nXSrc = 0, int nYSrc = 0);
	HRESULT Draw(CFastDIB *pDstDIB, UINT uDrawMode = DRAWMODE_COPY,
		int nXDst = 0, int nYDst = 0, int nWidth = 0, int nHeight = 0, int nXSrc = 0, int nYSrc = 0,
		BYTE nAlphaValue = 0xFF, BYTE nBoardWidth = 0, DWORD nBoardColor = 0);

	FASTDIB *GetDibInfo(void) { return &m_DibInfo; };
	FASTDIB *GetBackDibInfo(void) { return &m_DibInfoBack; };

private:
	void _DrawCopy(DRAWPARAM *dpm);

	void _DrawIgnoreAlpha(DRAWPARAM *pdpm);
	void _DrawNormal(DRAWPARAM *pdpm);
	void _DrawFlipAlpha(DRAWPARAM *pdpm);

	void _DrawAddSaturation(DRAWPARAM *pdpm);
	void _DrawAddSaturationFlipAlpha(DRAWPARAM *pdpm);

	void _DrawSubSaturation(DRAWPARAM *pdpm);
	void _DrawSubSaturationFlipAlpha(DRAWPARAM *pdpm);

	void _MatrixTrans(FDIBMATRIX *pfdMatrix1, FDIBMATRIX fdMatrix2);
	void _MatrixTrans(CPoint *pcpCoord, FDIBMATRIX fdMatrix);    
};

#endif	// _FASTDIB_H_
