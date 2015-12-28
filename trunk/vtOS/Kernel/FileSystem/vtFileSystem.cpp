
#include "..\..\..\FacePad\stdafx.h"
#include "vtFileSystem.h"
#include <windows.h>
#include <afx.h>
#include <crtdbg.h>
#include <string.h>
#include <stdlib.h>

__inline void copyFileTime(VT_FILE_TIME *dest, FILETIME *src)
{
	ASSERT(dest != NULL && src != NULL);

	dest->dwHighDateTime = src->dwHighDateTime;
	dest->dwLowDateTime  = src->dwLowDateTime;
}

vtFileInfo::vtFileInfo(void)
{
	FILETIME fileTime;
	GetSystemTimeAsFileTime(&fileTime);

	initFileInfo(&fileTime);
}

vtFileInfo::vtFileInfo( const vtFileInfo& objectSrc )
{
    CopyFileInfo(objectSrc);
}

void vtFileInfo::operator=( const vtFileInfo& objectSrc )
{
    CopyFileInfo(objectSrc);
}

vtFileInfo::~vtFileInfo(void)
{
}

void vtFileInfo::initFileInfo( FILETIME *fileTime )
{
    ASSERT(fileTime != NULL);
    dwFileAttributes = 0;
    GetSystemTimeAsFileTime(fileTime);
    copyFileTime(&ftCreationTime, fileTime);
    copyFileTime(&ftLastAccessTime, fileTime);
    copyFileTime(&ftLastWriteTime, fileTime);
    dwFileSizeHigh = 0;
    dwFileSizeLow  = 0;
    dwReserved0    = 0;
    dwReserved1    = 0;

    _tcscpy_s(cFileName, _countof(cFileName), _T(""));
    _tcscpy_s(cAlternateFileName, _countof(cAlternateFileName), _T(""));

    dwFileAllocSizeHigh = 0;
    dwFileAllocSizeLow  = 0;
}

void vtFileInfo::CopyFileInfo( const vtFileInfo& objectSrc )
{
    dwFileAttributes = objectSrc.dwFileAttributes;
    ftCreationTime   = objectSrc.ftCreationTime;
    ftLastAccessTime = objectSrc.ftLastAccessTime;
    ftLastWriteTime  = objectSrc.ftLastWriteTime;
    dwFileSizeHigh = objectSrc.dwFileSizeHigh;
    dwFileSizeLow  = objectSrc.dwFileSizeLow;
    dwReserved0    = objectSrc.dwReserved0;
    dwReserved1    = objectSrc.dwReserved1;

    _tcscpy_s(cFileName, _countof(cFileName), objectSrc.cFileName);
    _tcscpy_s(cAlternateFileName, _countof(cAlternateFileName), objectSrc.cAlternateFileName);

    dwFileAllocSizeHigh = objectSrc.dwFileAllocSizeHigh;
    dwFileAllocSizeLow  = objectSrc.dwFileAllocSizeLow;
}
