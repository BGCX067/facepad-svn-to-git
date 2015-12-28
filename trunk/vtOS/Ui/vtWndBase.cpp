// vtWndBase.cpp : 实现文件
//

#include "..\..\FacePad\stdafx.h"
#include "..\..\FacePad\FacePad.h"
#include "vtWndBase.h"
#include "MemDC.h"


// vtWndBase

IMPLEMENT_DYNAMIC(vtWndBase, CWnd)

vtWndBase::vtWndBase() : m_strClassName(VTWNDBASE_CLASSNAME),
						m_clrBkgnd(GetSysColor(COLOR_WINDOW))
{
	//m_strClassName = VTWNDBASE_CLASSNAME;

	//m_clrBkgnd = GetSysColor(COLOR_WINDOW);
	m_bkBrush.CreateSolidBrush(m_clrBkgnd);
}

vtWndBase::~vtWndBase()
{
	m_bkBrush.DeleteObject();
}


BEGIN_MESSAGE_MAP(vtWndBase, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// vtWndBase 消息处理程序

// Register the window class if it has not already been registered.

BOOL vtWndBase::RegisterWindowClass(LPCTSTR lpszClassName)
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, lpszClassName, &wndcls)))
	{
		wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc      = ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
		wndcls.hInstance        = hInst;
		wndcls.hIcon            = NULL;
		wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground    = NULL;
		//wndcls.hbrBackground    = (HBRUSH) m_bkBrush.m_hObject;
		//wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
		wndcls.lpszMenuName     = NULL;
		wndcls.lpszClassName    = lpszClassName;

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}

LPCTSTR vtWndBase::GetWndClassName( void )
{
    return (LPCTSTR)m_strClassName;
}

void vtWndBase::SetWndClassName( LPCTSTR lpszClassName )
{
    m_strClassName = lpszClassName;
}

BOOL vtWndBase::Create(CString strTitle, UINT dwStyle, const CRect & rect,
						 CWnd *pParentWnd, UINT nID)
{
	BOOL bResult = RegisterWindowClass(m_strClassName);
	if (!bResult) return FALSE;
	return CWnd::Create(m_strClassName, strTitle, dwStyle, rect, pParentWnd, nID);
}

BOOL vtWndBase::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						 const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	BOOL bResult = RegisterWindowClass(lpszClassName);
	if (!bResult) return FALSE;
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

int vtWndBase::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return 0;
}

BOOL vtWndBase::OnEraseBkgnd( CDC* pDC )
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

void vtWndBase::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rcClient;
	GetClientRect(&rcClient);

	CMemDC dcMem(&dc);

	CDC *pDC = (CDC *)&dcMem;
	if (pDC && pDC->m_hDC) {
		dcMem.FillSolidRect(&rcClient, m_clrBkgnd);
	}
}
