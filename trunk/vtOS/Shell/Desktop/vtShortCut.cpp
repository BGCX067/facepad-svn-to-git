//#include "StdAfx.h"
#include "vtShortCut.h"

///////////////////////////////////////////////
// vtShortCut
///////////////////////////////////////////////

vtShortCut::vtShortCut( void )
{
	_tcscpy_s(cLinkName, _countof(cLinkName), _T(""));				// 快捷方式的名称
	_tcscpy_s(cTargetFileName, _countof(cTargetFileName), _T(""));	// 目标文件名
	_tcscpy_s(cTargetPathName, _countof(cTargetPathName), _T(""));	// 目标位置
	_tcscpy_s(cFileType, _countof(cFileType), _T(""));				// 快捷方式的文件类型
	_tcscpy_s(cDefaultIcon, _countof(cDefaultIcon), _T(""));		// 快捷方式的文件类型

	wShortCutMod = 0;
	wShortCutKey = 0;
	wShowCmd     = VT_SHOW_CMD_NORMAL;
	wReserved0   = 0;
	dwReserved3  = 0;
	dwReserved4  = 0;

	_tcscpy_s(cFileNameOfLinkIcon, _countof(cFileNameOfLinkIcon), _T(""));
	nIndexOfLinkIcon = -1;
	nIndexOfIconResList = -1;
}

vtShortCut::vtShortCut( const vtShortCut& objectSrc )
{
    CopyShortCut(objectSrc);
}

void vtShortCut::operator=( const vtShortCut& objectSrc )
{
    CopyShortCut(objectSrc);
}

vtShortCut::~vtShortCut( void )
{
}

TCHAR * vtShortCut::SetLinkName( TCHAR *_cLinkName )
{
	_tcscpy_s(cLinkName, _countof(cLinkName), _cLinkName);
	return _cLinkName;
}

void vtShortCut::CopyShortCut( const vtShortCut& objectSrc )
{
    CopyObject(objectSrc);
    CopyFileInfo(objectSrc);

    _tcscpy_s(cLinkName, _countof(cLinkName), objectSrc.cLinkName);
    _tcscpy_s(cTargetFileName, _countof(cTargetFileName), objectSrc.cTargetFileName);
    _tcscpy_s(cTargetPathName, _countof(cTargetPathName), objectSrc.cTargetPathName);
    _tcscpy_s(cFileType, _countof(cFileType), objectSrc.cFileType);
    _tcscpy_s(cDefaultIcon, _countof(cDefaultIcon), objectSrc.cDefaultIcon);

    wShortCutMod = objectSrc.wShortCutMod;
    wShortCutKey = objectSrc.wShortCutKey;
    wShowCmd     = objectSrc.wShowCmd;
    wReserved0   = objectSrc.wReserved0;
    dwReserved3  = objectSrc.dwReserved3;
    dwReserved4  = objectSrc.dwReserved4;

    _tcscpy_s(cFileNameOfLinkIcon, _countof(cFileNameOfLinkIcon), objectSrc.cFileNameOfLinkIcon);
    nIndexOfLinkIcon = objectSrc.nIndexOfLinkIcon;
    nIndexOfIconResList = objectSrc.nIndexOfIconResList;
}

///////////////////////////////////////////////
// vtShortCutList
///////////////////////////////////////////////

vtShortCutList::vtShortCutList( void )
{
	//
}

vtShortCutList::~vtShortCutList( void )
{
	Clear();
    //this->clear();
}

void vtShortCutList::Clear( void )
{
	list<vtShortCut *>::iterator itList;
	for (itList = begin(); itList != end(); ) {
		vtShortCut *shortCut = (vtShortCut *)(*itList);
		if (shortCut != NULL) {
			//itList = this->remove(shortCut);
			itList = this->erase(itList);
            delete shortCut;
			shortCut = NULL;
		}
        else
            itList++;
	}
}

size_t vtShortCutList::SizeOf( void )
{
	return (size_t)(this->size());
}

int vtShortCutList::IndexOf( vtShortCut *_shortCut )
{
	int _index = -1;
	ASSERT(_shortCut != NULL);
	if (_shortCut != NULL) {
		list<vtShortCut *>::iterator itList;
		int _counter = 0;
		for (itList = begin(); itList != end(); itList++) {
			if (_shortCut == (vtShortCut *)(*itList))
				return _counter;
			_counter++;
		}
	}
	return _index;
}

int vtShortCutList::IndexOf( list<vtShortCut *>::iterator _itList )
{
	int _index = -1;
	ASSERT(_itList != this->end());
	if (_itList != this->end()) {
		list<vtShortCut *>::iterator itList;
		int _counter = 0;
		for (itList = begin(); itList != end(); itList++) {
			if (itList == _itList)
				return _counter;
			_counter++;
		}
	}
	return _index;
}

list<vtShortCut *>::iterator vtShortCutList::IterOf( vtShortCut *_shortCut )
{
	list<vtShortCut *>::iterator _itList = end();
	ASSERT(_shortCut != NULL);
	if (_shortCut != NULL) {
		list<vtShortCut *>::iterator itList;
		int _counter = 0;
		for (itList = begin(); itList != end(); itList++) {
			if (_shortCut == (vtShortCut *)(*itList))
				return itList;
		}
	}
	return _itList;
}

vtShortCut * vtShortCutList::GetItem( int _index )
{
	ASSERT(_index < 0 || _index >= (int)size());
	if (_index < 0 || _index >= (int)size())
		return NULL;

	list<vtShortCut *>::iterator itList;
	int _counter = 0;
	for (itList = begin(); itList != end(); itList++) {
		if (_counter == _index)
			return (vtShortCut *)(*itList);
		_counter++;
	}
	return NULL;
}

list<vtShortCut *>::iterator vtShortCutList::GetItemIter( int _index )
{
	list<vtShortCut *>::iterator itList = end();

	ASSERT(_index < 0 || _index >= (int)size());
	if (_index < 0 || _index >= (int)size())
		return itList;

	int _counter = 0;
	for (itList = begin(); itList != end(); itList++) {
		if (_counter == _index)
			return itList;
		_counter++;
	}
	return itList;
}

vtShortCut *vtShortCutList::operator[]( int _index )
{
	return GetItem(_index);
}

int vtShortCutList::Add( vtShortCut *shortCut )
{
	int _index = -1;
	ASSERT(shortCut != NULL);
	if (shortCut != NULL) {
        vtShortCut *newShortCut = new vtShortCut(*shortCut);
		this->push_back(newShortCut);
	}
	return _index;
}

int vtShortCutList::Remove( int _index )
{
#if 1
	int _retIndex = -1;
	ASSERT(_index >= 0 && _index < (int)size());
	if (_index >= 0 && _index < (int)size()) {
		list<vtShortCut *>::iterator itList = GetItemIter(_index);
		if (itList != end()) {
			vtShortCut *shortCut = (vtShortCut *)(*itList);
			if (shortCut != NULL) {
				itList = this->erase(itList);
				delete shortCut;
				shortCut = NULL;
				_retIndex = _index;
			}
		}
	}
	return _retIndex;
#else
	int _retIndex = -1;
	ASSERT(_index >= 0 && _index < (int)size());
	if (_index >= 0 && _index < (int)size()) {
		vtShortCut *shortCut = GetItem(_index);
		if (shortCut != NULL) {
			this->remove(shortCut);
            delete shortCut;
			shortCut = NULL;
			_retIndex = _index;
		}
	}
	return _retIndex;
#endif
}

int vtShortCutList::Remove( vtShortCut *_shortCut )
{
	int _index = -1;
	if (_shortCut != NULL) {
		_index = IndexOf(_shortCut);
		if (_index >= 0 && _index < (int)size()) {
			this->remove(_shortCut);
            delete _shortCut;
			_shortCut = NULL;
		}
	}
	return _index;
}
