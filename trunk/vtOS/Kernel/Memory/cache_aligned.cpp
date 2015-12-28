
#include "..\vtCommon.h"
#include <stdlib.h>
#include <memory.h>
#include <crtdbg.h>
#include "cache_aligned.h"

static unsigned char _cAlignSignFill    = 0xE9;     /* fill no-man's sign for aligned routines */
static unsigned char _cClearSignFill    = 0x00;     /* fill no-man's sign for free routines */

// MS1B: BitScanForward
unsigned cache_aligned_t::_NearestPowerOf2( unsigned x )
{
#if 0
    unsigned MS1B = 1;
	while (MS1B < x)
		MS1B <<= 1;

    return MS1B;
#else
    // MS1B
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
#endif
}

/*******************************************************************************
*static int _CheckBytes() - verify byte range set to proper value
*
*Purpose:
*       verify byte range set to proper value
*
*Entry:
*       unsigned char *pb       - pointer to start of byte range
*       unsigned char bCheck    - value byte range should be set to
*       size_t nSize            - size of byte range to be checked
*
*Return:
*       TRUE - if all bytes in range equal bcheck
*       FALSE otherwise
*
*******************************************************************************/
bool __cdecl cache_aligned_t::_CheckBytes(
        unsigned char *pb,
        unsigned char bCheck,
        size_t nSize
        )
{
        bool bOkay = true;
        while (nSize--)
        {
            if (*pb++ != bCheck)
            {
/* Internal error report is just noise; calling functions all report results - JWM */
/*                _RPT3(_CRT_WARN, "memory check error at 0x%p = 0x%02X, should be 0x%02X.\n", */
/*                    (BYTE *)(pb-1),*(pb-1), bCheck); */
                bOkay = false;
            }
        }
        return bOkay;
}

cache_aligned_t::cache_aligned_t( void )
{
	Init(DEFAILT_CACHE_ALIGN_SIZE, true);
}

cache_aligned_t::cache_aligned_t( size_t nSize,
                                 size_t nAlignSize, /*= DEFAILT_CACHE_ALIGN_SIZE */
                                 bool bAutoDelete /*= false */)
{
    Init(nAlignSize, bAutoDelete);
    Malloc(nSize);
}

cache_aligned_t::cache_aligned_t( const cache_aligned_t& src, bool bCopyData /*= true */ )
{
    if (bCopyData)
        Copy(src, true);
    else
        Clone(src, true);
}

void cache_aligned_t::operator =( const cache_aligned_t& src )
{
    Copy(src);
}

cache_aligned_t::~cache_aligned_t( void )
{
    if (m_bAutoDelete)
	    Free(true);
}

void cache_aligned_t::Init( size_t nAlignSize /*= DEFAILT_CACHE_ALIGN_SIZE */,
                           bool bAutoDelete /*= true */ )
{
    m_pvData  = NULL;
    m_pvAlloc = NULL;

    m_nSize      = 0;
    m_nAllocSize = 0;

    SetAlignSize(nAlignSize);

    m_bAutoDelete = bAutoDelete;
    m_bInited = false;
}

void cache_aligned_t::Free( bool bForceDelete /*= false */ )
{
    if (m_bInited) {
        bool bNeedDelete = (bForceDelete || m_bAutoDelete);
        if (bNeedDelete) {
            _ASSERT(m_pvAlloc != NULL);
            if (m_pvAlloc != NULL) {
                ALIGN_BLOCK_HEADER *pBlockHdr;
                void *pvAlloc;
                _ASSERT(m_pvData != NULL);
                if (m_pvData != NULL) {
                    pBlockHdr = (ALIGN_BLOCK_HEADER *)((unsigned int)m_pvData & ~(sizeof(unsigned int) - 1)) - 1;
                    _ASSERT(pBlockHdr < m_pvData);

                    pvAlloc = _FreeBlockHeader(pBlockHdr);
                    _ASSERT(pvAlloc != NULL);
                }
                ::free(m_pvAlloc);
            }

            m_pvAlloc = NULL;
            m_pvData  = NULL;

            m_nSize      = 0;
            m_nAllocSize = 0;

            m_bInited = false;
        }
    }
}

// full copy, include struct and data
bool cache_aligned_t::Copy( const cache_aligned_t& src, bool bIsInit /*= false */ )
{
    if (!bIsInit)
        Free(true);
    Init(src.AlignSize(), src.GetAutoDelete());
	Malloc(src.Size());
    return (CopyData(src) != NULL);
}

// only copy struct, not copy data
void cache_aligned_t::Clone( const cache_aligned_t& src, bool bIsInit /*= false */ )
{
    if (!bIsInit)
        Free(true);
    Init(src.AlignSize(), src.GetAutoDelete());
	Malloc(src.Size());
}

// only copy data
void *cache_aligned_t::CopyData( const cache_aligned_t& src )
{
    return MemCopy_s(src.GetPtr(), src.Size());
}

size_t cache_aligned_t::_StdAlignSize( size_t nAlignSize )
{
    nAlignSize = _NearestPowerOf2(nAlignSize);
    _ASSERT(IS_POWER_OF_2(nAlignSize));

    nAlignSize = (nAlignSize > sizeof(unsigned int)) ? nAlignSize : sizeof(unsigned int);
    _ASSERT(nAlignSize > 0);

    _ASSERT(nAlignSize >= MIN_CACHE_ALIGN_SIZE);
    if (nAlignSize < MIN_CACHE_ALIGN_SIZE)
        nAlignSize = MIN_CACHE_ALIGN_SIZE;

    _ASSERT(nAlignSize <= MAX_CACHE_ALIGN_SIZE);
    if (nAlignSize > MAX_CACHE_ALIGN_SIZE)
        nAlignSize = MAX_CACHE_ALIGN_SIZE;

    return nAlignSize;
}

void cache_aligned_t::SetAlignSize( size_t nAlignSize )
{
    m_nAlignSize = _StdAlignSize(nAlignSize);
}

void *cache_aligned_t::Malloc( size_t nSize, bool bForceRealloc /*= false */ )
{
    if (!IsInited() || bForceRealloc)
        return Realloc(nSize);
    else
        return NULL;
}

void *cache_aligned_t::Realloc( size_t nSize )
{
    // Release previous alloc memory data first
    Free(true);

    ALIGN_BLOCK_HEADER *pBlockHdr;

    size_t nAlignSize = _StdAlignSize(m_nAlignSize);
    size_t nAlignMask = nAlignSize - 1;

    size_t nAllocSize;
    // alloc size align to nAlignSize bytes (isn't must need)
    nAllocSize = (nSize + nAlignMask * 2 + sizeof(ALIGN_BLOCK_HEADER)) & (~nAlignMask);

    if (nAllocSize > 0 && nAllocSize >= (nSize + sizeof(ALIGN_BLOCK_HEADER))) {
        void *pvAlloc = (void *)malloc(nAllocSize);
        if (pvAlloc != NULL) {
            // Save pvAlloc's value first
            m_pvAlloc = pvAlloc;
            // Data pointer align to nAlignSize bytes
            if (sizeof(UINT *) <= sizeof(UINT)) {
                m_pvData = (void *)(((size_t)(unsigned char *)pvAlloc + nAlignMask + sizeof(ALIGN_BLOCK_HEADER))
                    & (~nAlignMask));
            }
            else {
                m_pvData = (void *)(((unsigned __int64)pvAlloc + nAlignMask + sizeof(ALIGN_BLOCK_HEADER))
                    & (~((unsigned __int64)nAlignMask)));
            }

            pBlockHdr = (ALIGN_BLOCK_HEADER *)(m_pvData) - 1;
            _ASSERT(pBlockHdr >= pvAlloc);

            pBlockHdr->pvAlloc = pvAlloc;
            memset((void *)pBlockHdr->Sign, _cAlignSignFill, ALIGN_SIGN_SIZE);

            m_nSize      = nSize;
            m_nAllocSize = nAllocSize;

            // for debug
            int nFrontPaddedSize = FrontPaddedSize();
            int nLastPaddedSize  = LastPaddedSize();
            //nFrontPaddedSize = (unsigned long)m_pvData - (unsigned long)m_pvAlloc;
            //nLastPaddedSize  = nAllocSize - nSize - sizeof(ALIGN_BLOCK_HEADER) - nFrontPaddedSize;

            _ASSERT(nFrontPaddedSize >= sizeof(ALIGN_BLOCK_HEADER));
            _ASSERT(nLastPaddedSize >= 0);

            m_bInited = true;

            return m_pvData;
        }
        return NULL;
    }
    else {
        // alloc size overflow or error
        return NULL;
    }
}

void *cache_aligned_t::_FreeBlockHeader( ALIGN_BLOCK_HEADER *pBlockHdr,
                                        bool bFreeMemBlock /* = false*/)
{
    void *pvAlloc = NULL;

    _ASSERT(pBlockHdr != NULL);
    if (pBlockHdr != NULL) {
        pvAlloc = pBlockHdr->pvAlloc;
        _ASSERT((void *)pBlockHdr >= pvAlloc);

        if ((void *)pBlockHdr < pvAlloc) {
            // We don't know where pvData was allocated
            _RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", pvAlloc);
            pvAlloc = (void *)-2;
        }
        else {
            if (_CheckBytes(pBlockHdr->Sign, _cAlignSignFill, ALIGN_SIGN_SIZE)) {
                // Set and fill clear sign 
                memset(pBlockHdr->Sign, _cClearSignFill, ALIGN_SIGN_SIZE);

                // Set pvAlloc's value to NULL
                pBlockHdr->pvAlloc = NULL;

                // Free memory block if need
                if (pvAlloc != NULL) {
                    if (bFreeMemBlock)
                        free(pvAlloc);
                }
            }
            else {
                // We don't know where pvData was allocated
                _RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", pvAlloc);
                pvAlloc = (void *)-1;
            }
        }
    }
    return pvAlloc;
}

void *cache_aligned_t::FreeBlock( const void *pvData )
{
    ALIGN_BLOCK_HEADER *pBlockHdr;
    void *pvAlloc = NULL;

    _ASSERT(pvData != NULL);
    if (pvData != NULL) {
        pBlockHdr = (ALIGN_BLOCK_HEADER *)((unsigned int)pvData & ~(sizeof(unsigned int) - 1)) - 1;
        _ASSERT(pBlockHdr < pvData);

        pvAlloc = _FreeBlockHeader(pBlockHdr, true);
        _ASSERT(pvAlloc != NULL);
    }
    return pvAlloc;
}

void *cache_aligned_t::MemSet( int nValue )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0)
            return memset(m_pvData, nValue, m_nSize);
    }
    return NULL;
}

void *cache_aligned_t::MemCopy( const void *src, size_t count )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            count = (count > m_nSize) ? m_nSize : count;
            return memcpy(m_pvData, src, count);
        }
    }
    return NULL;
}

void *cache_aligned_t::MemCopy_s( const void *src, size_t count )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            errno_t err = memcpy_s(m_pvData, m_nSize, src, count);
            if (err == 0)
                return m_pvData;
        }
    }
    return NULL;
}

void *cache_aligned_t::MemMove( const void *src, size_t count )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            count = (count > m_nSize) ? m_nSize : count;
            return memmove(m_pvData, src, count);
        }
    }
    return NULL;
}

void *cache_aligned_t::MemMove_s( const void *src, size_t count )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            errno_t err = memmove_s(m_pvData, m_nSize, src, count);
            if (err == 0)
                return m_pvData;
        }
    }
    return NULL;
}

void *cache_aligned_t::MemChr( int c )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            return memchr(m_pvData, c, m_nSize);
        }
    }
    return NULL;
}

void *cache_aligned_t::MemChr( size_t offset, int c )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            if (offset < m_nSize)
                return memchr((unsigned char *)m_pvData + offset, c, (m_nSize - offset));
        }
    }
    return NULL;
}

int cache_aligned_t::MemCmp( const void *buf, size_t count )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            count = (count > m_nSize) ? m_nSize : count;
            return memcmp((unsigned char *)m_pvData, buf, count);
        }
    }
    return NULL;
}

int cache_aligned_t::MemICmp( const void *buf, size_t count )
{
    if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            count = (count > m_nSize) ? m_nSize : count;
            return _memicmp((unsigned char *)m_pvData, buf, count);
        }
    }
    return NULL;
}

int cache_aligned_t::MemICmp_l( const void *buf, size_t count, _locale_t locale )
{
        if (IsInited()) {
        if (m_pvAlloc != NULL && m_pvData != NULL && m_nSize > 0) {
            count = (count > m_nSize) ? m_nSize : count;
            return _memicmp_l((unsigned char *)m_pvData, buf, count, locale);
        }
    }
    return NULL;
}
