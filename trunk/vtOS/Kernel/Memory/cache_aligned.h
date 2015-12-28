#ifndef _CACHE_ALIGNED_T_H_
#define _CACHE_ALIGNED_T_H_

#pragma once

#define DEFAILT_CACHE_ALIGN_SIZE    128

// 32bit CPU max cache line align size
#define MIN_CACHE_ALIGN_SIZE        64
// <= 8M Bytes
#define MAX_CACHE_ALIGN_SIZE        0x00800000UL

#define ALIGN_SIGN_SIZE             sizeof(void *)

#define IS_POWER_OF_2(x)            (((x)&(x-1)) == 0)

typedef struct _ALIGN_BLOCK_HEADER
{
    void            *pvAlloc;
    unsigned char   Sign[ALIGN_SIGN_SIZE];
} ALIGN_BLOCK_HEADER, *PALIGN_BLOCK_HEADER;

class cache_aligned_t
{
private:
	void   *m_pvData;
    void   *m_pvAlloc;

    size_t  m_nAlignSize;
    size_t  m_nSize;
    size_t  m_nAllocSize;

    bool    m_bAutoDelete;
    bool    m_bInited;

public:
	cache_aligned_t(void);
	cache_aligned_t(size_t nSize,
        size_t nAlignSize = DEFAILT_CACHE_ALIGN_SIZE,
        bool bAutoDelete = true);
    cache_aligned_t(const cache_aligned_t& src, bool bCopyData = true);
    void operator =(const cache_aligned_t& src);
    virtual ~cache_aligned_t(void);	

public:
    static void *FreeBlock(const void *pvData);
    static unsigned _NearestPowerOf2(unsigned x);
    static bool __cdecl _CheckBytes(unsigned char *, unsigned char, size_t);

    size_t  AlignSize(void) const { return m_nAlignSize; };
    size_t  Size(void) const { return m_nSize; };
    size_t  AllocSize(void) const { return m_nAllocSize; };

    bool    GetAutoDelete(void) const { return m_bAutoDelete; };
    bool    SetAutoDelete(bool bAutoDelete) {
        bool bOldValue = m_bAutoDelete;
        m_bAutoDelete = bAutoDelete;
        return bOldValue;
    };

    int     FrontPaddedSize(void) const {
        return (int)((unsigned char *)m_pvData - (unsigned char *)m_pvAlloc);
    };
    int     LastPaddedSize (void) const {
        return (int)(m_nAllocSize - m_nSize - sizeof(ALIGN_BLOCK_HEADER) - FrontPaddedSize());
    };

    bool    IsInited(void)       { return m_bInited; };
    void   *GetPtr(void) const   { return m_pvData;  };
    void   *DataPtr(void) const  { return m_pvData;  };
    void   *AllocPtr(void) const { return m_pvAlloc; };

    void   *Malloc   (size_t nSize, bool bForceRealloc = false);
    void   *Realloc  (size_t nSize);
    bool    Copy     (const cache_aligned_t& src, bool bIsInit = false); // full copy, include struct and data
    void    Clone    (const cache_aligned_t& src, bool bIsInit = false); // only copy struct, not copy data
    void   *CopyData (const cache_aligned_t& src);                       // only copy data
    void    Free     (bool bForceDelete = false);

    void   *MemSet   (int nValue);
    void   *MemCopy  (const void *src, size_t count);
    void   *MemMove  (const void *src, size_t count);
    void   *MemCopy_s(const void *src, size_t count);
    void   *MemMove_s(const void *src, size_t count);

    void   *MemChr   (int c);
    void   *MemChr   (size_t offset, int c);

    int     MemCmp   (const void *buf, size_t count);
    int     MemICmp  (const void *buf, size_t count);
    int     MemICmp_l(const void *buf, size_t count, _locale_t locale);

protected:
    void    Init(size_t nAlignSize = DEFAILT_CACHE_ALIGN_SIZE, bool bAutoDelete = true);
    void    SetAlignSize(size_t nAlignSize);

private:
    static void *_FreeBlockHeader(ALIGN_BLOCK_HEADER *pBlockHdr, bool bFreeMemBlock = false);
    size_t  _StdAlignSize(size_t nAlignSize);
	
};

#endif  // _CACHE_ALIGNED_T_H_
