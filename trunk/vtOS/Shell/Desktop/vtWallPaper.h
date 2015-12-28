#pragma once

#include "..\..\Kernel\vtObject.h"

typedef enum _VT_ALIGN_MODE {
	VT_ALIGN_CENTER,
	VT_ALIGN_TILE,
	VT_ALIGN_STRETCH,
	VT_ALIGN_MAX
} VT_ALIGN_MODE;

typedef struct _VT_WALLPAPER_INFO {
	LONG width;
	LONG height;

	LONG align;
	COLORREF bgcolor;
	TCHAR cPictureFile[MAX_PATH];
} VT_WALLPAPER_INFO,  *PVT_WALLPAPER_INFO,  *LPVT_WALLPAPER_INFO;

class vtWallPaper : public vtObject, public VT_WALLPAPER_INFO
{
public:
	vtWallPaper(void);
	virtual ~vtWallPaper(void);
};
