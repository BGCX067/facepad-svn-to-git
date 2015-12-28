
#ifndef _ATOMIC_C_H_
#define _ATOMIC_C_H_

#pragma once

class atomic_c
{
public:
	long m_nValue;

public:
	atomic_c(void);
	virtual ~atomic_c(void);

	atomic_c(const atomic_c& src);
	void operator=(const atomic_c& src);

protected:
    void __inline Copy(const atomic_c& src);

private:
	void __inline Init(long value);
};

#endif  // _ATOMIC_C_H_
