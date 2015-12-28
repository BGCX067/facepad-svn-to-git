// MemDC.h : header file
//

#ifndef _MEMDC_H_
#define _MEMDC_H_

#pragma once

#include "..\..\FacePad\stdafx.h"

//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-1997, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//                   Added print support.
//           25 feb 98 - fixed minor assertion bug
//
//           03/12/2009 modified by Guozi
//
// This class implements a memory Device Context

class CMemDC : public CDC
{
public:
	CMemDC();
	CMemDC(CDC* pDC);
	CMemDC(CDC* pDC, const CRect& rcBounds);
	CMemDC(CDC* pDC, const RECT* rcBounds);
	virtual ~CMemDC();

	void InitData(void);
	void FreeObject(void);

	BOOL CreateFromDC(CDC* pDC);
	BOOL CreateFromDC(CDC* pDC, CRect* rc);
	BOOL CreateFromDC(CDC* pDC, int w, int h, int x = 0, int y = 0);

	void SetCopyBackStatus(BOOL bAlreadyCopyBack)
	{
		m_bAlreadyCopyBack = bAlreadyCopyBack;
	};

	BOOL CopyFrom(CDC *pDC = NULL);
	BOOL CopyBack(CDC *pDC = NULL, BOOL bSwapBack = TRUE);
	BOOL BitBltDC(CDC *pDC, int x, int y, int w, int h,
		int srcX = 0, int srcY = 0, BOOL bSwapBack = TRUE);

	// Allow usage as a pointer
	CMemDC* operator->() { return this; }

	// Allow usage as a pointer
	operator CMemDC*() { return this; }

public:
	CBitmap  m_bitmap;      // Offscreen bitmap

private:
	CDC*     m_pDC;         // Saves CDC passed in constructor
	CBitmap* m_pOldBitmap;  // bitmap originally found in CMemDC
	CRect    m_rect;        // Rectangle of drawing area.
	BOOL     m_bMemDC;      // TRUE if CDC really is a Memory DC.
	BOOL     m_bAlreadyCopyBack;// TRUE if CMemDC is have copy back
	BOOL     m_bSelectOldBmp;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif  //!_MEMDC_H_
