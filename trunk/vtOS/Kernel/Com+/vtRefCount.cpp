//#include "StdAfx.h"
#include "vtRefCount.h"

vtRefCount::vtRefCount( void )
{
	initRefCount(0);
    _AddRef();
}

vtRefCount::vtRefCount( long nRecCount )
{
    initRefCount(nRecCount);
}

vtRefCount::vtRefCount( const vtRefCount& objectSrc )
{
	initRefCount(objectSrc.m_nRecCount);
}

void vtRefCount::operator=( const vtRefCount& objectSrc )
{
	initRefCount(objectSrc.m_nRecCount);
}

vtRefCount::~vtRefCount( void )
{
	_Release();
}

/*
inline void vtRefCount::initRefCount( long nRecCount )
{
    m_nRecCount = nRecCount;
}

inline long vtRefCount::_AddRef( void )
{
    m_nRecCount++;
    return m_nRecCount;
}

inline long vtRefCount::_Release( void )
{
    m_nRecCount--;
    return m_nRecCount;
}

long vtRefCount::GetRefCount( void )
{
    return m_nRecCount;
}

long vtRefCount::SetRefCount( long nRecCount )
{
    m_nRecCount = nRecCount;
    return m_nRecCount;
}
//*/
