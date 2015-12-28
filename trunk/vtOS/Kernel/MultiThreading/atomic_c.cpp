
#include "..\vtCommon.h"
#include "atomic_c.h"

atomic_c::atomic_c( void )
{
	Init(NULL);
}

atomic_c::atomic_c( const atomic_c& src )
{
	Copy(src);
}

void atomic_c::operator=( const atomic_c& src )
{
	Copy(src);
}

atomic_c::~atomic_c( void )
{
	//
}

void atomic_c::Init( long value )
{
    m_nValue = value;
}

void atomic_c::Copy( const atomic_c& src )
{
    m_nValue = src.m_nValue;
}
