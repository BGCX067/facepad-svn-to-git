#pragma once

#include "..\..\FacePad\targetver.h"
#include <afx.h>

class vtObject : public CObject
{
public:
	LONG m_nObjectId;

public:
	vtObject(void);
	virtual ~vtObject(void);

protected:
	vtObject(const vtObject& objectSrc);		// no implementation
	void operator=(const vtObject& objectSrc);	// no implementation

    void CopyObject(const vtObject& objectSrc);

private:
	void initObject(LONG nObjectId);
};
