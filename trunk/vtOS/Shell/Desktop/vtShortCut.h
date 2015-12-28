#pragma once

#include "..\..\Kernel\vtObject.h"
#include "..\..\Kernel\FileSystem\vtFileSystem.h"
#include "..\..\Kernel\Com+\vtRefCount.h"
#include <list>
#include <vector>
#include "WinDef.h"

#define MAX_SHORTCUT_NAME     MAX_PATH
#define MAX_SHORTCUT_STRING   MAX_PATH

using namespace std;

typedef enum _VT_SHOW_CMD {
	VT_SHOW_CMD_NORMAL,
	VT_SHOW_CMD_MAXIMIZE,
	VT_SHOW_CMD_MINIMIZE,
	VT_SHOW_CMD_MAX
} VT_SHOW_CMD;

typedef struct _VT_SHORTCUT_INFO {
	TCHAR   cLinkName[MAX_SHORTCUT_NAME];			// 快捷方式的名称
	TCHAR   cTargetFileName[MAX_SHORTCUT_STRING];	// 目标文件名
	TCHAR   cTargetPathName[MAX_SHORTCUT_STRING];	// 目标位置
	TCHAR   cFileType[MAX_SHORTCUT_STRING];			// 快捷方式的文件类型
	TCHAR   cDefaultIcon[MAX_SHORTCUT_STRING];		// 快捷方式关联的默认图标
	WORD    wShortCutMod;
	WORD    wShortCutKey;
	WORD    wShowCmd;
	WORD    wReserved0;
	DWORD   dwReserved3;
	DWORD   dwReserved4;
	TCHAR   cFileNameOfLinkIcon[MAX_SHORTCUT_STRING];
	LONG    nIndexOfLinkIcon;
	LONG    nIndexOfIconResList;
} VT_SHORTCUT_INFO,  *PVT_SHORTCUT_INFO,  *LPVT_SHORTCUT_INFO;
#define _VT_SHORTCUT_INFO_

class vtShortCut : public vtObject, public vtFileInfo, public VT_SHORTCUT_INFO
{
public:
	vtShortCut(void);
	virtual ~vtShortCut(void);

    vtShortCut(const vtShortCut& objectSrc);		// no implementation
    void operator=(const vtShortCut& objectSrc);	// no implementation

public:
	TCHAR * SetLinkName(TCHAR *_cLinkName);

protected:
    void CopyShortCut(const vtShortCut& objectSrc);

private:
	//
};

class vtShortCutList : public list<vtShortCut *>
{
public:
	vtShortCutList(void);
	virtual ~vtShortCutList(void);

public:
	int Add(vtShortCut *_shortCut);
	int Remove(int _index);
	int Remove(vtShortCut *_shortCut);
	void Clear(void);

	size_t SizeOf( void );
	int IndexOf(vtShortCut *_shortCut);
	int IndexOf(list<vtShortCut *>::iterator _itList);
	list<vtShortCut *>::iterator IterOf(vtShortCut *_shortCut);

	vtShortCut *GetItem(int _index);
	list<vtShortCut *>::iterator GetItemIter(int _index);
	vtShortCut *operator[] ( int _index );

protected:
	//

public:
	//
};
