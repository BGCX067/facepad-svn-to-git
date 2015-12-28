#pragma once

#include "..\..\..\FacePad\stdafx.h"
#include <WinDef.h>

typedef struct _VT_FILE_TIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} VT_FILE_TIME, *PVT_FILE_TIME, *LPVT_FILE_TIME;
#define _VTFILETIME_

typedef struct _VT_WIN32_FILE_INFO {
	DWORD      dwFileAttributes;
	VT_FILE_TIME ftCreationTime;
	VT_FILE_TIME ftLastAccessTime;
	VT_FILE_TIME ftLastWriteTime;
	DWORD      dwFileSizeHigh;
	DWORD      dwFileSizeLow;
	DWORD      dwReserved0;
	DWORD      dwReserved1;
	TCHAR      cFileName[MAX_PATH];
	TCHAR      cAlternateFileName[14];
	DWORD      dwFileAllocSizeHigh;
	DWORD      dwFileAllocSizeLow;
} VT_WIN32_FILE_INFO,  *PVT_WIN32_FILE_INFO,  *LPVT_WIN32_FILE_INFO;
#define _VT_WIN32_FILE_INFO_

//#define AFX_NOVTABLE  __declspec(novtable)
class __declspec(novtable) vtFileInfo : public VT_WIN32_FILE_INFO
{
public:
	vtFileInfo(void);
	virtual ~vtFileInfo(void);

    vtFileInfo(const vtFileInfo& objectSrc);		// no implementation
    void operator=(const vtFileInfo& objectSrc);	// no implementation

protected:
	void initFileInfo(FILETIME *fileTime);
    void CopyFileInfo(const vtFileInfo& objectSrc);
};
