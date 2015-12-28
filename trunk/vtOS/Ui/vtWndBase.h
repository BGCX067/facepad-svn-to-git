#pragma once

#include "..\..\FacePad\stdafx.h"

#define VTWNDBASE_CLASSNAME    _T("vtWndBase")

// vtWndBase

class vtWndBase : public CWnd
{
	DECLARE_DYNAMIC(vtWndBase)

public:
	vtWndBase();
	virtual ~vtWndBase();

public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL Create(CString strTitle, UINT dwStyle, const CRect& rect,
		CWnd* pParentWnd, UINT nID);

    LPCTSTR GetWndClassName(void);
    void    SetWndClassName(LPCTSTR lpszClassName);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

protected:
	BOOL RegisterWindowClass(LPCTSTR lpszClassName);

protected:
	CString m_strClassName;

	CBrush m_bkBrush;
	COLORREF m_clrBkgnd;
};
