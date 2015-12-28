
#include "..\..\..\FacePad\stdafx.h"
#include "vtWindows.h"

vtWindows::vtWindows(void)
{
    initWindows(NULL);
}

vtWindows::vtWindows( HWND hwnd )
{
    initWindows(hwnd);
}

vtWindows::vtWindows( CWnd *pWnd )
{
    if (pWnd != NULL)
        initWindows(pWnd->GetSafeHwnd());
}

vtWindows::~vtWindows(void)
{
}

void vtWindows::initWindows( HWND hwnd )
{
    m_hWnd = hwnd;
    m_pvtDesktop = NULL;
}

BOOL vtWindows::Attach( HWND hwnd )
{
    if (hwnd == NULL)
        return FALSE;

    Detach();
    m_hWnd = hwnd;

    m_pvtDesktop = new vtDesktop;
    if (m_pvtDesktop != NULL) {
        m_pvtDesktop->Attach(hwnd);
        m_pvtDesktop->initShortCutList();
    }
    return ::IsWindow(hwnd);
}

BOOL vtWindows::Attach( CWnd *pWnd )
{
    if (pWnd != NULL) {
        return Attach(pWnd->GetSafeHwnd());
    }
    else
        return FALSE;
}

HWND vtWindows::Detach( void )
{
    HWND hOldWnd = m_hWnd;
    if (m_pvtDesktop != NULL) {
        m_pvtDesktop->DeleteObject();
        m_pvtDesktop->Detach();
        delete m_pvtDesktop;
        m_pvtDesktop = NULL;
    }
    DeleteObject();
    m_hWnd = NULL;
    return hOldWnd;
}

void vtWindows::DeleteObject( void )
{
    if (m_pvtDesktop != NULL) {
        m_pvtDesktop->Detach();
        delete m_pvtDesktop;
        m_pvtDesktop = NULL;
    }
}
