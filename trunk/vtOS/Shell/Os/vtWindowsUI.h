#pragma once

#include "..\..\..\FacePad\stdafx.h"
#include "vtWindows.h"
#include "..\..\UI\vtWndBase.h"

// vtWindowsUI

class vtWindowsUI : public vtWndBase
{
	DECLARE_DYNAMIC(vtWindowsUI)

public:
	vtWindowsUI();
	virtual ~vtWindowsUI();

public:
	//

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

public:
	vtWindows *m_pvtWindows;
    vtDesktop *m_pvtDesktop;
};
