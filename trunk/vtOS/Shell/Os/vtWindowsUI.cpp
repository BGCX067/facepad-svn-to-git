// vtWindowsUI.cpp : 实现文件
//

#include "..\..\..\FacePad\stdafx.h"
#include "..\..\..\FacePad\FacePad.h"
#include "vtWindowsUI.h"
#include "..\..\UI\MemDC.h"

// vtWindowsUI

IMPLEMENT_DYNAMIC(vtWindowsUI, CWnd)

vtWindowsUI::vtWindowsUI()
{
	m_clrBkgnd = WIN_2003_DESKTOP_DEFAULT_BGCOLOR;
	if (m_bkBrush.m_hObject != NULL)
		m_bkBrush.DeleteObject();
	m_bkBrush.CreateSolidBrush(m_clrBkgnd);

    m_pvtWindows = NULL;
    m_pvtDesktop = NULL;
}

vtWindowsUI::~vtWindowsUI()
{
    if (m_pvtWindows != NULL) {
        m_pvtWindows->DeleteObject();
        delete m_pvtWindows;
        m_pvtWindows = NULL;
        m_pvtDesktop = NULL;
    }
}


BEGIN_MESSAGE_MAP(vtWindowsUI, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
    ON_WM_TIMER()
END_MESSAGE_MAP()



// vtWindowsUI 消息处理程序

int vtWindowsUI::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

    if (m_pvtWindows != NULL) {
        m_pvtWindows->Detach();
        delete m_pvtWindows;
        m_pvtDesktop = NULL;
    }

    m_pvtWindows = new vtWindows;
    if (m_pvtWindows != NULL) {
        m_pvtWindows->Attach(this->GetSafeHwnd());
        m_pvtDesktop = m_pvtWindows->m_pvtDesktop;
    }

	return 0;
}

BOOL vtWindowsUI::OnEraseBkgnd( CDC* pDC )
{
#if 0
	CRect rc;
	GetClientRect(&rc);

	pDC->FillSolidRect(&rc, m_clrBkgnd);
	//return CWnd::OnEraseBkgnd(pDC);
    return TRUE;
#else
    return FALSE;
#endif
}

void vtWindowsUI::OnPaint()
{
#if 1
    PAINTSTRUCT ps;
    CDC *pDC = BeginPaint(&ps);
    ASSERT(pDC != NULL);
    if (pDC != NULL)
    {
        CRect rcPaint;
        rcPaint = ps.rcPaint;

#if 0
        CMemDC dcMem(pDC);

        CDC *pMemDC = (CDC *)&dcMem;
        if (pMemDC && pMemDC->m_hDC) {
            dcMem.FillSolidRect(&rcPaint, m_clrBkgnd);
        }
        dcMem->DeleteDC();
#endif

        if (m_pvtDesktop != NULL)
            m_pvtDesktop->PaintDesktop(pDC, &rcPaint, 0);
    }

    EndPaint(&ps);
#else
	CPaintDC dc(this); // device context for painting

	CRect rcClient;
	GetClientRect(&rcClient);

	CMemDC dcMem(&dc);

	CDC *pDC = (CDC *)&dcMem;
	if (pDC && pDC->m_hDC) {
		dcMem.FillSolidRect(&rcClient, m_clrBkgnd);
	}
    dcMem->DeleteDC();
#endif
}

void vtWindowsUI::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    vtWndBase::OnTimer(nIDEvent);
}
