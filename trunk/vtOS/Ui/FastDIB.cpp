//////////////////////////////////////////////////////////////
//
//

#include "..\..\FacePad\stdafx.h"
#include <shlwapi.h>
#include <math.h>
#include <Vfw.h>
#include "FastDIB.h"

#pragma comment(lib, "Vfw32.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

DWORD       CFastDIB::m_nMaxWidth  = 0;
DWORD       CFastDIB::m_nMaxHeight = 0;
BOOL        CFastDIB::m_bInited = FALSE;
COLORTABLE  CFastDIB::m_ct[32][32];
ANGLETABLE  CFastDIB::m_at[3600];
DWORD       CFastDIB::m_hslTable[262144];
int        *CFastDIB::m_mapx = NULL;
int        *CFastDIB::m_offsetx = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void MMXFastFill(void *pvBuf, DWORD size, BYTE Value)
{
	DWORD dwBufAddress = (DWORD)pvBuf;
	__asm {
		mov EAX, 1
	}
	/*
		push edi
		mov edi dwBufAddress
		mov ecx size
		test ecx ecx
		jz @exit
		xor eax eax
		mov al Value

		movd mm0 eax
		punpcklbw mm0 mm0
		punpcklbw mm0 mm0
		punpcklbw mm0 mm0

		mov edx ecx
		shr edx 3 //store 16 bytes in one loop
		and ecx 0xf
		jz @faststore

		@slowstore:
		rep stosb

		@faststore:
		movntq [edi] mm0
		movntq [edi+8] mm0
		movntq [edi+16] mm0
		movntq [edi+24] mm0


		add edi 32
		dec edx
		jnz @faststore

		@exit:
		emms
		pop edi
	}
	//*/
}

CFastDIB::CFastDIB(void)
{
	ZeroMemory(&m_DibInfo, sizeof(FASTDIB));
	ZeroMemory(&m_DibInfoBack, sizeof(FASTDIB));
	m_bLocked = FALSE;
}

CFastDIB::~CFastDIB(void)
{
	if ((m_DibInfo.nType & FDIBTYPE_NULLFRONT) != FDIBTYPE_NULLFRONT) {
		if (m_DibInfo.pvData != NULL)
			delete[] m_DibInfo.pvData;
	}
	if ((m_DibInfo.nType & FDIBTYPE_NULLBACK) != FDIBTYPE_NULLBACK) {
		if (m_DibInfoBack.pvData != NULL)
			delete[] m_DibInfoBack.pvData;
	}
}

void CFastDIB::FreeDIB(void)
{
	if (m_DibInfo.pvData != NULL)
		delete[] m_DibInfo.pvData;
	if (m_DibInfoBack.pvData != NULL)
		delete[] m_DibInfoBack.pvData;
	ZeroMemory(&m_DibInfo, sizeof(FASTDIB));
	ZeroMemory(&m_DibInfoBack, sizeof(FASTDIB));
}

void CFastDIB::Initialize(DWORD nMaxWidth, DWORD nMaxHeight)
{
	if (m_bInited)
		return;

	CFastDIB::m_nMaxWidth  = nMaxWidth;
	CFastDIB::m_nMaxHeight = nMaxHeight;

	CFastDIB::m_mapx = new int[nMaxWidth];
	CFastDIB::m_offsetx = new int[nMaxWidth];

	int i, j;
	for (i = 0; i<32; i++)
	{
		for (j = 0; j<32; j++)
		{
			CFastDIB::m_ct[i][j].m_part0 = (DWORD)(((31-i) * (31-j) / (float)(31 * 31)) * 128);
			CFastDIB::m_ct[i][j].m_part1 = (DWORD)((i * (31-j) / (float)(31 * 31)) * 128);
			CFastDIB::m_ct[i][j].m_part2 = (DWORD)((i * j / (float)(31 * 31)) * 128);
			CFastDIB::m_ct[i][j].m_part3 = 128 - CFastDIB::m_ct[i][j].m_part0 - CFastDIB::m_ct[i][j].m_part1 - CFastDIB::m_ct[i][j].m_part2;
		}
	}

	double tmpf = 3.1415926535 / 180.0;
	for (i = 0; i<3600; i++)
	{
		CFastDIB::m_at[i].m_sinpart = (int)(sin((i/10.0)*tmpf)*1024);
		CFastDIB::m_at[i].m_cospart = (int)(cos((i/10.0)*tmpf)*1024);
	}

	for (i = 0; i<262144; i++)
	{
		DWORD ncolor = i;
		BYTE B=(BYTE)(ncolor & 0x3F);
		BYTE G=(BYTE)((ncolor >>  6) & 0x3F);
		BYTE R=(BYTE)((ncolor >> 12) & 0x3F);
		BYTE L = 0;
		BYTE S = 0;
		short H = 0;

		BYTE nMax = R;
		if (nMax < G)
			nMax = G;
		if (nMax < B)
			nMax = B;
		BYTE nMin = R;
		if (nMin > G)
			nMin = G;
		if (nMin > B)
			nMin = B;
		if (nMax == nMin)
		{
			H = 0;
			S = 0;
			L = 0;
		}
		else
		{
			L = (BYTE)(((nMax + nMin)/126.0) * 240);
			S = 0;
			H = 0;
			if (L < 120)
				S = (BYTE)(((nMax-nMin)/(float)(nMax+nMin)) * 240);
			else
				S = (BYTE)(((nMax-nMin)/(float)(126-nMax-nMin)) * 240);

			if (R == nMax)
				H = (short)((G-B) * 60/(float)(nMax-nMin));
			else
			{
				if (G == nMax)
					H = (short)((2 + (B-R)/(float)(nMax-nMin)) * 60);
				else if (B == nMax)
					H = (short)((4 + (R-G)/(float)(nMax-nMin)) * 60);
			}
			if (H < 0)
				H += 360;
			CFastDIB::m_hslTable[i] = (L << 24) | (S << 16) | (H);
		}
	}

	m_bInited = TRUE;
}

void CFastDIB::UnInitialize(void)
{
	if (m_bInited) {
		 if (CFastDIB::m_mapx) {
			 delete[] CFastDIB::m_mapx;
			 CFastDIB::m_mapx = NULL;
		 }
		 if (CFastDIB::m_offsetx) {
			 delete[] CFastDIB::m_offsetx;
			 CFastDIB::m_offsetx = NULL;
		 }
	}
}

HRESULT CFastDIB::Finalization(void)
{
	if (!m_bInited)
		return E_NOTINIT;

    if (CFastDIB::m_mapx) {
	    delete[] CFastDIB::m_mapx;
        CFastDIB::m_mapx = NULL;
    }
    if (CFastDIB::m_offsetx) {
	    delete[] CFastDIB::m_offsetx;
        CFastDIB::m_offsetx = NULL;
    }
	return S_OK;
}

FASTDIB_COLOR CFastDIB::YUVTORGB(FASTDIB_COLOR paramYUV)
{
	/*
	R = Y+1.14V
	G = Y-0.396U-0.581V
	B = Y+2.029U
	//*/
	FASTDIB_COLOR rc;
	rc.dw = 0;
	if ((paramYUV.dw & 0x00FFFFFF) == YUV_BLACK)
	{
		rc.dw = 0;
		return rc;
	}

	if ((paramYUV.dw & 0x00FFFFFF) == YUV_WHITE)
	{
		rc.dw = 0;
		return rc;
	}
	int sYUVtemp[3] = {0, 0, 0};
	sYUVtemp[0] = paramYUV.Y - 16;
	sYUVtemp[1] = paramYUV.U - 128;
	sYUVtemp[2] = paramYUV.V - 128;
	int sign[3] = {0, 0, 0};
	for (int i = 0; i<3; i++)
	{
		if ((sYUVtemp[i] >> 15) > 0)
		{
			sign[i] = -1;
			sYUVtemp[i] = (~sYUVtemp[i]) + 1;
		}
		else
			sign[i] = 1;
	};

	int R = (sign[0]*sYUVtemp[0]*YUV11-sign[1]*sYUVtemp[1]*YUV21
		+sign[2]*sYUVtemp[2]*YUV31-(1<<15)) >> 16;

	int G = (sign[0]*sYUVtemp[0]*YUV12-sign[1]*sYUVtemp[1]*YUV22
		-sign[2]*sYUVtemp[2]*YUV32-(1<<15)) >> 16;

	int B = (sign[0]*sYUVtemp[0]*YUV13+sign[1]*sYUVtemp[1]*YUV23
		-sign[2]*sYUVtemp[2]*YUV33-(1<<15)) >> 16;

	// limit the RGB value to 0-255
	_SATURATION(0, 255, R);
	_SATURATION(0, 255, G);
	_SATURATION(0, 255, B);

	rc.R = R;
	rc.G = G;
	rc.B = B;

	return rc;
}

FASTDIB_COLOR CFastDIB::RGBTOYUV(FASTDIB_COLOR paramRGB)
{
	/*
	Y = 0.299R+0.587G+0.114B
	U = -0.147R-0.289G+0.436B
	V = 0.615R-0.515G-0.1B
	//*/
	FASTDIB_COLOR rc;
	rc.dw = 0;
	if ((paramRGB.dw & 0x00FFFFFF)==RGB_BLACK)
	{
		rc.dw=YUV_BLACK;
		rc.T=paramRGB.A;
		return rc;
	};

	if ((paramRGB.dw & 0x00FFFFFF)==RGB_WHITE)
	{
		rc.dw=YUV_WHITE;
		rc.T=paramRGB.A;
		return rc;
	};

	int R = paramRGB.R;
	int G = paramRGB.G;
	int B = paramRGB.B;
	int T = 0;
	int Y = (((R*RGB11+G*RGB21+B*RGB31+(1<<7))>>8)+(16<<8)+(1<<7))>>8;
	int U = (((B*RGB32-R*RGB12-G*RGB22+(1<<7))>>8)+(128<<8)+(1<<7))>>8;
	int V = (((R*RGB13-G*RGB23-B*RGB33+(1<<7))>>8)+(128<<8)+(1<<7))>>8;

	_SATURATION(16, 235, Y);
	_SATURATION(16, 240, U);
	_SATURATION(16, 240, V);

	rc.Y = Y;
	rc.U = U;
	rc.V = V;

	return rc;
}

HRESULT CFastDIB::InitPaintParam(PAINTPARAM *pppm)
{
	if (pppm == NULL)
		return E_EMPTYOBJECT;

	ZeroMemory(pppm, sizeof(PAINTPARAM));
	pppm->m_nColorOffset = 255;
	pppm->m_nColorPlus = 64;
	pppm->m_nLightOffset = 255;
	pppm->m_nLightPlus = 64;

	return S_OK;
}

HRESULT CFastDIB::FlipBuffer()
{
	_CHECKTYPE(GetDibInfo(), FDIBTYPE_DOUBLEBUF, TCM_BELONG);
	void *p = m_DibInfo.pvData;
	m_DibInfo.pvData = m_DibInfoBack.pvData;
	m_DibInfoBack.pvData = p;
	return S_OK;
}

void CFastDIB::_MatrixTrans(CPoint *pcpCoord, FDIBMATRIX fdMatrix)
{
	DWORD tmpx = pcpCoord->x;
	DWORD tmpy = pcpCoord->y;
	pcpCoord->x = tmpx*fdMatrix.aPart[0][0]+tmpy*fdMatrix.aPart[1][0]+fdMatrix.aPart[2][0];
	pcpCoord->y = tmpx*fdMatrix.aPart[0][1]+tmpy*fdMatrix.aPart[1][1]+fdMatrix.aPart[2][1];
}

void CFastDIB::_MatrixTrans(FDIBMATRIX *pfdMatrix1, FDIBMATRIX fdMatrix2)
{
	FDIBMATRIX tmpfdMatrix;
	ZeroMemory(&tmpfdMatrix, sizeof(FDIBMATRIX));
	memcpy(&tmpfdMatrix, pfdMatrix1, sizeof(FDIBMATRIX));
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<3; j++) {
			pfdMatrix1->aPart[i][j] = tmpfdMatrix.aPart[i][0] * fdMatrix2.aPart[0][j] +
				tmpfdMatrix.aPart[i][1] * fdMatrix2.aPart[1][j] +
				tmpfdMatrix.aPart[i][2] * fdMatrix2.aPart[2][j];
		}
	}
}

CString CFastDIB::GetErrorString(HRESULT hErrorCode)
{
	switch (hErrorCode)
	{
		case 0x90000001: return _T("unSuport color mode");
		case 0x90000002: return _T("loadimage failure");
		case 0x90000003: return _T("getobject failure");
		case 0x90000004: return _T("file not found");
		case 0x90000005: return _T("type mismatch");
		case 0x90000006: return _T("invalid size");
		case 0x90000007: return _T("empty object");
		case 0x90000008: return _T("dib locked");
		case 0x90000009: return _T("not inited");
		case E_FAIL:     return _T("E_FAIL");
		default:         return _T("UnExpected Error");
	}
}

HRESULT CFastDIB::LoadFromRes(UINT uIDResource)
{
	if (uIDResource == 0)
		return NULL;

	HBITMAP hBitmap = (HBITMAP)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(uIDResource), IMAGE_BITMAP,
		0, 0, LR_CREATEDIBSECTION);

	//DWORD dwError = GetLastError();

	/////////////清除原有信息//////////////
	return LoadFromHBitmap(hBitmap);
}

HRESULT CFastDIB::LoadFromRes(LPCTSTR lpszResourceName)
{
	if (lpszResourceName == NULL)
		return NULL;

	HBITMAP hBitmap = (HBITMAP)LoadImage(AfxGetApp()->m_hInstance, lpszResourceName, IMAGE_BITMAP,
		0, 0, LR_CREATEDIBSECTION);

	//DWORD dwError = GetLastError();

	/////////////清除原有信息//////////////
	return LoadFromHBitmap(hBitmap);
}

HRESULT CFastDIB::LoadFromFile(LPCTSTR lpszFileName)
{
	if (!PathFileExists(lpszFileName))
		return E_FILENOTFOUND;

	/////////////清除原有信息//////////////
	return LoadFromHBitmap((HBITMAP)LoadImage(NULL, lpszFileName, IMAGE_BITMAP,
		0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION));
}

HRESULT CFastDIB::LoadFromDIB(CFastDIB *pSrcDib)
{
	if (pSrcDib == NULL)
		return E_FAIL;

	_CHECKTYPE(pSrcDib->GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);
	if ((pSrcDib->GetDibInfo()->nType == FDIBTYPE_DOUBLEBUF) && (!CFastDIB::m_bInited))
		return E_NOTINIT;

	if (!(pSrcDib->GetDibInfo()->nType == FDIBTYPE_DOUBLEBUF))
	{
		if (m_DibInfo.bmHeight * m_DibInfo.bmWidth < (pSrcDib->GetDibInfo()->bmHeight * pSrcDib->GetDibInfo()->bmWidth))
		{
			if (m_DibInfo.pvData != NULL)
				delete[] m_DibInfo.pvData;
			m_DibInfo.pvData = new DWORD[pSrcDib->GetDibInfo()->bmHeight * pSrcDib->GetDibInfo()->bmWidth];
		}
		if (pSrcDib->GetBackDibInfo()->pvData != NULL)
		{
			delete[] pSrcDib->GetBackDibInfo()->pvData;
			pSrcDib->GetBackDibInfo()->pvData = NULL;
		}
		memcpy(m_DibInfo.pvData, pSrcDib->GetDibInfo()->pvData, (pSrcDib->GetDibInfo()->bmHeight) * (pSrcDib->GetDibInfo()->bmWidth) * 4);
	}
	else
	{
		if (m_DibInfo.pvData == NULL)
			m_DibInfo.pvData = new DWORD[CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight];
		if (m_DibInfoBack.pvData == NULL)
			m_DibInfoBack.pvData = new DWORD[CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight];
		memcpy(m_DibInfo.pvData, pSrcDib->GetBackDibInfo()->pvData, (pSrcDib->GetDibInfo()->bmHeight) * (pSrcDib->GetDibInfo()->bmWidth) * 4);
		memcpy(m_DibInfoBack.pvData, pSrcDib->GetBackDibInfo()->pvData, (pSrcDib->GetDibInfo()->bmHeight) * (pSrcDib->GetDibInfo()->bmWidth) * 4);
	}
    m_DibInfo.bmWidth  = pSrcDib->GetDibInfo()->bmWidth;
	m_DibInfo.bmHeight = pSrcDib->GetDibInfo()->bmHeight;
	m_DibInfoBack.bmWidth  = m_DibInfo.bmWidth;
	m_DibInfoBack.bmHeight = m_DibInfo.bmHeight;
	m_DibInfo.nType = pSrcDib->GetDibInfo()->nType;

	return S_OK;
}

HRESULT CFastDIB::LoadFromDIBEx(CFastDIB *pSrcDib)
{
	if (pSrcDib == NULL)
		return E_FAIL;

	_CHECKTYPE(pSrcDib->GetDibInfo(), FDIBTYPE_RGBA, TCM_BELONG);
	_CHECKTYPE(GetDibInfo(), FDIBTYPE_DOUBLEBUF, TCM_BELONG);

    m_DibInfo.bmWidth  = pSrcDib->GetDibInfo()->bmWidth;
	m_DibInfo.bmHeight = pSrcDib->GetDibInfo()->bmHeight;
	m_DibInfoBack.bmWidth  = m_DibInfo.bmWidth;
	m_DibInfoBack.bmHeight = m_DibInfo.bmHeight;

	memcpy(m_DibInfo.pvData, pSrcDib->GetDibInfo()->pvData,
        CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight * 4);
	memcpy(m_DibInfoBack.pvData, pSrcDib->GetDibInfo()->pvData,
        CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight * 4);

	return S_OK;
}

HRESULT CFastDIB::LoadFromHBitmap(HBITMAP hSrcBitmap)
{
	if ((m_DibInfo.nType == FDIBTYPE_DOUBLEBUF) && (!CFastDIB::m_bInited))
		return E_NOTINIT;
	if (hSrcBitmap == NULL)
		return E_FAIL;

	BITMAP tmpBitmap;
	if (GetObject(hSrcBitmap, sizeof(BITMAP), &tmpBitmap) != 0)
	{
		if ((tmpBitmap.bmBitsPixel != 32) && (tmpBitmap.bmBitsPixel != 8))
			return E_UNSUPORTCOLOR;
		if ((tmpBitmap.bmBitsPixel == 8) && (m_DibInfo.nType == FDIBTYPE_DOUBLEBUF))
			return E_UNSUPORTCOLOR;

		if (m_DibInfo.nType != FDIBTYPE_DOUBLEBUF)
		{
			int bitsPixel = 0;
			int bitsPixelOld = 0;
			if (m_DibInfo.nType == FDIBTYPE_RGBA)
				bitsPixelOld = 32;
			else
				bitsPixelOld = 8;
			if (tmpBitmap.bmBitsPixel == 32)
			{
				bitsPixel = 32;
				if (m_DibInfo.bmWidth * m_DibInfo.bmHeight * bitsPixelOld
                    < tmpBitmap.bmWidth * tmpBitmap.bmHeight * bitsPixel)
				{
					delete[] m_DibInfo.pvData;
					m_DibInfo.pvData = new DWORD[tmpBitmap.bmWidth * tmpBitmap.bmHeight];
				}
				m_DibInfo.nType = FDIBTYPE_RGBA;
				memcpy(m_DibInfo.pvData, tmpBitmap.bmBits, tmpBitmap.bmWidthBytes * tmpBitmap.bmHeight);
			}
			else if (tmpBitmap.bmBitsPixel == 8)
			{
				bitsPixel = 8;
				if (m_DibInfo.bmWidth * m_DibInfo.bmHeight * bitsPixelOld
                    < tmpBitmap.bmWidth * tmpBitmap.bmHeight * bitsPixel)
				{
					delete[] m_DibInfo.pvData;
					m_DibInfo.pvData = new BYTE[tmpBitmap.bmWidth * tmpBitmap.bmHeight];
				}
				BYTE *pDst = (BYTE*)m_DibInfo.pvData;
				BYTE *pSrc = (BYTE*)tmpBitmap.bmBits;
				m_DibInfo.nType = FDIBTYPE_8BITBUF;

				int BytesPerScanline = 0;
				if ((tmpBitmap.bmWidthBytes % 4) == 0)
					BytesPerScanline = tmpBitmap.bmWidthBytes;
				else
					BytesPerScanline = tmpBitmap.bmWidthBytes + 2;

				for (int j = 0; j<tmpBitmap.bmHeight; j++)
				{
					memcpy(pDst, pSrc, tmpBitmap.bmWidth);
					pDst += tmpBitmap.bmWidth;
					pSrc += BytesPerScanline;
				}
			}
		}
		else
		{
			memcpy(m_DibInfo.pvData, tmpBitmap.bmBits, tmpBitmap.bmWidthBytes * tmpBitmap.bmHeight);
			memcpy(m_DibInfoBack.pvData, tmpBitmap.bmBits, tmpBitmap.bmWidthBytes * tmpBitmap.bmHeight);
		}

        m_DibInfo.bmWidth  = tmpBitmap.bmWidth;
		m_DibInfo.bmHeight = tmpBitmap.bmHeight;
		m_DibInfoBack.bmWidth  = m_DibInfo.bmWidth;
		m_DibInfoBack.bmHeight = m_DibInfo.bmHeight;

		return S_OK;
	}
	else
		return E_FAIL;
}

HRESULT CFastDIB::LoadFromBuf(FASTDIB *lpDibInfo, FASTDIB *lpDibInfoBack)
{
	memcpy(GetDibInfo(), lpDibInfo, sizeof(FASTDIB));
	memcpy(GetBackDibInfo(), lpDibInfoBack, sizeof(FASTDIB));
	return S_OK;
}

HRESULT CFastDIB::FastBlt(HDC hdc, int nXDst, int nYDst, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);
	if (hdc == NULL)
		return E_FAIL;

	if (nWidth == 0)  nWidth  = m_DibInfo.bmWidth;
	if (nHeight == 0) nHeight = m_DibInfo.bmHeight;

	BOOL bResult;

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = m_DibInfo.bmWidth;    //Width;
	bmi.bmiHeader.biHeight      = m_DibInfo.bmHeight;   //Height;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage   = nWidth * nHeight * 4;

	/*
	// 方法一
	void *pvBits = NULL;
	HDC hTmpDC = CreateCompatibleDC(hdc);
	if (hTmpDC)
	{
		HBITMAP hTmpBitmap = CreateCompatibleBitmap(hdc, m_DibInfo.m_bmWidth, m_DibInfo.m_bmHeight);
		if (hTmpBitmap)
		{
			bResult = SetDIBits(hTmpDC, hTmpBitmap, 0, m_DibInfo.m_bmHeight, m_DibInfo.m_pvData, &bmi, DIB_RGB_COLORS);

			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hTmpDC, hTmpBitmap);
			BitBlt(hdc, nXDst, nYDst, Width, Height, hTmpDC, nXSrc, nYSrc, SRCCOPY);
			SelectObject(hTmpDC, hOldBitmap);
			DeleteObject(hTmpBitmap);
		}
		DeleteDC(hTmpDC);
	}
	//*/

	/*
	// 方法二
	void *pvBits = NULL;
	HDC hTmpDC = CreateCompatibleDC(hdc);
	if (hTmpDC)
	{
		HBITMAP hTmpBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
		if (hTmpBitmap)
		{
			//bResult = ::SetBitmapBits(hTmpBitmap, Width * Height * 4, m_DibInfo.m_pvData);

			BITMAP tmpBitmap;
			GetObject(hTmpBitmap, sizeof(BITMAP), &tmpBitmap);
			memcpy(tmpBitmap.bmBits, m_DibInfo.m_pvData, (m_DibInfo.m_bmWidth * m_DibInfo.m_bmHeight) << 2);

			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hTmpDC, hTmpBitmap);
			BitBlt(hdc, nXDst, nYDst, Width, Height, hTmpDC, nXSrc, nYSrc, SRCCOPY);
			SelectObject(hTmpDC, hOldBitmap);
			DeleteObject(hTmpBitmap);
		}
		DeleteDC(hTmpDC);
	}
	//*/

	/*
	// 方法三
	// 这种方法只能用于目标宽,高和源宽,高一样的情况, 或者
	// 缩放显示(但要求源的宽,高要跟初始设置的一致,因为pvData的排列是固定了的,
	// 也就是说只能对全图缩放,不能一次性对局部缩放)
	// 或者可以先整图原样缩放(或不缩放)到一个bitmap里面,然后再截取你想要的部分,跟前面一种方法类似(有点多此一举)
	bResult = StretchDIBits(hdc, nXDst, nYDst, Width, Height, nXSrc, nYSrc,
		m_DibInfo.m_bmWidth, m_DibInfo.m_bmHeight,
		m_DibInfo.m_pvData, &bmi, DIB_RGB_COLORS, SRCCOPY);
	//*/

	///*
	// 方法四, 缺点, 需要Vfw32(msvfw32.dll)支持, 但速度应该是最快的
	HDRAWDIB hdd = DrawDibOpen();

	if (hdd != NULL)
	{
		bResult = DrawDibBegin(hdd,
			hdc,
			-1,
			-1,
			&bmi.bmiHeader,
			//nWidth,
			//nHeight,
			nWidth,
			nHeight,
			//DDF_SAME_HDC |
			//DDF_UPDATE |
			0);

		if (bResult)
		{
			bResult = DrawDibDraw(hdd,
				hdc,
				nXDst,
				nYDst,
				nWidth,
				nHeight,
				&bmi.bmiHeader,
				m_DibInfo.pvData,
				nXSrc,
				nYSrc,
				nWidth,
				nHeight,
				//DDF_SAME_HDC | DDF_SAME_DRAW | DDF_UPDATE
				//DDF_SAME_DIB |
				//DDF_SAME_SIZE |
				//DDF_UPDATE |
				DDF_NOTKEYFRAME |
				DDF_SAME_HDC |
				DDF_SAME_DRAW);
		}

		DrawDibEnd(hdd);
	}
	//*/

	return S_OK;
}

HRESULT CFastDIB::Draw(CFastDIB *pDstDIB, UINT uDrawMode, int nXDst, int nYDst, int nWidth, int nHeight, int nXSrc, int nYSrc,
					   BYTE nAlphaValue, BYTE nBoardWidth, DWORD nBoardColor)
{
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);
	_CHECKTYPE(pDstDIB->GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);

	if (nWidth == 0) nWidth  = m_DibInfo.bmWidth-nXSrc;
	if (nHeight== 0) nHeight = m_DibInfo.bmHeight-nYSrc;

	int i = 0, j = 0;

	int SrcHeight = m_DibInfo.bmHeight;
	int SrcWidth  = m_DibInfo.bmWidth;
	int DstHeight = pDstDIB->GetBackDibInfo()->bmHeight;
	int DstWidth  = pDstDIB->GetBackDibInfo()->bmWidth;

	DWORD *pSrc = NULL;
	DWORD *pDst = NULL;
	DWORD *pDst2 = NULL;

	////////////////////////////////////////////////////////

	if (nXDst < 0)
	{
		nWidth += nXDst;
		nXSrc -= nXDst;
		nXDst = 0;
	}
	if (nYDst < 0)
	{
		nHeight += nYDst;
		nYSrc -= nYDst;
		nYDst = 0;
	}
	int tmpx = (nWidth + nXDst - DstWidth);
	if (tmpx > 0)
		nWidth -= tmpx;
	int tmpy = (nHeight + nYDst - DstHeight);
	if (tmpy > 0)
		nHeight -= tmpy;

	////////裁剪DstZone///////////

	if (nXSrc < 0)
	{
		nWidth += nXSrc;
		nXSrc = 0;
	}
	if (nYSrc < 0)
	{
		nHeight += nYSrc;
		nYSrc = 0;
	}
	int tmpx2 = (nWidth + nXSrc - SrcWidth);
	if (tmpx2 > 0)
		nWidth -= tmpx2;
	int tmpy2 = (nHeight + nYSrc - SrcHeight);
	if (tmpy2 > 0)
		nHeight -= tmpy2;

	///////////////////区域裁剪////////////////////////////

	if ((uDrawMode & DRAWMODE_ALPHABLEND) != DRAWMODE_ALPHABLEND)
		nAlphaValue = 255;

	if ((pDstDIB->GetDibInfo()->nType == FDIBTYPE_DOUBLEBUF)
        && ((uDrawMode & DRAWMODE_FROMBACK) == DRAWMODE_FROMBACK))
		pSrc = (DWORD *)m_DibInfoBack.pvData + (SrcHeight-nYSrc-1) * SrcWidth + nXSrc;
	else
		pSrc = (DWORD *)m_DibInfo.pvData + (SrcHeight-nYSrc-1) * SrcWidth + nXSrc;

	pDst2 = (DWORD *)(pDstDIB->GetDibInfo()->pvData) + (DstHeight-nYDst-1) * DstWidth + nXDst;
	if (pDstDIB->GetDibInfo()->nType == FDIBTYPE_DOUBLEBUF)
		pDst = (DWORD *)(pDstDIB->GetBackDibInfo()->pvData) + (DstHeight-nYDst-1) * DstWidth + nXDst;
	else
		pDst = (DWORD *)(pDstDIB->GetDibInfo()->pvData) + (DstHeight-nYDst-1) * DstWidth + nXDst;

	UINT DrawMode = uDrawMode & 0x0000FFFF;

	DRAWPARAM dpm;
	ZeroMemory(&dpm, sizeof(DRAWPARAM));
	dpm.nXDst = nXDst;
	dpm.nYDst = nYDst;
	dpm.nWidth  = nWidth;
	dpm.nHeight = nHeight;
	dpm.nXSrc = nXSrc;
	dpm.nYSrc = nYSrc;
	dpm.nDstWidth  = DstWidth;
	dpm.nDstHeight = DstHeight;
	dpm.nSrcWidth  = SrcWidth;
	dpm.nSrcHeight = SrcHeight;
	dpm.pDst  = pDst;
	dpm.pDst2 = pDst2;
	dpm.pSrc  = pSrc;
	dpm.nAlphaValue = nAlphaValue;
	dpm.nBoardWidth = nBoardWidth;
	dpm.nBoardColor = nBoardColor;

	try {
		if (uDrawMode == DRAWMODE_COPY) {
			_DrawCopy(&dpm);
		}
		else
		{
			switch (DrawMode)
			{
			case DRAWMODE_IGNOREALPHA:
				_DrawIgnoreAlpha(&dpm);
				break;
			case DRAWMODE_NORMAL:
				_DrawNormal(&dpm);
				break;
			case DRAWMODE_ADDSATURATION:
				_DrawAddSaturation(&dpm);
				break;
			case DRAWMODE_SUBSATURATION:
				_DrawSubSaturation(&dpm);
				break;
			case DRAWMODE_XSUBSATURATION:
				_DrawSubSaturationFlipAlpha(&dpm);
				break;
			case DRAWMODE_XADDSATURATION:
				_DrawAddSaturationFlipAlpha(&dpm);
				break;
			case DRAWMODE_FLIPALPHA:
				_DrawFlipAlpha(&dpm);
                break;
            default:
                break;
			}
		}
	}
	catch (...) {
		return E_FAIL;
	}
	return S_OK;
}

void CFastDIB::_DrawCopy(DRAWPARAM *pdpm)
{
	DWORD *pCurrentSrcp = pdpm->pSrc;
	DWORD *pCurrentDstp = pdpm->pDst2;
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;

	DWORD nWidth4 = nWidth << 2;
	for (int j = 0; j<(int)nHeight; j++)
	{
		memcpy(pCurrentDstp, pCurrentSrcp, nWidth4);
		pCurrentSrcp -= pdpm->nSrcWidth;
		pCurrentDstp -= pdpm->nDstWidth;
	}
}

void CFastDIB::_DrawIgnoreAlpha(DRAWPARAM *pdpm)
{
	DWORD CurrentSrc  = DWORD(pdpm->pSrc);
	DWORD CurrentDst  = DWORD(pdpm->pDst);
	DWORD CurrentDst2 = DWORD(pdpm->pDst2);
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;
	DWORD DstWidth4 = pdpm->nDstWidth << 2;
	DWORD SrcWidth4 = pdpm->nSrcWidth << 2;
	DWORD nBoardColor  = pdpm->nBoardColor;
	BYTE  nAlphaValue  = pdpm->nAlphaValue;
	BYTE  nBoardWidth  = pdpm->nBoardWidth;
	DWORD dwBoardWidth = pdpm->nBoardWidth;

	int i = 0, j = 0;

	__int64 AlphaMask = 0x0100010001000100;
	__int64 nTest64 = 0x00FF00FF00FF00FF;

	__asm {
		mov j, 0

		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
LOOP1:
		mov i, 0

LOOP2:
		movd mm0, [EBX]

///*
		////////////////////////////////////////////////////////
		////////////////画色边//////////////////////////////////
		mov EAX, 0
		mov AL, nBoardWidth
		cmp j, EAX
		jb LABEL1
		cmp i, EAX
		jb LABEL1
		mov EAX, nWidth
		sub EAX, dwBoardWidth
		cmp i, EAX
		jnb LABEL1
		mov EAX, nHeight
		sub EAX, dwBoardWidth
		cmp j, EAX
		jnb LABEL1

		movd [EDX], mm0
		jmp LABEL2

LABEL1: mov EAX, nBoardColor
		///////////////////////////////////////////////////////
		mov [EDX], EAX
//*/

LABEL2:
		movd [EDX], mm0

		//mov EAX, [EBX]
		//mov [EDX], EAX
		add EBX, 4
		add ECX, 4
		add EDX, 4
		inc i
		mov EAX, i
		cmp EAX, nWidth
		jb LOOP2
		/////////////////////////////////////////////////////////////////
		// pSrc = pSrc - 4*SrcWidth
		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2

		sub EBX, SrcWidth4
		sub ECX, DstWidth4
		sub EDX, DstWidth4

		mov CurrentSrc, EBX
		mov CurrentDst, ECX
		mov CurrentDst2, EDX
		/////////////////////////////////////////////////////////////////
		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOP1

		EMMS
	}
}

void CFastDIB::_DrawNormal(DRAWPARAM *pdpm)
{
	DWORD CurrentSrc  = DWORD(pdpm->pSrc);
	DWORD CurrentDst  = DWORD(pdpm->pDst);
	DWORD CurrentDst2 = DWORD(pdpm->pDst2);
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;
	DWORD DstWidth4 = pdpm->nDstWidth << 2;
	DWORD SrcWidth4 = pdpm->nSrcWidth << 2;
	DWORD nBoardColor  = pdpm->nBoardColor;
	BYTE  nAlphaValue  = pdpm->nAlphaValue;
	BYTE  nBoardWidth  = pdpm->nBoardWidth;
	DWORD dwBoardWidth = pdpm->nBoardWidth;

	int i = 0, j = 0;

	__int64 AlphaMask = 0x0100010001000100;

	__asm {
		mov j, 0

		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
LOOP1:
		mov i, 0

LOOP2:
		mov EAX, [EBX]
		pxor mm7, mm7
		movd mm0, EAX
		movd mm1, [ECX]

		shr EAX, 24
		mul BYTE PTR nAlphaValue
		shr EAX, 8

		movd mm2, EAX
		punpcklwd mm2, mm2
		punpckldq mm2, mm2
		//mm2 = 源图片Alpha值
		movq mm3, AlphaMask
		psubusw mm3, mm2
		//mm3 = 256-源图片Alpha值
		punpcklbw mm0, mm7
		punpcklbw mm1, mm7

		pmullw mm0, mm2
		pmullw mm1, mm3

		paddusw mm0, mm1
		psrlw mm0, 8

		packuswb mm0, mm7

/*
		////////////////////////////////////////////////////////
		////////////////画色边//////////////////////////////////
		mov EAX, 0
		mov AL, nBoardWidth
		cmp j, EAX
		jb LABEL1
		cmp i, EAX
		jb LABEL1
		mov EAX, nWidth
		sub EAX, dwBoardWidth
		cmp i, EAX
		jnb LABEL1
		mov EAX, nHeight
		sub EAX, dwBoardWidth
		cmp j, EAX
		jnb LABEL1

		movd [EDX], mm0
		jmp LABEL2

LABEL1: mov EAX, nBoardColor
		///////////////////////////////////////////////////////
		mov [EDX], EAX
//*/

//LABEL2:
		add EBX, 4
		movd [EDX], mm0
		add ECX, 4
		add EDX, 4
		inc i
		mov EAX, i
		cmp EAX, nWidth
		jb LOOP2
		/////////////////////////////////////////////////////////////////
		// pSrc = pSrc - 4*SrcWidth
		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2

		sub EBX, SrcWidth4
		sub ECX, DstWidth4
		sub EDX, DstWidth4

		mov CurrentSrc, EBX
		mov CurrentDst, ECX
		mov CurrentDst2, EDX
		/////////////////////////////////////////////////////////////////
		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOP1

		EMMS
	}
}

void CFastDIB::_DrawAddSaturation(DRAWPARAM *pdpm)
{
	DWORD CurrentSrc  = DWORD(pdpm->pSrc);
	DWORD CurrentDst  = DWORD(pdpm->pDst);
	DWORD CurrentDst2 = DWORD(pdpm->pDst2);
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;
	DWORD DstWidth4 = pdpm->nDstWidth << 2;
	DWORD SrcWidth4 = pdpm->nSrcWidth << 2;
	BYTE  nAlphaValue  = pdpm->nAlphaValue;
	DWORD nBoardColor  = pdpm->nBoardColor;
	BYTE  nBoardWidth  = pdpm->nBoardWidth;
	DWORD dwBoardWidth = pdpm->nBoardWidth;

	int i = 0, j = 0;

	__int64 AlphaMask = 0x0100010001000100;

	__asm {
		mov j, 0
		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
LOOP1:
		mov i, 0

LOOP2:
		pxor mm7, mm7
		mov EAX, [EBX]
		movd mm0, EAX
		movd mm1, [ECX]

		shr EAX, 24
		mul BYTE PTR nAlphaValue
		shr EAX, 8

		movd mm2, EAX
		punpcklwd mm2, mm2
		punpckldq mm2, mm2

		punpcklbw mm0, mm7
		punpcklbw mm1, mm7

		pmullw mm0, mm2
		pmullw mm1, AlphaMask

		paddusw mm0, mm1
		psrlw mm0, 8

		packuswb mm0, mm7

		////////////////画色边//////////////////////////////////
		mov EAX, 0
		mov AL, nBoardWidth
		cmp j, EAX
		jb LABEL1
		cmp i, EAX
		jb LABEL1
		mov EAX, nWidth
		sub EAX, dwBoardWidth
		cmp i, EAX
		jnb LABEL1
		mov EAX, nHeight
		sub EAX, dwBoardWidth
		cmp j, EAX
		jnb LABEL1

		movd [EDX], mm0
		jmp LABEL2

LABEL1: mov EAX, nBoardColor
		///////////////////////////////////////////////////////
		movd [EDX], mm0

LABEL2: add EBX, 4
		add ECX, 4
		add EDX, 4
		inc i
		mov EAX, i
		cmp EAX, nWidth
		jb LOOP2

		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
		sub EBX, SrcWidth4
		sub ECX, DstWidth4
		sub EDX, DstWidth4
		mov CurrentSrc, EBX
		mov CurrentDst, ECX
		mov CurrentDst2, EDX

		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOP1

		EMMS
	}
}

void CFastDIB::_DrawSubSaturation(DRAWPARAM *pdpm)
{
	DWORD CurrentSrc  = (DWORD)(pdpm->pSrc);
	DWORD CurrentDst  = (DWORD)(pdpm->pDst);
	DWORD CurrentDst2 = (DWORD)(pdpm->pDst2);
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;
	DWORD DstWidth4 = pdpm->nDstWidth << 2;
	DWORD SrcWidth4 = pdpm->nSrcWidth << 2;
	BYTE  nAlphaValue = pdpm->nAlphaValue;

	int i = 0, j = 0;

	__int64 AlphaMask = 0x0100010001000100;


	__asm {
		mov j, 0
		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
LOOP1:
		mov i, 0

LOOP2:
		pxor mm7, mm7
		mov EAX, [EBX]
		movd mm0, EAX
		movd mm1, [ECX]

		shr EAX, 24
		mul BYTE PTR nAlphaValue
		shr EAX, 8

		movd mm2, EAX
		punpcklwd mm2, mm2
		punpckldq mm2, mm2

		punpcklbw mm0, mm7
		punpcklbw mm1, mm7

		pmullw mm0, mm2
		pmullw mm1, AlphaMask

		psubusw mm1, mm0
		psrlw mm1, 8

		packuswb mm1, mm7

		movd [EDX], mm1

		add EBX, 4
		add ECX, 4
		add EDX, 4
		inc i
		mov EAX, i
		cmp EAX, nWidth
		jb LOOP2

		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
		sub EBX, SrcWidth4
		sub ECX, DstWidth4
		sub EDX, DstWidth4
		mov CurrentSrc, EBX
		mov CurrentDst, ECX
		mov CurrentDst2, EDX

		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOP1

		EMMS
	}
}

void CFastDIB::_DrawFlipAlpha(DRAWPARAM *pdpm)
{
	DWORD CurrentSrc  = (DWORD)(pdpm->pSrc);
	DWORD CurrentDst  = (DWORD)(pdpm->pDst);
	DWORD CurrentDst2 = (DWORD)(pdpm->pDst2);
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;
	DWORD DstWidth4 = pdpm->nDstWidth << 2;
	DWORD SrcWidth4 = pdpm->nSrcWidth << 2;
	BYTE  nAlphaValue = pdpm->nAlphaValue;

	int i = 0, j = 0;

	__int64 AlphaMask = 0x0100010001000100;

	__asm {
		mov j, 0
		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
LOOP1:
		mov i, 0

LOOP2:
		pxor mm7, mm7
		mov EAX, [EBX]
		movd mm0, EAX
		movd mm1, [ECX]

		shr EAX, 24
		mul BYTE PTR nAlphaValue
		shr EAX, 8

		movd mm2, EAX
		punpcklwd mm2, mm2
		punpckldq mm2, mm2

		movq mm3, AlphaMask
		psubusw mm3, mm2

		punpcklbw mm0, mm7
		punpcklbw mm1, mm7

		pmullw mm0, mm3
		pmullw mm1, mm2

		paddusw mm0, mm1
		psrlw mm0, 8

		packuswb mm0, mm7

		movd [EDX], mm0

		add EBX, 4
		add ECX, 4
		add EDX, 4
		inc i
		mov EAX, i
		cmp EAX, nWidth
		jb LOOP2

		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
		sub EBX, SrcWidth4
		sub ECX, DstWidth4
		Sub EDX, DstWidth4
		mov CurrentSrc, EBX
		mov CurrentDst, ECX
		mov CurrentDst2, EDX

		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOP1

		EMMS
	}
}

void CFastDIB::_DrawAddSaturationFlipAlpha(DRAWPARAM *pdpm)
{
	DWORD CurrentSrc  = (DWORD)(pdpm->pSrc);
	DWORD CurrentDst  = (DWORD)(pdpm->pDst);
	DWORD CurrentDst2 = (DWORD)(pdpm->pDst2);
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;
	DWORD DstWidth4 = pdpm->nDstWidth << 2;
	DWORD SrcWidth4 = pdpm->nSrcWidth << 2;
	BYTE  nAlphaValue = pdpm->nAlphaValue;

	int i = 0, j = 0;

	__int64 AlphaMask = 0x0100010001000100;

	__asm {
		mov j, 0
		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
LOOP1:
		mov i, 0

LOOP2:
		pxor mm7, mm7
		mov EAX, [EBX]
		movd mm0, EAX
		movd mm1, [ECX]

		shr EAX, 24
		mul BYTE PTR nAlphaValue
		shr EAX, 8

		movd mm2, EAX
		punpcklwd mm2, mm2
		punpckldq mm2, mm2

		punpcklbw mm0, mm7
		punpcklbw mm1, mm7

		pmullw mm1, mm2
		pmullw mm0, AlphaMask

		paddusw mm0, mm1
		psrlw mm0, 8

		packuswb mm0, mm7

		movd [EDX], mm0

		add EBX, 4
		add ECX, 4
		add EDX, 4
		inc i
		mov EAX, i
		cmp EAX, nWidth
		jb LOOP2

		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
		sub EBX, SrcWidth4
		sub ECX, DstWidth4
		sub EDX, DstWidth4
		mov CurrentSrc, EBX
		mov CurrentDst, ECX
		mov CurrentDst2, EDX

		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOP1

		EMMS
	}
}

void CFastDIB::_DrawSubSaturationFlipAlpha(DRAWPARAM *pdpm)
{
	DWORD CurrentSrc  = (DWORD)(pdpm->pSrc);
	DWORD CurrentDst  = (DWORD)(pdpm->pDst);
	DWORD CurrentDst2 = (DWORD)(pdpm->pDst2);
	DWORD nWidth  = pdpm->nWidth;
	DWORD nHeight = pdpm->nHeight;
	DWORD DstWidth4 = pdpm->nDstWidth << 2;
	DWORD SrcWidth4 = pdpm->nSrcWidth << 2;
	BYTE  nAlphaValue = pdpm->nAlphaValue;

	int i = 0, j = 0;

	__int64 AlphaMask = 0x0100010001000100;

	__asm {
		mov j, 0
		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
LOOP1:
		mov i, 0

LOOP2:
		pxor mm7, mm7
		mov EAX, [EBX]
		movd mm0, EAX
		movd mm1, [ECX]

		shr EAX, 24
		mul BYTE PTR nAlphaValue
		shr EAX, 8

		movd mm2, EAX
		punpcklwd mm2, mm2
		punpckldq mm2, mm2

		punpcklbw mm0, mm7
		punpcklbw mm1, mm7

		pmullw mm1, mm2
		pmullw mm0, AlphaMask

		psubusw mm0, mm1
		psrlw mm0, 8

		packuswb mm0, mm7

		movd [EDX], mm0

		add EBX, 4
		add ECX, 4
		add EDX, 4
		inc i
		mov EAX, i
		cmp EAX, nWidth
		jb LOOP2

		mov EBX, CurrentSrc
		mov ECX, CurrentDst
		mov EDX, CurrentDst2
		sub EBX, SrcWidth4
		sub ECX, DstWidth4
		sub EDX, DstWidth4
		mov CurrentSrc, EBX
		mov CurrentDst, ECX
		mov CurrentDst2, EDX

		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOP1

		EMMS
	}
}

HRESULT CFastDIB::CreateDIB(DWORD dwWidth, DWORD dwHeight, FASTDIBTYPE fdt)
{
	if (((fdt & FDIBTYPE_DOUBLEBUF) == FDIBTYPE_DOUBLEBUF) && (!CFastDIB::m_bInited))
		return E_NOTINIT;

	if ((m_DibInfo.nType & FDIBTYPE_NULLFRONT) != FDIBTYPE_NULLFRONT)
	{
		if (m_DibInfo.pvData != NULL)
		{
			delete[] m_DibInfo.pvData;
			m_DibInfo.pvData = NULL;
		}
	}
	if ((m_DibInfo.nType & FDIBTYPE_NULLBACK) != FDIBTYPE_NULLBACK)
	{
		if (m_DibInfoBack.pvData != NULL)
		{
			delete[] m_DibInfoBack.pvData;
			m_DibInfoBack.pvData = NULL;
		}
	}
	m_DibInfo.bmWidth  = dwWidth;
	m_DibInfo.bmHeight = dwHeight;
	m_DibInfoBack.bmWidth  = dwWidth;
	m_DibInfoBack.bmHeight = dwHeight;
	m_DibInfo.nType = fdt;
	FASTDIBTYPE tmpType = (m_DibInfo.nType & 0x0000FFFF);
	switch (tmpType)
	{
	case FDIBTYPE_RGBA:
		if ((m_DibInfo.nType & FDIBTYPE_NULLFRONT) != FDIBTYPE_NULLFRONT) {
			m_DibInfo.pvData = new DWORD[dwWidth * dwHeight];
		}
		break;
	case FDIBTYPE_8BITBUF:
		if ((m_DibInfo.nType & FDIBTYPE_NULLFRONT) != FDIBTYPE_NULLFRONT) {
			m_DibInfo.pvData = new BYTE[dwWidth * dwHeight];
		}
		break;
	case FDIBTYPE_16BITBUF:
		if ((m_DibInfo.nType & FDIBTYPE_NULLFRONT) != FDIBTYPE_NULLFRONT) {
			m_DibInfo.pvData = new WORD[dwWidth * dwHeight];
		}
		break;
	case FDIBTYPE_DOUBLEBUF:
		if ((m_DibInfo.nType & FDIBTYPE_NULLFRONT) != FDIBTYPE_NULLFRONT) {
			m_DibInfo.pvData = new DWORD[CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight];
		}
		if ((m_DibInfo.nType & FDIBTYPE_NULLBACK) != FDIBTYPE_NULLBACK) {
			m_DibInfoBack.pvData = new DWORD[CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight];
		}
	}
	return S_OK;
}

HRESULT CFastDIB::WaterFX(CFastDIB *pCurrentBuf, CFastDIB *pNextBuf)
{
	_CHECKTYPE(pCurrentBuf->GetDibInfo(), FDIBTYPE_8BITBUF, TCM_BELONG);
	_CHECKTYPE(pNextBuf->GetDibInfo(), FDIBTYPE_8BITBUF, TCM_BELONG);

	if (pCurrentBuf->GetDibInfo()->bmHeight != pNextBuf->GetDibInfo()->bmHeight)
		return E_FAIL;
	if (pCurrentBuf->GetDibInfo()->bmWidth != pNextBuf->GetDibInfo()->bmWidth)
		return E_FAIL;

	DWORD dwWidth  = pNextBuf->GetDibInfo()->bmWidth;
	DWORD dwHeight = pNextBuf->GetDibInfo()->bmHeight;

	BYTE *pCurrent[5] = {NULL, NULL, NULL, NULL, NULL};

	BYTE *pNext = (BYTE *)(pNextBuf->GetDibInfo()->pvData);
	pCurrent[0] = (BYTE *)(pCurrentBuf->GetDibInfo()->pvData);

	for (int j = 0; j<(int)dwHeight; j++)
	{
		for (int i = 0; i<(int)dwWidth; i++)
		{
			if (j > 1)
				pCurrent[1] = pCurrent[0] - dwWidth;
			else
				pCurrent[1] = pCurrent[0];

			if (i < (int)dwWidth-1)
				pCurrent[2] = pCurrent[0] + 1;
			else
				pCurrent[2] = pCurrent[0];

			if (j < (int)dwHeight-1)
				pCurrent[3] = pCurrent[0] + dwWidth;
			else
				pCurrent[3] = pCurrent[0];

			if (i > 1)
				pCurrent[4] = pCurrent[0] - 1;
			else
				pCurrent[4] = pCurrent[0];

			short tmpint = ((*pCurrent[1] + *pCurrent[2] + *pCurrent[3] + *pCurrent[4]) >> 1) - (*pNext);
			if (tmpint < 0)
				*pNext = 0;
			else
				*pNext = (BYTE)tmpint;

			BYTE tmpn = ((*pNext) >> 5);
			*pNext -= tmpn;

			pCurrent[0]++;
			pNext++;
		}
	}
	BYTE *p = (BYTE *)pCurrentBuf->GetDibInfo()->pvData;
	pCurrentBuf->GetDibInfo()->pvData = pNextBuf->GetDibInfo()->pvData;
	pNextBuf->GetDibInfo()->pvData = p;
    /*
	memcpy(pCurrentBuf->GetDibInfo()->pvData, pNextBuf->GetDibInfo()->pvData,
        pNextBuf->GetDibInfo()->bmWidth * pNextBuf->GetDibInfo()->bmHeight);
    //*/
	return S_OK;
}

HRESULT CFastDIB::WaterFX16(CFastDIB *pCurrentBuf, CFastDIB *pNextBuf)
{
	_CHECKTYPE(pCurrentBuf->GetDibInfo(), FDIBTYPE_16BITBUF, TCM_BELONG);
	_CHECKTYPE(pNextBuf->GetDibInfo(), FDIBTYPE_16BITBUF, TCM_BELONG);

	if (pCurrentBuf->GetDibInfo()->bmHeight != pNextBuf->GetDibInfo()->bmHeight)
		return E_FAIL;
	if (pCurrentBuf->GetDibInfo()->bmWidth != pNextBuf->GetDibInfo()->bmWidth)
		return E_FAIL;

	DWORD dwWidth  = pNextBuf->GetDibInfo()->bmWidth;
	DWORD dwHeight = pNextBuf->GetDibInfo()->bmHeight;

	WORD *pCurrent[5] = {NULL, NULL, NULL, NULL, NULL};

	WORD *pNext = (WORD *)(pNextBuf->GetDibInfo()->pvData);
	pCurrent[0] = (WORD *)(pCurrentBuf->GetDibInfo()->pvData);

	for (int j = 0; j<(int)dwHeight; j++)
	{
		for (int i = 0; i<(int)dwWidth; i++)
		{
			if (j > 1)
				pCurrent[1] = pCurrent[0] - dwWidth;
			else
				pCurrent[1] = pCurrent[0];

			if (i<(int)dwWidth-1)
				pCurrent[2] = pCurrent[0] + 1;
			else
				pCurrent[2] = pCurrent[0];

			if (j<(int)dwHeight-1)
				pCurrent[3] = pCurrent[0] + dwWidth;
			else
				pCurrent[3] = pCurrent[0];

			if (i > 1)
				pCurrent[4] = pCurrent[0] - 1;
			else
				pCurrent[4] = pCurrent[0];

			short tmpint = ((*pCurrent[1] + *pCurrent[2] + *pCurrent[3] + *pCurrent[4])>>1) - (*pNext);
			if (tmpint < 0)
				*pNext = 0;
			else
				*pNext = (BYTE)tmpint;

			BYTE tmpn = ((*pNext) >> 5);
			*pNext -= tmpn;

			pCurrent[0]++;
			pNext++;
		}
	}
	WORD *p = (WORD *)pCurrentBuf->GetDibInfo()->pvData;
	pCurrentBuf->GetDibInfo()->pvData = pNextBuf->GetDibInfo()->pvData;
	pNextBuf->GetDibInfo()->pvData = p;

	return S_OK;
}

HRESULT CFastDIB::DistortByDIB(int x, int y, CFastDIB *pDIB)
{
	_CHECKTYPE(pDIB->GetDibInfo(), FDIBTYPE_8BITBUF, TCM_BELONG);
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);

	//////////////入口检查////////////////////////
	int DstWidth  = m_DibInfoBack.bmWidth;
	int DstHeight = m_DibInfoBack.bmHeight;
	int SrcWidth  = pDIB->GetDibInfo()->bmWidth;
	int SrcHeight = pDIB->GetDibInfo()->bmHeight;
	int nWidth  = SrcWidth;
	int nHeight = SrcHeight;
	if (x < 0)
	{
		nWidth += x;
		x = 0;
	}
	if (y < 0)
	{
		nHeight += y;
		y = 0;
	}
	if (nWidth + x > DstWidth)
		nWidth = DstWidth - x;
	if (nHeight + y > DstHeight)
		nHeight = DstHeight - y;

	/////////////区域裁减///////////////////////////
	DWORD *pSrc = NULL;
	DWORD *tmpBuf = NULL;
	if (m_DibInfo.nType != FDIBTYPE_DOUBLEBUF)
	{
		tmpBuf = new DWORD[DstWidth * DstHeight];
		memcpy((void *)tmpBuf, (void *)m_DibInfo.pvData, DstWidth * DstHeight * 4);
		pSrc = tmpBuf;
	}
	else
	{
		tmpBuf = (DWORD*)m_DibInfoBack.pvData;
		pSrc = tmpBuf;
	}
	DWORD *pDst = (DWORD *)m_DibInfo.pvData;
	BYTE *pMap[5] = {(BYTE *)pDIB->GetDibInfo()->pvData, NULL, NULL, NULL, NULL};
	/////////////内存分配/////////////////////////////
	for (int j=2; j<nHeight-1; j++)
	{
		pMap[0] = (BYTE *)pDIB->GetDibInfo()->pvData + (SrcHeight-j) * SrcWidth;
		pSrc = (DWORD *)tmpBuf + (DstHeight-j-y) * DstWidth + x;
		pDst = (DWORD *)m_DibInfo.pvData + (DstHeight-j-y) * DstWidth + x;
		for (int i = 0; i<nWidth; i++)
		{
			if (j > 0)
				pMap[1] = pMap[0] - SrcWidth;
			else
				pMap[1] = pMap[0];

			if (i<(int)SrcWidth)
				pMap[2] = pMap[0] + 1;
			else
				pMap[2] = pMap[0];

			if (j<(int)SrcHeight)
				pMap[3] = pMap[0] + SrcWidth;
			else
				pMap[3] = pMap[0];

			if (i > 0)
				pMap[4] = pMap[0] - 1;
			else
				pMap[4] = pMap[0];

			int dify = (*pMap[3]) - (*pMap[1]);
			int difx = (*pMap[2]) - (*pMap[4]);

			if ((dify > 0) && (dify > j+y)) dify = j+y;
			if ((dify < 0) && (dify >= (DstHeight-j-y))) dify = DstHeight-j-y;
			if ((difx < 0) && (difx > i+x)) difx = i+x;
			if ((difx > 0) && (difx >=( DstWidth-i-x))) difx = DstWidth-i-x;

			*pDst = *((DWORD *)pSrc + dify * DstWidth + difx);

			pMap[0]++;
			pDst++;
			pSrc++;
		}
	}
	//////////////计算/////////////////////
	if (m_DibInfo.nType != FDIBTYPE_DOUBLEBUF)
		delete[] tmpBuf;
	return S_OK;
}

HRESULT CFastDIB::DistortByDIB16(int x, int y, CFastDIB *pDIB)
{
	_CHECKTYPE(pDIB->GetDibInfo(), FDIBTYPE_16BITBUF, TCM_BELONG);
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);

	int DstWidth  = m_DibInfo.bmWidth;
	int DstHeight = m_DibInfo.bmHeight;

	int SrcWidth  = pDIB->GetDibInfo()->bmWidth;
	int SrcHeight = pDIB->GetDibInfo()->bmHeight;

	int nWidth  = SrcWidth;
	int nHeight = SrcHeight;

	if (x < 0)
	{
		nWidth += x;
		x = 0;
	}
	if (y < 0)
	{
		nHeight += y;
		y = 0;
	}
	int tmpx = (nWidth + x - DstWidth);
	if (tmpx > 0)
		nWidth -= tmpx;
	int tmpy = (nHeight + y - DstHeight);
	if (tmpy > 0)
		nHeight -= tmpy;

	DWORD *tmpBuf = NULL;
	if (m_DibInfo.nType == FDIBTYPE_DOUBLEBUF)
		tmpBuf = (DWORD *)m_DibInfoBack.pvData;
	else
	{
		tmpBuf = new DWORD[DstWidth * DstHeight];
		memcpy((void *)tmpBuf, (void *)m_DibInfo.pvData, DstWidth * DstHeight * 4);
	}

	DWORD *pDst = (DWORD *)m_DibInfo.pvData;
	DWORD *p = pDst;
	DWORD *pSrc = tmpBuf;

	WORD *pMap[5] = {(WORD *)pDIB->GetDibInfo()->pvData, NULL, NULL, NULL, NULL};
	for (int j = 0; j<nHeight; j++)
	{
		for (int i = 0; i<nWidth; i++)
		{
			if (j > 0)
				pMap[1] = pMap[0] - SrcWidth;
			else
				pMap[1] = pMap[0];

			if (i<(int)SrcWidth-1)
				pMap[2] = pMap[0] + 1;
			else
				pMap[2] = pMap[0];

			if (j<(int)SrcHeight-1)
				pMap[3] = pMap[0] + SrcWidth;
			else
				pMap[3] = pMap[0];

			if (i > 0)
				pMap[4] = pMap[0] - 1;
			else
				pMap[4] = pMap[0];

			int dify = (*pMap[3]) - (*pMap[1]);
			int difx = (*pMap[2]) - (*pMap[4]);

			int tmpy2 = j + dify;
			int tmpx2 = i + difx;

			if (tmpy2 < 0)
				dify = 0;

			if (tmpy2 > nHeight - 1)
				dify = 0 ;

			if (tmpx2 < 0)
				difx = i;

			if (tmpx2 > nWidth - 1)
				difx = nWidth-i-1;

			*pDst = *(pSrc + dify * SrcWidth + difx);

			//*pDst = *pSrc;

			pMap[0]++;
			pDst++;
			pSrc++;
		}
		p += SrcWidth;
		pDst = p;
	}
	if (m_DibInfo.nType != FDIBTYPE_DOUBLEBUF)
		delete[] tmpBuf;
	return S_OK;
}

HRESULT CFastDIB::ActivateBuf(int x, int y, BYTE nEnerge)
{
	_CHECKTYPE(GetDibInfo(), FDIBTYPE_8BITBUF, TCM_BELONG);
	if (nEnerge == 0)
		return S_OK;

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	BYTE *p = (BYTE *)m_DibInfo.pvData;
	p = p + (m_DibInfo.bmHeight-y-1) * m_DibInfo.bmWidth + x;

	int tmp = *p + nEnerge;
	if (tmp < 0)
		*p = 0;
	else {
		if (tmp > 0xFF)
			*p = 0xFF;
		else
			*p = tmp;
	}
	return S_OK;
}

HRESULT CFastDIB::ActivateBuf16(int x, int y, int r, WORD nEnerge)
{
	_CHECKTYPE(GetDibInfo(), FDIBTYPE_16BITBUF, TCM_BELONG);
	if (nEnerge == 0)
		return S_OK;

    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (r < 0)
        r = 0;

	WORD *p = (WORD *)m_DibInfo.pvData;
	WORD *p1 = p;
	p = p + (m_DibInfo.bmHeight-y-1) * (m_DibInfo.bmWidth) + x;
	for (int j = 0; j<r; j++)
	{
		for (int i = 0; i<r; i++)
		{
			int tmp = *p + nEnerge;
			if (tmp < 0)
				*p = 0;
			else
				*p = tmp;
			p++;
		}
		p1 += m_DibInfo.bmWidth;
		p = p1;

	}
	return S_OK;
}

HRESULT CFastDIB::CreateCompatibleDIB(CFastDIB *pDIB)
{
	if (pDIB == NULL)
		return E_EMPTYOBJECT;

	CreateDIB(pDIB->GetBackDibInfo()->bmWidth, pDIB->GetBackDibInfo()->bmHeight, pDIB->GetDibInfo()->nType);
	return S_OK;
}

HRESULT CFastDIB::Stretch(double fxtime, double fytime, DWORD x, DWORD y, DWORD nWidth, DWORD nHeight, BOOL bBlur)
{
	/////////////////////////////
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);

	DWORD tmpdw = (DWORD)((m_DibInfoBack.bmWidth-x) * fxtime);
	if (nWidth > tmpdw)
		nWidth = tmpdw;
	tmpdw = (DWORD)((m_DibInfoBack.bmHeight-y) * fytime);
	if (nHeight > tmpdw)
		nHeight = tmpdw;

	////////////裁剪/////////////
	DWORD *pSrc0 = NULL, *pSrc1 = NULL, *pSrc2 = NULL, *pSrc3 = NULL;
	DWORD *tmpBuf = NULL;
	DWORD SrcWidth  = m_DibInfoBack.bmWidth;
	DWORD SrcHeight = m_DibInfoBack.bmHeight;
	DWORD *pSource  = NULL;

	if (m_DibInfo.nType == FDIBTYPE_DOUBLEBUF)
	{
		tmpBuf = (DWORD *)m_DibInfo.pvData;
		pSource = (DWORD *)m_DibInfoBack.pvData;
	}
	else
	{
		tmpBuf = new DWORD[nWidth * nHeight];
		pSource = (DWORD *)m_DibInfo.pvData;
	}
	DWORD *pDst = tmpBuf;

	register int mapy = 0;
	register int offsety = 0;
	int ytime = (int)(1/fytime*1024);
	int xtime = (int)(1/fxtime*1024);

	int tmpint;
	//////////局部变量///////////
	for (int j=nHeight-1; j >= 0; j--)
	{
		tmpint = j * ytime;
		mapy = SrcHeight - (tmpint >> 10) - y - 1;
		if (bBlur)
		{
			offsety = tmpint & 0x000003FF;
			offsety >>= 5;
		}
		for (int i = 0; i<(int)nWidth; i++)
		{
			if (j == (int)nHeight - 1)
			{
				tmpint = i * xtime;
				m_mapx[i] = (tmpint >>10) + x;
				tmpint &= 0x000003FF;
				tmpint >>= 5;
				m_offsetx[i] = tmpint;
			}

			pSrc0 = pSource + mapy * SrcWidth + m_mapx[i];

			if (!bBlur) {
				*pDst = *pSrc0;
			}
			else
			{
				if (m_mapx[i] < (int)SrcWidth)
					pSrc1 = pSrc0 + 1;
				else
					pSrc1 = pSrc0;
				if ((mapy > 1) && (m_mapx[i] < (int)SrcWidth))
					pSrc2 = pSrc0 - SrcWidth + 1;
				else
					pSrc2 = pSrc0;
				if (mapy > 1)
					pSrc3 = pSrc0 - SrcWidth;
				else
					pSrc3 = pSrc0;

				COLORTABLE *tmpct = &m_ct[m_offsetx[i]][offsety];

				__asm {
					pxor mm7, mm7

					mov EBX, tmpct;
					mov EAX, pSrc0
					movd mm0, [EAX]
					punpcklbw mm0, mm7

					mov EAX, pSrc1
					movd mm1, [EAX]
					punpcklbw mm1, mm7

					movd mm2, [EBX]tmpct.m_part0
					movd mm3, [EBX]tmpct.m_part1
					punpcklwd mm2, mm2
					punpckldq mm2, mm2
					punpcklwd mm3, mm3
					punpckldq mm3, mm3

					pmullw mm0, mm2
					pmullw mm1, mm3

					paddusw mm0, mm1
					////////////////////////
					////////////////////////
					mov EAX, pSrc2
					movd mm4, [EAX]
					punpcklbw mm4, mm7

					mov EAX, pSrc3
					movd mm1, [EAX]
					punpcklbw mm1, mm7

					movd mm2, [EBX]tmpct.m_part2
					movd mm3, [EBX]tmpct.m_part3
					punpcklwd mm2, mm2
					punpckldq mm2, mm2
					punpcklwd mm3, mm3
					punpckldq mm3, mm3

					pmullw mm4, mm2
					pmullw mm1, mm3

					paddusw mm4, mm1
					///////////////////////
					///////////////////////
					paddusw mm0, mm4

					psrlw mm0, 7
					packuswb mm0, mm7

					mov EAX, pDst
					movd [EAX], mm0

					EMMS
				}
			}
			pDst++;
		}
	}

	if (m_DibInfo.nType != FDIBTYPE_DOUBLEBUF)
	{
        if (m_DibInfo.pvData != NULL)
		    delete[] m_DibInfo.pvData;
		m_DibInfo.pvData = tmpBuf;
		m_DibInfoBack.bmWidth  = nWidth;
		m_DibInfoBack.bmHeight = nHeight;
	}
	m_DibInfo.bmWidth  = nWidth;
	m_DibInfo.bmHeight = nHeight;

	return S_OK;
}

HRESULT CFastDIB::Rotation(int nAngle, int cx, int cy, DWORD nWidth, DWORD nHeight, BOOL bBlur)
{
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);

	int mapx = 0; int mapy = 0;

	if (nWidth == 0)
		nWidth = m_DibInfoBack.bmWidth;
	if (nHeight == 0)
		nHeight = m_DibInfoBack.bmHeight;

	if (nWidth > nHeight)
		nHeight = nWidth;
	else
		nWidth = nHeight;

	if (nWidth > CFastDIB::m_nMaxWidth)
		nWidth = CFastDIB::m_nMaxWidth;
	if (nHeight > CFastDIB::m_nMaxHeight)
		nHeight = CFastDIB::m_nMaxHeight;

	int angle = 0;
	//////////裁剪////////////////
	angle = nAngle % 3600;
	if (angle < 0)
		angle = 3600 + angle;
	/////////换算角度/////////////
	DWORD *pSource = NULL;
	DWORD *pDest = NULL;
	if (m_DibInfo.nType == FDIBTYPE_DOUBLEBUF)
	{
		pSource = (DWORD*)m_DibInfoBack.pvData;
		pDest = (DWORD*)m_DibInfo.pvData;
	}
	else
	{
		pSource = (DWORD*)m_DibInfo.pvData;
		pDest = new DWORD[nWidth * nHeight];
	}
	DWORD *pSrc0 = NULL, *pSrc1 = NULL, *pSrc2 = NULL, *pSrc3 = NULL;
	int tmpint = 0;
	DWORD SrcWidth  = m_DibInfoBack.bmWidth;
	DWORD SrcHeight = m_DibInfoBack.bmHeight;
	//////////////局部变量/////////////////////
	for (int j = 0; j<(int)nHeight; j++)
	{
		pDest = (DWORD*)m_DibInfo.pvData + (nHeight-j-1) * m_DibInfoBack.bmWidth;
		for (int i = 0; i<(int)nWidth; i++)
		{
			tmpint = (i-cx) * m_at[angle].m_cospart - (j-cy) * m_at[angle].m_sinpart;
			mapx = (tmpint >> 10) + cx;
			register int offsetx = (tmpint & 0x3FF)>>5;
			tmpint = (j-cy) * m_at[angle].m_cospart + (i-cx) * m_at[angle].m_sinpart;
			mapy = (tmpint >> 10) + cy;
			register int offsety = (tmpint & 0x3FF) >> 5;
			//计算坐标映射//
			if ((mapx < 0) || (mapx > m_DibInfoBack.bmWidth-1))
			{
				*pDest = 0;
				pDest++;
				continue;
			}
			if ((mapy < 0)||(mapy > m_DibInfoBack.bmHeight-1))
			{
				*pDest = 0;
				pDest++;
				continue;
			}
			//判断边界//
			pSrc0 = pSource + (nHeight - mapy -1) * m_DibInfoBack.bmWidth + mapx;
            if (!bBlur) {
				*pDest = *pSrc0;
            }
			//不插值//
			else
			{
				if (mapx <( int)nWidth)
					pSrc1 = pSrc0 + 1;
				else
					pSrc1 = pSrc0;
				if ((mapy < (int)nHeight - 1) && (mapx < (int)nWidth))
					pSrc2 = pSrc0 - SrcWidth + 1;
				else
					pSrc2 = pSrc0;
				if (mapy < (int)nHeight - 1)
					pSrc3 = pSrc0 - SrcWidth;
				else
					pSrc3 = pSrc0;
				//插值点//
				COLORTABLE *tmpct = &m_ct[offsetx][offsety];

				__asm {
					mov EAX, pSrc0
					mov EDX, pSrc1
					pxor mm7, mm7
					movd mm0, [EAX]
					movd mm1, [EDX]
					mov EBX, tmpct;
					punpcklbw mm0, mm7
					punpcklbw mm1, mm7

					movd mm2, [EBX]tmpct.m_part0
					movd mm3, [EBX]tmpct.m_part1
					punpcklwd mm2, mm2
					punpckldq mm2, mm2
					punpcklwd mm3, mm3
					punpckldq mm3, mm3

					pmullw mm0, mm2
					pmullw mm1, mm3

					paddusw mm0, mm1
					////////////////////////
					////////////////////////
					mov EAX, pSrc2
					mov EDX, pSrc3
					movd mm4, [EAX]
					movd mm1, [EDX]
					punpcklbw mm4, mm7
					punpcklbw mm1, mm7

					movd mm2, [EBX]tmpct.m_part2
					movd mm3, [EBX]tmpct.m_part3
					punpcklwd mm2, mm2
					punpckldq mm2, mm2
					punpcklwd mm3, mm3
					punpckldq mm3, mm3

					pmullw mm4, mm2
					pmullw mm1, mm3

					paddusw mm4, mm1
					///////////////////////
					///////////////////////
					paddusw mm0, mm4

					psrlw mm0, 7
					packuswb mm0, mm7

					mov EAX, pDest
					movd [EAX], mm0

					EMMS
				}
				//插值计算//
			}
			//插值//
			pDest++;

		}
	}
	m_DibInfo.bmWidth  = nWidth;
	m_DibInfo.bmHeight = nHeight;

	if (m_DibInfo.nType != FDIBTYPE_DOUBLEBUF)
	{
        if (m_DibInfo.pvData != NULL)
		    delete[] m_DibInfo.pvData;
		m_DibInfo.pvData = pDest;
	}

	return S_OK;
}

HRESULT CFastDIB::SyncDoubleBuffer()
{
	_CHECKTYPE(GetDibInfo(), FDIBTYPE_DOUBLEBUF, TCM_BELONG);
	if (!m_bInited)
		return E_NOTINIT;

	memcpy(m_DibInfo.pvData, m_DibInfoBack.pvData, m_nMaxWidth * m_nMaxHeight * 4);

	return S_OK;
}

HRESULT CFastDIB::BitmapWipe(CFastDIB *pBufDIB, BYTE nCurrentTime, BYTE nBoardWidth, BYTE nBoardWidthColor,
							 BOOL bColorBoard, DWORD nBoardColor, DWORD x, DWORD y)
{
	_CHECKTYPE(pBufDIB->GetDibInfo(), FDIBTYPE_8BITBUF, TCM_BELONG);
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);

	//入口检查
	if (!bColorBoard)
	{
		nBoardWidthColor = 0;
		nBoardColor = 0;
	}
	else
	{
		if (nBoardWidth > 0)
		{
			if (nBoardWidthColor < nBoardWidth)
				nBoardWidth = nBoardWidthColor;
			else
				nBoardWidthColor = nBoardWidth;
		}
	}
	DWORD SrcWidth   = pBufDIB->GetDibInfo()->bmWidth;
	DWORD SrcHeight  = pBufDIB->GetDibInfo()->bmHeight;
	DWORD DstWidth   = m_DibInfoBack.bmWidth;
	DWORD DstHeight  = m_DibInfoBack.bmHeight;
	DWORD DstWidth4  = DstWidth  << 2;
	DWORD DstHeight4 = DstHeight << 2;
	DWORD nWidth  = SrcWidth;
	DWORD nHeight = SrcHeight;
	if (nWidth + x > DstWidth)
		nWidth = DstWidth - x;
	if (nHeight + y > DstHeight)
		nHeight = DstHeight - y;
	BYTE *pMap = (BYTE *)pBufDIB->GetDibInfo()->pvData + (SrcHeight-1) * SrcWidth;
	DWORD *pSrc = NULL;
	DWORD *pDst = (DWORD *)m_DibInfo.pvData + (DstHeight-y-1) * DstWidth + x;
	DWORD *pSrcoffset = NULL;
	if (m_DibInfo.nType != FDIBTYPE_DOUBLEBUF)
	{
		pSrc = (DWORD *)m_DibInfo.pvData + (DstHeight-y-1) * DstWidth + x;
	}
	else
	{
		pSrc = (DWORD *)m_DibInfoBack.pvData + (DstHeight-y-1) * DstWidth + x;
	}
	BYTE bl[256];
	BYTE bl2[256];
	BYTE *pbl = bl, *pbl2 = bl2;
	BYTE nBoardWidthHalf = nBoardWidth >> 1;
	BYTE nBoardWidthColorHalf = nBoardWidthColor >> 1;

	for (int k = 0; k<nBoardWidth; k++) {
		bl[k] = (k*255/nBoardWidth);
	}

	for (int l = 0; l<nBoardWidthColor; l++)
	{

		if ((l<nBoardWidthColorHalf)||(nBoardWidth == 0))
			bl2[l] = 255;//nBoardWidth*255/nBoardWidthHalf;
		else
			bl2[l] = (nBoardWidthColor-l)*255/nBoardWidthColorHalf;
	}


	DWORD dwTmp = 0, dwTmp2 = 0;
	BYTE tmpByte = 0, tmpByte2 = 0, tmpByte3 = 0;
	DWORD dwCurrentTime = nCurrentTime;
	__int64 AlphaMask = 0x00FF00FF00FF00FF;
	int j = 0, i = 0;
	__asm {
		mov j, 0
		mov EBX, pMap
		mov ECX, pSrc
		mov EDX, pDst

LOOPj:
		mov i, 0
LOOPi:
		mov AL, BYTE PTR [EBX]
		cmp AL, nCurrentTime
		jb LABEL1
		mov tmpByte, 0
		jmp LABEL2
LABEL1: mov tmpByte, 255
		mov AH, nCurrentTime
		sub AH, AL
		mov tmpByte2, AH
		cmp AH, nBoardWidth
		jb LABEL3
		jmp LABEL4

LABEL3: mov EAX, 0
		mov AL, tmpByte2
		add EAX, pbl
		mov AL, [EAX]
		mov tmpByte, AL

LABEL4: mov EAX, 0
		mov AL, tmpByte2
		add EAX, pbl2
		mov AL, [EAX]
		mov tmpByte3, AL

		mov AH, tmpByte2
		cmp AH, nBoardWidthColor
		jb LABEL5
		jmp LABEL2

LABEL5:
		pxor mm7, mm7
		movd mm0, nBoardColor
		movd mm1, [ECX]
		punpcklbw mm0, mm7
		punpcklbw mm1, mm7
		mov EAX, 0
		mov AL, tmpByte3
		movd mm2, EAX
		punpcklwd mm2, mm2
		punpckldq mm2, mm2
		movq mm3, AlphaMask
		psubusw mm3, mm2
		pmullw mm0, mm2
		pmullw mm1, mm3
		paddusw mm0, mm1
		psrlw mm0, 8
		packuswb mm0, mm0
		movd EAX, mm0
		EMMS

		// mov EAX, nBoardColor
		mov [EDX], EAX
		jmp LABEL6

LABEL2: mov EAX, [ECX]
		jmp LABEL7

LABEL6: mov EAX, [EDX]

LABEL7: mov dwTmp, EAX
		shr EAX, 24
		mul tmpByte
		shr AX, 8
		shl EAX, 24;
		and dwTmp, 0x00FFFFFF
		or EAX, dwTmp
		mov [EDX], EAX

		add EBX, 1
		add ECX, 4
		add EDX, 4

		inc i;
		mov EAX, i
		cmp EAX, nWidth
		jb LOOPi

		mov EBX, pMap
		mov ECX, pSrc
		mov EDX, pDst

		sub EBX, SrcWidth
		sub ECX, DstWidth4
		sub EDX, DstWidth4

		mov pMap, EBX
		mov pSrc, ECX
		mov pDst, EDX

		inc j
		mov EAX, j
		cmp EAX, nHeight
		jb LOOPj

	}
	return S_OK;
}

HRESULT CFastDIB::ColorTransparency(PCOLORTRANSPARENCY pctr)
{
	//检查入口
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);
	//.........
	//源内存、目标内存
	DWORD *pSrc = NULL;
	if (m_DibInfo.nType == FDIBTYPE_DOUBLEBUF)
		pSrc = (DWORD *)m_DibInfoBack.pvData;
	else
		pSrc = (DWORD *)m_DibInfo.pvData;
	DWORD *pDst = (DWORD *)m_DibInfo.pvData;
	//.........
	//局部变量
	DWORD *pHslTable = CFastDIB::m_hslTable; //RGB转HSL色彩映射表
	int hslindex = 0; //RGB转HSL色彩映射表索引
	DWORD HSLValue = 0;
	WORD H = 0;
	BYTE S = 0;
	BYTE L = 0;
	BYTE A = 0;
	//.........
	if (pctr->m_nBoardBlur>pctr->m_wHueRange)
		pctr->m_nBoardBlur = pctr->m_wHueRange;
	//计算色彩边缘柔化表、亮度高门柔化表、亮度低门柔化表
	BYTE bl[181];
	BYTE blHigh[255];
	BYTE blLow[255];
	int i = 0;
	//色彩边缘柔化表
	if (pctr->m_bColorKey)
	{
		for (i = 0; i<pctr->m_nBoardBlur; i++) {
			bl[i] = (BYTE)((i<<8)/pctr->m_nBoardBlur);
		}
	}
	//亮度高门柔化、低门柔化表
	if (pctr->m_bLightKey)
	{
		for (i = 0; i<pctr->m_nHighLevelBlur; i++)
			blHigh[i] = (BYTE)(((pctr->m_nHighLevelBlur-i)<<8)/pctr->m_nHighLevelBlur);
		for (i = 0; i<pctr->m_nLowLevelBlur; i++)
			blLow[i] = (BYTE)(((pctr->m_nLowLevelBlur-i)<<8)/pctr->m_nLowLevelBlur);
	}
	/////////////////////////////////////////////////////
	//抠象计算
	for (int j = 0; j<m_DibInfoBack.bmHeight; j++)
	{
		for (i = 0; i<m_DibInfoBack.bmWidth; i++)
		{
			//将8x4位色转换成6x3位色以作为RGB转HSL色彩映射表（近似）索引
			__asm {
				mov EAX, pSrc
				mov ECX, [EAX]; CH<<G CL<<B
				mov EBX, ECX
				shr EBX, 16; BL<<R
				mov A, BH

				mov EDX, 0
				mov EAX, 0
				shr BL, 2
				movzx EDX, BL
				or EAX, EDX
				shl EAX, 6

				shr CH, 2
				movzx EDX, CH
				or EAX, EDX
				shl EAX, 6

				shr CL, 2
				movzx EDX, CL
				or EAX, EDX

				mov hslindex, EAX
			}
			//..........................
			//由索引通过查表获取当前颜色所对应的HSL值
			HSLValue = CFastDIB::m_hslTable[hslindex];
			H = (WORD) (HSLValue & 0x0000FFFF);
			S = (BYTE)((HSLValue & 0x00FF0000) >> 16);
			L = (BYTE)((HSLValue & 0xFF000000) >> 24);
			//..........................
			BYTE nAlphaValue = 0xFF;
			BYTE nAlphaValueColor = 0xFF;
			BYTE nAlphaValueLight = 0xFF;
			BOOL bInColorKey = FALSE;
			//通过色键计算Alpha值
			if (pctr->m_bColorKey)
			{
				short nHueDif = abs(pctr->m_wHue-H);
				if (nHueDif>180) nHueDif = 360-nHueDif;
				if ((nHueDif<pctr->m_wHueRange) && (S>pctr->m_nSaturation))
				{
					if (nHueDif>(pctr->m_wHueRange-pctr->m_nBoardBlur))
						nAlphaValueColor = bl[nHueDif-(pctr->m_wHueRange-pctr->m_nBoardBlur)];
					else
						nAlphaValueColor = 0;
					bInColorKey = TRUE;
				}
			}
			//通过亮键计算Alpha值
			if (pctr->m_bLightKey)
			{
				BYTE nLightDifHigh = abs(L-pctr->m_nHighLevelDoor);
				BYTE nLightDifLow  = abs(L-pctr->m_nLowLevelDoor);
				if ((L<pctr->m_nHighLevelDoor) && (L>pctr->m_nLowLevelDoor))
				{
					if (nLightDifHigh<pctr->m_nHighLevelBlur)
					{
						nAlphaValueLight = blHigh[nLightDifHigh];
						nAlphaValueColor = 255;
					}
					else
						if (nLightDifLow<pctr->m_nLowLevelBlur)
						{
							nAlphaValueLight = blLow[nLightDifLow];
							nAlphaValueColor = 255;
						}
						else
							nAlphaValueLight = 0;
				}
				else
				{
					nAlphaValueColor = 255;
				}
			}
			//合并色键、亮键所产生的效果
			if ((pctr->m_bColorKey) && (pctr->m_bLightKey))
			{
				if (bInColorKey)
					nAlphaValue = (nAlphaValueColor * nAlphaValueLight) >> 8;
				else
					nAlphaValue = 255;
			}
			else
			{
				if (pctr->m_bColorKey)
					nAlphaValue = nAlphaValueColor;
				if (pctr->m_bLightKey)
					nAlphaValue = nAlphaValueLight;
			}
			//将色键、亮键的抠象结构应用到目标图象
			nAlphaValue = A * nAlphaValue >> 8;
			*pDst = *pSrc;
			*pDst &= 0x00FFFFFF;
			*pDst |= (nAlphaValue<<24);

			pDst++;
			pSrc++;
		}
	}

	return S_OK;
}

HRESULT CFastDIB::Paint(PPAINTPARAM pppm)
{
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);
	DWORD *pDst = (DWORD *)m_DibInfo.pvData;
	DWORD *pSrc = NULL;
	if (m_DibInfo.nType == FDIBTYPE_DOUBLEBUF)
		pSrc = (DWORD *)m_DibInfoBack.pvData;
	else
		pSrc = (DWORD *)m_DibInfo.pvData;

	WORD H = 0;
	BYTE S = 0;
	BYTE L = 0;
	DWORD HSLValue;
	BYTE R = 0, G = 0, B = 0, A = 0;
	DWORD hslindex = 0;
	DWORD *pTempcolor = new DWORD[CFastDIB::m_nMaxWidth];
	DWORD dwTmpSrc = 0;
	for (int j = 0; j<m_DibInfo.bmHeight; j++)
	{
		for (int i = 0; i<m_DibInfo.bmWidth; i++)
		{
			//计算马赛克效果
			if ((pppm->m_nMosaicHorizontal > 0) && (pppm->m_nMosaicVertical > 0))
			{
				if ((j % pppm->m_nMosaicVertical) == 0)
				{
					if ((i % pppm->m_nMosaicHorizontal) == 0)
						pTempcolor[i] = *pSrc;
					else
						pTempcolor[i] = pTempcolor[i-1];
				}
				dwTmpSrc = pTempcolor[i];
			}
			else
				dwTmpSrc = *pSrc;
			/////////////////////////////////////////////////////
			//RGB转换为HSL
			__asm {
				mov ECX, dwTmpSrc; CH<<G CL<<B
				mov EBX, ECX
				shr EBX, 16; BL<<R
				mov A, BH

				mov EDX, 0
				mov EAX, 0
				shr BL, 2
				movzx EDX, BL
				or EAX, EDX
				shl EAX, 6

				shr CH, 2
				movzx EDX, CH
				or EAX, EDX
				shl EAX, 6

				shr CL, 2
				movzx EDX, CL
				or EAX, EDX

				mov hslindex, EAX
			}
			//..........................
			//由索引通过查表获取当前颜色所对应的HSL值
			HSLValue = CFastDIB::m_hslTable[hslindex];
			H = (WORD) (HSLValue & 0x0000FFFF);
			S = (BYTE)((HSLValue & 0x00FF0000) >> 16);
			L = (BYTE)((HSLValue & 0xFF000000) >> 24);
			//..........................
			//色彩亮度处理/////////////////////////

			//Add Code here

			///////////////////////////////////////
			//HSL转回RGB
			double tmp1 = 0, tmp2 = 0, tmp3[3]={0, 0, 0};
			BYTE ncolor[3] = {0, 0, 0};
			double fS = S/240.0, fH = H/360.0, fL = L/240.0;
			if (S == 0)
			{
				R = L;
				G = L;
				B = L;
			}
			else
			{
				if (L < 0.5)
					tmp2 = fL * (1.0 + fS);
				else
					tmp2 = fL + fS - fL * fS;

				tmp1 = 2.0 * fL - tmp2;

				tmp3[0] = fH + 0.33;
				if (tmp3[0] > 1.0)
					tmp3[0] = tmp3[0] - 1.0;
				tmp3[1] = fH;
				tmp3[2] = fH - 0.33;
				if (tmp3[2] < 0.0)
					tmp3[2] += 1.0;

				for (int k = 0; k<3; k++)
				{
					if (tmp3[k] * 6.0 < 1.0)
						ncolor[k] = (BYTE)((tmp1 + (tmp2-tmp1) * 6.0 * tmp3[k]) * 255);
					else
						if (2.0 * tmp3[k] < 1.0)
							ncolor[k] = (BYTE)(tmp2 * 255);
						else
							if (3.0 * tmp3[k] < 2.0)
								ncolor[k] = (BYTE)((tmp1 + (tmp2-tmp1) * 0.67 * tmp3[k] * 6.0) * 255);
				}

				dwTmpSrc = ((((0xFF00 | ncolor[0]) << 8) | ncolor[1]) << 8) | ncolor[2];

			}
			///////////////////////////////////////
			*pDst = dwTmpSrc;

			pSrc++;
			pDst++;
		}
	}
    if (pTempcolor)
	    delete[] pTempcolor;
	return S_OK;
}
HRESULT CFastDIB::QuickPaint(PPAINTPARAM pppm)
{
	_CHECKTYPE(GetDibInfo(), (FDIBTYPE_RGBA|FDIBTYPE_DOUBLEBUF), TCM_BELONG);
	DWORD *pDst = (DWORD *)m_DibInfo.pvData;
	DWORD *pSrc = NULL;
	if (m_DibInfo.nType == FDIBTYPE_DOUBLEBUF)
		pSrc = (DWORD *)m_DibInfoBack.pvData;
	else
		pSrc = (DWORD *)m_DibInfo.pvData;

	DWORD *pTempColor = new DWORD[CFastDIB::m_nMaxWidth];
	DWORD dwTmpSrc = 0;
	BYTE A = 0, R = 0, G = 0, B = 0;
	for (int j = 0; j<m_DibInfo.bmHeight; j++)
	{
		for (int i = 0; i<m_DibInfo.bmWidth; i++)
		{
			__asm {
				mov EAX, pSrc
				mov EAX, [EAX]
				mov B, AL
				mov G, AH
				shr EAX, 16
				mov R, AL
				mov A, AH
			}
			if (pppm->m_bNoise)
			{
				R = rand() % 255;
				G = rand() % 255;
				B = rand() % 255;
				__asm {
					mov AL, R
					mov BH, G
					mov BL, B
					shl EAX, 16
					mov AX, BX
					mov dwTmpSrc, EAX
				}
			}
			else
				dwTmpSrc = *pSrc;

			//计算马赛克效果
			if ((pppm->m_nMosaicHorizontal > 0) && (pppm->m_nMosaicVertical > 0))
			{
				if ((j % pppm->m_nMosaicVertical) == 0)
				{
					if ((i % pppm->m_nMosaicHorizontal) == 0)
						pTempColor[i] = dwTmpSrc;
					else
						pTempColor[i] = pTempColor[i-1];
				}
				dwTmpSrc = pTempColor[i];
				__asm {
					mov EAX, dwTmpSrc
					mov B, AL
					mov G, AH
					shr EAX, 16
					mov R, AL
				}
			}

			if (pppm->m_bReverse)
			{
				__asm {
					mov EAX, dwTmpSrc
					xor EAX, 0xFFFFFFFF
					mov dwTmpSrc, EAX
					mov B, AL
					mov G, AH
					shr EAX, 16
					mov R, AL
				}
			}

			if (pppm->m_nLightOffset!=255)
			{
				__int64 submask = 0x00FF00FF00FF00FF;
				__asm {
					pxor mm7, mm7
					movq mm2, submask
					xor EBX, EBX
					mov EAX, pppm
					mov BX, [EAX]PAINTPARAM.m_nLightOffset
					movd mm0, EBX
					movd mm1, dwTmpSrc
					punpckldq mm0, mm0
					punpcklbw mm1, mm7
					movq mm3, mm0
					psllq mm3, 16
					por mm0, mm3
					psubsw mm0, submask
					paddsw mm0, mm1
					packuswb mm0, mm7
					movd EAX, mm0

					mov dwTmpSrc, EAX
					mov B, AL
					mov G, AH
					shr EAX, 16
					mov R, AL

					EMMS
				}
			}

			if (pppm->m_nLightPlus!=64)
			{
				__asm {
					pxor mm7, mm7
					mov EAX, pppm
					movzx EAX, [EAX]PAINTPARAM.m_nLightPlus
					movd mm0, EAX
					movd mm1, dwTmpSrc
					punpckldq mm0, mm0
					punpcklbw mm1, mm7
					movq mm3, mm0
					psllq mm3, 16
					por mm0, mm3
					pmullw mm1, mm0
					psrlw mm1, 6
					packuswb mm1, mm7
					movd EAX, mm1

					mov dwTmpSrc, EAX
					mov B, AL
					mov G, AH
					shr EAX, 16
					mov R, AL

					EMMS
				}
			}

			if (pppm->m_nColorPlus!=64)
			{
				__int64 submask = 0x0040004000400040;
				__asm {
					mov EAX, pppm
					movzx EDX, [EAX]PAINTPARAM.m_nColorPlus
					pxor mm7, mm7
					movzx AX, R
					movzx BX, G
					add AX, BX
					movzx BX, B
					add AX, BX
					mov BL, 3
					div BL
					mov ECX, 0
					mov CL, AL
					movd mm0, ECX; mm0<-(R+G+B)/3 "colorAV"
					movd mm1, dwTmpSrc
					punpckldq mm0, mm0
					movq mm4, mm0
					psllq mm4, 16
					por mm0, mm4

					movd mm3, EDX
					punpckldq mm3, mm3
					movq mm4, mm3
					psllq mm4, 16
					por mm3, mm4; mm4<-PAINTPARAM.m_nColorPlus

					punpcklbw mm1, mm7
					movq mm5, mm1; mm5<-A,R,G,B
					psubsw mm1, mm0; mm1<-(R-colorAV), (G-colorAV), (B-colorAV)

					psubsw mm3, submask
					pmullw mm3, mm1
					psraw mm3, 6
					paddsw mm3, mm5
					packuswb mm3, mm7

					movd EAX, mm3
					mov dwTmpSrc, EAX
					mov B, AL
					mov G, AH
					shr EAX, 16
					mov R, AL

					EMMS
				}

				/*
				int tmpint = 0;
				int colorAV = (R+G+B)/3;
				short rX = (R-colorAV);
				short gX = (G-colorAV);
				short bX = (B-colorAV);
				tmpint = (int)(R+((pppm->m_nColorPlus-255)*rX>>8));
				if (tmpint>255) tmpint=255;
				else
				if (tmpint<0) tmpint = 0;
				R = (BYTE)tmpint;
				tmpint = (int)(G+((pppm->m_nColorPlus-255)*gX>>8));
				if (tmpint>255) tmpint=255;
				else
				if (tmpint<0) tmpint = 0;
				G = (BYTE)tmpint;
				tmpint = (int)(B+((pppm->m_nColorPlus-255)*bX>>8));
				if (tmpint>255) tmpint=255;
				else
				if (tmpint<0) tmpint = 0;
				B = (BYTE)tmpint;
				dwTmpSrc = (((((A << 8) | R) << 8) | G) << 8) | B;
				A = (BYTE)((dwTmpSrc&0xFF000000)>>24);
				R = (BYTE)((dwTmpSrc&0x00FF0000)>>16);
				G = (BYTE)((dwTmpSrc&0x0000FF00)>>8);
				B = (BYTE)(dwTmpSrc&0x000000FF);
				//*/
			}
			if (pppm->m_bColorMask)
			{
				__int64 colormask = 0x00BF00BF00BF00BF;
				__asm {
					pxor mm7, mm7
					movzx AX, R
					movzx BX, G
					add AX, BX
					movzx BX, B
					add AX, BX
					mov BL, 3
					div BL
					mov EBX, 0
					mov BL, AL
					movd mm0, EBX; mm0<-(R+G+B)/3
					mov EAX, pppm
					mov EAX, [EAX]PAINTPARAM.m_dwColorMask
					movd mm1, EAX
					punpckldq mm0, mm0
					movq mm3, mm0
					psllq mm3, 16
					por mm0, mm3
					punpcklbw mm1, mm7
					pand mm1, colormask
					paddusw mm0, mm1
					packuswb mm0, mm7
					movd EAX, mm0
					mov dwTmpSrc, EAX
					mov B, AL
					mov G, AH
					shr EAX, 16
					mov R, AL
					EMMS
				}
				/*
				int tmpint = 0;
				BYTE colorAV = (R+G+B)/3;
				tmpint = ((pppm->m_dwColorMask&0x00BF0000)>>16)+colorAV;
				if (tmpint>255) tmpint = 255;
				if (tmpint<0) tmpint = 0;
				R = tmpint;
				tmpint = ((pppm->m_dwColorMask&0x0000BF00)>>8)+colorAV;
				if (tmpint>255) tmpint = 255;
				if (tmpint<0) tmpint = 0;
				G = tmpint;
				tmpint = ((pppm->m_dwColorMask&0x000000BF))+colorAV;
				if (tmpint>255) tmpint = 255;
				if (tmpint<0) tmpint = 0;
				B = tmpint;
				dwTmpSrc = (((((A << 8) | R) << 8) | G) << 8) | B;
				//*/
			}

			if (pppm->m_nColorRange > 0)
			{
				__asm {
					pxor mm7, mm7
					mov EBX, 0
					mov EAX, pppm
					mov BL, [EAX]PAINTPARAM.m_nColorRange;
					movd mm0, EBX
					movd mm1, dwTmpSrc
					punpcklbw mm1, mm7
					psrlw mm1, mm0
					psllw mm1, mm0
					packuswb mm1, mm7
					movd EAX, mm1
					mov dwTmpSrc, EAX
					mov B, AL
					mov G, AH
					shr EAX, 16
					mov R, AL
					EMMS
				}
			}
			//#ifndef _SATURATION255

			//#endif

			if (pppm->m_nColorOffset != 255)
			{
				BYTE nMax = R;
				int tmpint = 0;
				if (G > nMax) nMax = G;
				if (B > nMax) nMax = B;
				if (R == nMax)
				{
					tmpint = G + ((pppm->m_nColorOffset - 255));
					_SATURATION255(tmpint);
					G = tmpint;
					tmpint = R - ((pppm->m_nColorOffset - 255));
					_SATURATION255(tmpint);
					R = tmpint;
				}
				if (G == nMax)
				{
					tmpint = B + ((pppm->m_nColorOffset - 255));
					_SATURATION255(tmpint);
					B = tmpint;
					tmpint = G - ((pppm->m_nColorOffset - 255));
					_SATURATION255(tmpint);
					G = tmpint;
				}
				if (B == nMax)
				{
					tmpint = R + ((pppm->m_nColorOffset - 255));
					_SATURATION255(tmpint);
					R = tmpint;
					tmpint = B - ((pppm->m_nColorOffset - 255));
					_SATURATION255(tmpint);
					B = tmpint;
				}
				dwTmpSrc = (((((A << 8) | R) << 8) | G) << 8) | B;
			}

			///////////////////////////////////////
			*pDst = (dwTmpSrc & 0x00FFFFFF) | (A << 24);

			pSrc++;
			pDst++;
		}
	}
    if (pTempColor)
	    delete[] pTempColor;
	return S_OK;
}

HRESULT CFastDIB::ClearImage()
{
	FASTDIBTYPE tmpType = m_DibInfo.nType & 0x0000FFFF;
	switch (tmpType)
	{
	case FDIBTYPE_8BITBUF:
		ZeroMemory(m_DibInfo.pvData, m_DibInfo.bmWidth * m_DibInfo.bmHeight);
		break;
	case FDIBTYPE_16BITBUF:
		ZeroMemory(m_DibInfo.pvData, m_DibInfo.bmWidth * m_DibInfo.bmHeight * 2);
		break;
	case FDIBTYPE_RGBA:
		ZeroMemory(m_DibInfo.pvData, m_DibInfo.bmWidth * m_DibInfo.bmHeight * 4);
		break;
	case FDIBTYPE_DOUBLEBUF:
		ZeroMemory(m_DibInfo.pvData, CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight * 4);
		ZeroMemory(m_DibInfoBack.pvData, CFastDIB::m_nMaxWidth * CFastDIB::m_nMaxHeight * 4);
		break;
	}
	return S_OK;
}

HRESULT CFastDIB::Fade(BYTE nThreshold)
{
	_CHECKTYPE(GetDibInfo(), FDIBTYPE_DOUBLEBUF, TCM_BELONG);
	DWORD *pSrc = (DWORD *)m_DibInfoBack.pvData;
	DWORD *pDst = (DWORD *)m_DibInfo.pvData;
	for (int j = 0; j<(int)CFastDIB::m_nMaxHeight; j++)
	{
		for (int i = 0; i<(int)CFastDIB::m_nMaxWidth; i++)
		{
			__asm {
				mov ECX, pSrc
				mov EDX, [ECX]
				mov EAX, EDX
				shr EAX, 24
				mov BL,  nThreshold
				mul BL
				shr EAX, 8
				shl EAX, 24
				and EDX, 0x00FFFFFF
				or  EDX, EAX
				mov ECX, pDst
				mov [ECX], EDX
			}
			pSrc++;
			pDst++;
		}
	}
	return S_OK;
}
