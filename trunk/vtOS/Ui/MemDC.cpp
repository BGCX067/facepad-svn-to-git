// MemDC.cpp : source file
//

//#include "stdafx.h"
#include "MemDC.h"

// constructor sets up the memory DC
CMemDC::CMemDC() : CDC()
{
	InitData();
}

// constructor sets up the memory DC
CMemDC::CMemDC(CDC* pDC) : CDC()
{
	InitData();
	CreateFromDC(pDC);
}

CMemDC::CMemDC(CDC* pDC, const CRect& rcBounds) : CDC()
{
	InitData();
	CreateFromDC(pDC, (CRect *)&rcBounds);
}

CMemDC::CMemDC(CDC* pDC, const RECT* rcBounds)
{
	InitData();
	if (rcBounds != NULL)
		CreateFromDC(pDC, (CRect *)&CRect(rcBounds));
}

// Destructor copies the contents of the mem DC to the original DC
CMemDC::~CMemDC()
{
	if (!m_bAlreadyCopyBack)
		CopyBack();

	if (!m_bSelectOldBmp && m_pOldBitmap != NULL)
	{
		SelectObject(m_pOldBitmap);
		m_pOldBitmap = NULL;
	}
}

void CMemDC::InitData(void)
{
	m_pDC = NULL;
	m_pOldBitmap = NULL;
	m_bMemDC = TRUE;
	m_bSelectOldBmp = TRUE;
	m_bAlreadyCopyBack = FALSE;

	m_rect.SetRect(0, 0, 0, 0);
}

void CMemDC::FreeObject(void)
{
	m_bMemDC = TRUE;
	if (m_pDC != NULL)
		m_bMemDC = !m_pDC->IsPrinting();
	if (m_bMemDC)
	{
		if (m_hDC != NULL)
		{
			if (!m_bSelectOldBmp)
			{
				SelectObject(m_pOldBitmap);
				m_pOldBitmap = NULL;
				m_bSelectOldBmp = TRUE;
			}
			DeleteDC();
		}
	}
	m_hDC = m_hAttribDC = NULL;
}

BOOL CMemDC::CreateFromDC(CDC* pDC)
{
	ASSERT(pDC != NULL);
	ASSERT(pDC->m_hDC != NULL);
	if (pDC == NULL || pDC->m_hDC == NULL)
		return FALSE;

	CRect rect;
	rect.SetRect(0, 0, 0, 0);
	pDC->GetClipBox(&rect);

	//TRACE(_T("rect(%d, %d, %d, %d)\n"), rect.left, rect.top, rect.right, rect.bottom);
	return CreateFromDC(pDC, rect.Width(), rect.Height(),
		rect.left, rect.top);
}

BOOL CMemDC::CreateFromDC(CDC* pDC, CRect* rc)
{
	if (rc == NULL)
		return FALSE;

	return CreateFromDC(pDC, rc->Width(), rc->Height(),
		rc->left, rc->top);
}

BOOL CMemDC::CreateFromDC(CDC* pDC, int w, int h, int x, int y)
{
	ASSERT(pDC != NULL);
	ASSERT(pDC->m_hDC != NULL);
	if (pDC == NULL || pDC->m_hDC == NULL)
		return FALSE;

	if (!(w | h))
		return FALSE;

	FreeObject();

	m_pDC = pDC;
	m_pOldBitmap = NULL;
	m_bMemDC = !pDC->IsPrinting();

	if (m_bMemDC)    // Create a Memory DC
	{
		m_rect.SetRect(x, y, x + w, y + h);
		//CRect rect;
		//rect.SetRect(0, 0, 0, 0);
		//pDC->GetClipBox(&rect);
		//TRACE(_T("rect(%d, %d, %d, %d)\n"), rect.left, rect.top, rect.right, rect.bottom);
		CreateCompatibleDC(pDC);
		m_bitmap.CreateCompatibleBitmap(pDC, w, h);
		m_pOldBitmap = SelectObject(&m_bitmap);
		m_bSelectOldBmp = FALSE;
		SetWindowOrg(x, y);
		//TRACE(_T("x, y: (%d, %d)\n"), x, y);

		//this->BitBlt(x, y, w, h, pDC, x, y, SRCCOPY);
	}
	else        // Make a copy of the relevent parts of the current DC for printing
	{
		m_bPrinting = pDC->m_bPrinting;
		m_hDC       = pDC->m_hDC;
		m_hAttribDC = pDC->m_hAttribDC;
	}

	return TRUE;
}

BOOL CMemDC::CopyFrom(CDC *pDC /* = NULL */)
{
	if (pDC == NULL)
		pDC = m_pDC;

	m_bMemDC = !pDC->IsPrinting();
	if (m_bMemDC)
	{
		//if (this->GetSafeHdc())
		if (pDC && pDC->m_hDC && this->GetSafeHdc())
		{
			// Copy the offscreen bitmap onto the screen.
			CRect rect;
			rect.SetRect(0, 0, 0, 0);
			pDC->GetClipBox(&rect);

			this->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				pDC, m_rect.left, m_rect.top, SRCCOPY);

			//this->BitBlt(x, y, w, h, pDC, x, y, SRCCOPY);
		}
	}
	return TRUE;
}

BOOL CMemDC::CopyBack(CDC *pDC /* = NULL */, BOOL bSwapBack /* = TRUE */)
{
	return BitBltDC(pDC, m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
		m_rect.left, m_rect.top, bSwapBack);
}

BOOL CMemDC::BitBltDC(CDC *pDC, int x, int y, int w, int h,
					int srcX, int srcY, BOOL bSwapBack)
{
	if (pDC == NULL)
		pDC = m_pDC;

	//ASSERT(pDC != NULL);
	if (pDC == NULL)
		return FALSE;
	//ASSERT(pDC->m_hDC != NULL);
	if (pDC->m_hDC == NULL)
		return FALSE;

	if (m_bMemDC)
	{
		// Copy the offscreen bitmap onto the screen.
		pDC->BitBlt(x, y, w, h, this, srcX, srcY, SRCCOPY);

		//Swap back the original bitmap.
		if (bSwapBack)
		{
			SelectObject(m_pOldBitmap);
			m_pOldBitmap = NULL;
			m_bSelectOldBmp = TRUE;
		}
	} else {
		// All we need to do is replace the DC with an illegal value,
		// this keeps us from accidently deleting the handles associated with
		// the CDC that was passed to the constructor.
		m_hDC = m_hAttribDC = NULL;
	}

	if (!m_bMemDC || (m_bMemDC && m_bSelectOldBmp))
		m_bAlreadyCopyBack = TRUE;

	return TRUE;
}
