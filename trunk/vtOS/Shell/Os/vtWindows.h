#pragma once

#include "..\..\..\FacePad\stdafx.h"
#include "vtOs.h"
#include "..\Desktop\vtDesktop.h"

class vtWindows : public vtOs
{
public:
	vtWindows(void);
	virtual ~vtWindows(void);

    vtWindows(HWND hwnd);
    vtWindows(CWnd *pWnd);

public:
    BOOL Attach(HWND hwnd);
    BOOL Attach(CWnd *pWnd);
    HWND Detach(void);

    void DeleteObject(void);

protected:
    void initWindows(HWND hwnd);

public:
	vtDesktop *m_pvtDesktop;

private:
    HWND m_hWnd;
};
