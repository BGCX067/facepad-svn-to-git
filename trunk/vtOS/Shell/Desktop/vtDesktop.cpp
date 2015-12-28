
#include "..\..\..\FacePad\stdafx.h"
#include "vtDesktop.h"
#include "..\..\Kernel\Memory\cache_aligned.h"
#include "..\..\Kernel\MultiThreading\atomic_t.h"

vtDesktop::vtDesktop(void)
{
    initDesktop(NULL, WIN_DESKTOP_DEFAULT_WIDTH, WIN_DESKTOP_DEFAULT_HEIGHT);

    cache_aligned_t a1(50, 63, false);
    a1.Malloc(127);

    cache_aligned_t a2(31, 128);
    cache_aligned_t a3;
    cache_aligned_t a4(1999);
    cache_aligned_t a5(a1);
    a2.Malloc(63);
    a2.Realloc(0);
    a1.Realloc(1);
    a1.Realloc(2);
    a2.Realloc(128);

    void *pBuffer = a2.GetPtr();
    a2.MemSet(0);
    a3 = a2;
    a1.Free();
    //a5.Free(true);
    pBuffer = a1.AllocPtr();
    if (pBuffer != NULL)
        free(pBuffer);

    pBuffer = a5.GetPtr();
    if (pBuffer != NULL)
        cache_aligned_t::FreeBlock(pBuffer);

    //if (pBuffer != NULL)
    //   cache_aligned_t::FreeBlock(pBuffer);

    atomic_t atomic;
    atomic_init(&atomic, 0);
    atomic_inc(&atomic);
    atomic_set_return(&atomic, 8);
    atomic_sub_return(3, &atomic);

    pBuffer = _aligned_malloc(54, 64);
    if (pBuffer != NULL)
        _aligned_free(pBuffer);
}

vtDesktop::vtDesktop( HWND hwnd )
{
    initDesktop(hwnd);
}

vtDesktop::vtDesktop( CWnd *pWnd )
{
    if (pWnd != NULL)
        initDesktop(pWnd->GetSafeHwnd());
}

vtDesktop::~vtDesktop(void)
{
    Detach();
}

void vtDesktop::initDesktop( HWND hwnd, int _width, int _height )
{
    m_hWnd = hwnd;

    if (_width < 0)
        _width = WIN_DESKTOP_DEFAULT_WIDTH;
    if (_height < 0)
        _height = WIN_DESKTOP_DEFAULT_HEIGHT;
	initDesktopData(_width, _height);

    m_pbmpBGBuf = NULL;
    m_pbmpBG = NULL;
    m_pbmpWallPaper = NULL;
}

void vtDesktop::initDesktopData( int _width, int _height )
{
	width   = _width;
	height  = _height;
	align   = VT_ALIGN_CENTER;

	paddingLeft   = 0;
	paddingRight  = 0;
	paddingTop    = 0;
	paddingBottom = 0;

	clientWidth   = 0;
	clientHeight  = 0;
	clientAlignH  = 0;
	clientAlignV  = 0;

    cellSpacingX  = 0;
    cellSpacingY  = 0;

    iconWidth  = WIN_DESKTOP_ICON_DEFAULT_WIDTH;
    iconHeight = WIN_DESKTOP_ICON_DEFAULT_HEIGHT;

	iconSpacingLeft   = 0;
	iconSpacingRight  = 0;
	iconSpacingTop    = 0;
	iconSpacingBottom = 0;

    iconTextWidth     = 0;
    iconTextHeight    = 0;
    iconTextSpacingX  = 0;
    iconTextSpacingY  = 0;
    iconTextLines     = 2;

	iconArrangement   = VT_CELL_LEFT2RIGHT_UP2DOWN;
	iconAutoAlign     = TRUE;
	iconAlignToGrid   = TRUE;

    bgcolor = WIN_2003_DESKTOP_DEFAULT_BGCOLOR;

	initWallPaperData(_width, _height);
}

void vtDesktop::initWallPaperData( int _width, int _height )
{
	wallPaper.width   = _width;
	wallPaper.height  = _height;
	wallPaper.align   = VT_ALIGN_CENTER;
	wallPaper.bgcolor = bgcolor;

	_tcscpy_s(wallPaper.cPictureFile, _countof(wallPaper.cPictureFile), _T(""));
}

BOOL vtDesktop::Attach( HWND hwnd )
{
    if (hwnd == NULL)
        return FALSE;

    m_hWnd = hwnd;

    LoadDesktopBitmaps(hwnd);
    return ::IsWindow(hwnd);
}

BOOL vtDesktop::Attach( CWnd *pWnd )
{
    if (pWnd != NULL) {
        return Attach(pWnd->GetSafeHwnd());
    }
    else
        return FALSE;
}

HWND vtDesktop::Detach( void )
{
    HWND hOldWnd = m_hWnd;
    DeleteObject();
    m_hWnd = NULL;
    return hOldWnd;
}

void vtDesktop::DeleteObject( void )
{
    FreeDesktopBitmaps();
}

int vtDesktop::initShortCutList( void )
{
	vtShortCut shortCut;
	shortCut.SetLinkName(_T("我的电脑"));
	shortCutList.Add(&shortCut);

	shortCut.SetLinkName(_T("我的文档"));
	shortCutList.Add(&shortCut);

	shortCut.SetLinkName(_T("网上邻居"));
	shortCutList.Add(&shortCut);

	shortCut.SetLinkName(_T("回收站"));
	shortCutList.Add(&shortCut);

	return shortCutList.SizeOf();
}

void vtDesktop::LoadDesktopBitmaps( HWND hwnd, int _width, int _height )
{
    FreeDesktopBitmaps();

    CDC *pDC = NULL;
    if (hwnd == NULL)
        hwnd = GetDesktopWindow();

    ASSERT(::IsWindow(hwnd));
    pDC = CDC::FromHandle(::GetDC(hwnd));
    ASSERT(pDC != NULL);
    if (pDC == NULL)
        return;

    CBitmap *pOldBmp = NULL;
    CDC dcMem;
    dcMem.CreateCompatibleDC(pDC);

    m_pbmpBGBuf = new CBitmap;
    if (m_pbmpBGBuf != NULL) {
        m_pbmpBGBuf->CreateCompatibleBitmap(pDC, _width, _height);
        pOldBmp = dcMem.SelectObject((CBitmap *)m_pbmpBGBuf);
        dcMem.FillSolidRect(0, 0, _width, _height, RGB(0, 0, 0));
        if (pOldBmp != NULL) {
            dcMem.SelectObject(pOldBmp);
            pOldBmp = NULL;
        }
    }

    m_pbmpBG = new CBitmap;
    if (m_pbmpBG != NULL) {
        m_pbmpBG->CreateCompatibleBitmap(pDC, _width, _height);
        pOldBmp = dcMem.SelectObject((CBitmap *)m_pbmpBG);
        dcMem.FillSolidRect(0, 0, _width, _height, bgcolor);
        if (pOldBmp != NULL) {
            dcMem.SelectObject(pOldBmp);
            pOldBmp = NULL;
        }
    }

    m_pbmpWallPaper = new CBitmap;
    if (m_pbmpWallPaper != NULL) {
        m_pbmpWallPaper->CreateCompatibleBitmap(pDC, _width, _height);
        pOldBmp = dcMem.SelectObject((CBitmap *)m_pbmpWallPaper);
        dcMem.FillSolidRect(0, 0, _width, _height, bgcolor);
        if (pOldBmp != NULL) {
            dcMem.SelectObject(pOldBmp);
            pOldBmp = NULL;
        }
    }
    dcMem.DeleteDC();

    ::ReleaseDC(hwnd, pDC->m_hDC);
    pDC->DeleteDC();
}

void vtDesktop::LoadDesktopBitmaps( HWND hwnd /*= NULL*/ )
{
    LoadDesktopBitmaps(hwnd, width, height);
}

void vtDesktop::FreeDesktopBitmaps( void )
{
    if (m_pbmpWallPaper != NULL) {
        m_pbmpWallPaper->DeleteObject();
        delete m_pbmpWallPaper;
        m_pbmpWallPaper = NULL;
    }
    if (m_pbmpBG != NULL) {
        m_pbmpBG->DeleteObject();
        delete m_pbmpBG;
        m_pbmpBG = NULL;
    }
    if (m_pbmpBGBuf != NULL) {
        m_pbmpBGBuf->DeleteObject();
        delete m_pbmpBGBuf;
        m_pbmpBGBuf = NULL;
    }
}

void vtDesktop::PaintDesktop( CDC *pDC, CRect *lpRect, UINT flags )
{
    ASSERT(pDC != NULL);
    if (pDC != NULL)
    {
        CBitmap *pOldBmp = NULL;
        CDC dcMem;
        dcMem.CreateCompatibleDC(pDC);
        pOldBmp = dcMem.SelectObject((CBitmap *)m_pbmpBGBuf);

#if 1
        PaintDesktopBackground(&dcMem, lpRect, flags);
        PaintDesktopIconList(&dcMem, lpRect, flags);

        pDC->BitBlt(lpRect->left, lpRect->top, lpRect->Width(), lpRect->Height(),
            &dcMem, lpRect->left, lpRect->top, SRCCOPY);
#else
        PaintDesktopBackground(pDC, lpRect, flags);
        PaintDesktopIconList(pDC, lpRect, flags);
#endif

        if (pOldBmp != NULL) {
            dcMem.SelectObject(pOldBmp);
            pOldBmp = NULL;
        }
        dcMem.DeleteDC();
    }
}

void vtDesktop::PaintDesktopBackground( CDC *pDC, CRect *lpRect, UINT flags )
{
    ASSERT(pDC != NULL);
    if (pDC != NULL)
    {
        if (m_pbmpBG != NULL) {
            CBitmap *pOldBmp = NULL;
            CDC dcMem;
            dcMem.CreateCompatibleDC(pDC);

            pOldBmp = dcMem.SelectObject((CBitmap *)m_pbmpBG);
            pDC->BitBlt(lpRect->left, lpRect->top, lpRect->Width(), lpRect->Height(),
                &dcMem, lpRect->left, lpRect->top, SRCCOPY);
            if (pOldBmp != NULL) {
                dcMem.SelectObject(pOldBmp);
                pOldBmp = NULL;
            }
            dcMem.DeleteDC();
        }
    }
}

void vtDesktop::PaintDesktopIconList( CDC *pDC, CRect *lpRect, UINT flags )
{
    ASSERT(pDC != NULL);
    if (pDC != NULL)
    {
        list<vtShortCut *>::iterator itList;
        vtShortCut *shortcut;
        CRect rcIcon;
        int index = 0;
        for (itList = shortCutList.begin(); itList != shortCutList.end(); itList++) {
            shortcut = (vtShortCut *)*itList;
            ASSERT(shortcut != NULL);
            if (shortcut != NULL) {
                PaintDesktopIcon(pDC, &rcIcon, shortcut, index, flags);
                index++;
            }
        }
    }
}

void vtDesktop::PaintDesktopIcon( CDC *pDC, CRect *lpRect, vtShortCut *shortcut, int index, UINT flags )
{
    ASSERT(pDC != NULL);
    if (pDC != NULL) {
        //
    }
}
