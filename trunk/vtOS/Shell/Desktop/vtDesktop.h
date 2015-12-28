#pragma once

#include "..\..\..\FacePad\stdafx.h"
#include "..\..\Kernel\vtCommon.h"
#include "..\..\Kernel\vtObject.h"
//#include <afxwin.h>
#include "vtShortCut.h"
#include "vtWallPaper.h"

typedef enum _VT_ARRANGEMENT_MODE {
	VT_CELL_LEFT2RIGHT_UP2DOWN,
	VT_CELL_LEFT2RIGHT_DOWN2UP,
	VT_CELL_RIGHT2LEFT_UP2DOWN,
	VT_CELL_RIGHT2LEFT_DOWN2UP,
	VT_CELL_UP2DOWN_LEFT2RIGHT,
	VT_CELL_UP2DOWN_RIGHT2LEFT,
	VT_CELL_DOWN2UP_LEFT2RIGHT,
	VT_CELL_DOWN2UP_RIGHT2LEFT,
	VT_CELL_ARRANGEMENT_MAX
} VT_ARRANGEMENT_MODE;

typedef struct _VT_DESKTOP_INFO {
	LONG width;
	LONG height;
    LONG align;

	LONG paddingLeft;
	LONG paddingRight;
	LONG paddingTop;
	LONG paddingBottom;

	LONG clientWidth;
	LONG clientHeight;
	LONG clientAlignH;
	LONG clientAlignV;

	LONG cellSpacingX;
	LONG cellSpacingY;

    LONG iconWidth;
    LONG iconHeight;

    LONG iconSpacingLeft;
    LONG iconSpacingRight;
    LONG iconSpacingTop;
    LONG iconSpacingBottom;

    LONG iconTextWidth;
    LONG iconTextHeight;
    LONG iconTextSpacingX;
    LONG iconTextSpacingY;
    LONG iconTextLines;

	LONG iconArrangement;
	UINT iconAutoAlign;
	UINT iconAlignToGrid;

    COLORREF bgcolor;

	VT_WALLPAPER_INFO wallPaper;
} VT_DESKTOP_INFO,  *PVT_DESKTOP_INFO,  *LPVT_DESKTOP_INFO;
#define _VT_DESKTOP_INFO_

class vtDesktop : public vtObject, public VT_DESKTOP_INFO
{
public:
	vtDesktop(void);
	virtual ~vtDesktop(void);

    vtDesktop(HWND hwnd);
    vtDesktop(CWnd *pWnd);

public:
    BOOL Attach(HWND hwnd);
    BOOL Attach(CWnd *pWnd);
    HWND Detach(void);

    void DeleteObject(void);

    void PaintDesktop(CDC *pDC, CRect *lpRect, UINT flags);
    void PaintDesktopBackground(CDC *pDC, CRect *lpRect, UINT flags);
    void PaintDesktopIconList(CDC *pDC, CRect *lpRect, UINT flags);
    //void PaintDesktopIconList(HWND hwnd, HDC hdc, RECT *lprect, UINT flags);

    void PaintDesktopIcon(CDC *pDC, CRect *lpRect, vtShortCut *shortcut, int index, UINT flags);

    void LoadDesktopBitmaps(HWND hwnd = NULL);
    void LoadDesktopBitmaps(HWND hwnd, int _width, int _height);
    void FreeDesktopBitmaps(void);

    int initShortCutList(void);

protected:
    void initDesktop(HWND hwnd, int _width = -1, int _height = -1);
	void initDesktopData(int _width, int _height);
	void initWallPaperData(int _width, int _height);

public:
	//

private:
    HWND            m_hWnd;
	vtShortCut      m_ShortCut;
	vtShortCutList  shortCutList;

    CBitmap         *m_pbmpBGBuf;
    CBitmap         *m_pbmpBG;
    CBitmap         *m_pbmpWallPaper;
};
