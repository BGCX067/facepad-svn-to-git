//#include "StdAfx.h"
#include "vtObject.h"

vtObject::vtObject( void )
{
	initObject(NULL);
}

vtObject::vtObject( const vtObject& objectSrc )
{
	initObject(objectSrc.m_nObjectId);
}

void vtObject::operator=( const vtObject& objectSrc )
{
	initObject(objectSrc.m_nObjectId);
}

vtObject::~vtObject( void )
{
	//
}

void vtObject::initObject( LONG nObjectId )
{
    m_nObjectId = nObjectId;
}

void vtObject::CopyObject( const vtObject& objectSrc )
{
    m_nObjectId = objectSrc.m_nObjectId;
}
