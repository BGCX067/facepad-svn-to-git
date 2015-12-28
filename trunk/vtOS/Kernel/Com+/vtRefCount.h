#pragma once

class vtRefCount
{
private:
    long m_nRecCount;

public:
	vtRefCount(void);
    vtRefCount(long nRecCount);
	virtual ~vtRefCount(void);

    inline long _AddRef(void)  { m_nRecCount++;; return m_nRecCount; }
    inline long _Release(void) { m_nRecCount--;; return m_nRecCount; }

    inline long GetRefCount(void) { return m_nRecCount; }
    inline long SetRefCount(long nRecCount) { m_nRecCount = nRecCount; return m_nRecCount; }

protected:
	vtRefCount(const vtRefCount& objectSrc);		// no implementation
	void operator=(const vtRefCount& objectSrc);	// no implementation

private:
    inline void initRefCount(long nRecCount) { m_nRecCount = nRecCount; }
};
