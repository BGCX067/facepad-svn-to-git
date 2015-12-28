
#ifndef _ATOMIC_T_H_
#define _ATOMIC_T_H_

#pragma once

#include <stdio.h>
//#include <windows.h>
//#include <winnt.h>

typedef int INT;
typedef unsigned int UINT;

#if __GNUC__
#define _INLINE __inline__
#elif _WIN32||defined(__INTEL_COMPILER)
#define _INLINE __inline
#else
#define _INLINE inline
#endif

#define ATOMIC_ALIGN_SIZE   64

#if __GNUC__ || __SUNPRO_CC

#define _DECL_ALIGN_FIELD(t,f,a)            t f  __attribute__ ((aligned(a)));
#define _DECL_ALIGN_FIELD_VOLATILE(t,f,a)   volatile t f  __attribute__ ((aligned(a)));

#elif defined(__INTEL_COMPILER)||_MSC_VER >= 1300

#define _DECL_ALIGN_FIELD(t,f,a)            __declspec(align(a)) t f;
#define _DECL_ALIGN_FIELD_VOLATILE (t,f,a)  volatile __declspec(align(a)) t f;

#else

#define _DECL_ALIGN_FIELD(t,f,a)            t f;
#define _DECL_ALIGN_FIELD_VOLATILE (t,f,a)  volatile t f;
#error Do not know syntax for forcing alignment.

#endif /* __GNUC__ */

#if __GNUC__

#include <include/asm/atomic.h>

#elif _WIN32||defined(__INTEL_COMPILER)||_MSC_VER >= 1300

/**
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */
#define ATOMIC_INIT(i)  { (i) } 

#if __GNUC__ || __SUNPRO_CC

typedef struct _atomic_t __attribute__ aligned(ATOMIC_ALIGN_SIZE) {
    volatile INT counter;
} atomic_t, *patomic_t; 

#elif defined(__INTEL_COMPILER)||_MSC_VER >= 1300

typedef __declspec(align(ATOMIC_ALIGN_SIZE)) struct _atomic_t {
    volatile INT counter;
} atomic_t, *patomic_t; 

#else

typedef struct _atomic_t {
    volatile INT counter;
} atomic_t, *patomic_t; 

#endif

static
_INLINE
INT atomic_read(const atomic_t *v)
{
    return (*(volatile int *)&(v)->counter);
}

static
_INLINE
void atomic_set(atomic_t *v, INT i)
{
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov edx, i
        mov eax, dword ptr [ecx]
cmpxchg_loop:
        // 为什么是跳转到这里? 因为执行一次cmpxchg以后, 如果eax的值跟[ecx]的值不一样,
        // [ecx]的值会写入到eax, 可以省略一次[ecx]到eax的赋值, 不会出现死循环
        lock cmpxchg dword ptr [ecx], edx
        jne short cmpxchg_loop
    }
}

static
_INLINE
INT atomic_set_return(atomic_t *v, INT i)
{
    INT val;
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov edx, i
        mov eax, dword ptr [ecx]
cmpxchg_loop:
        lock cmpxchg dword ptr [ecx], edx
        jne short cmpxchg_loop
        mov val, eax
    }
    return val;
}

static
_INLINE
void atomic_set_fast(atomic_t *v, INT i)
{
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov edx, i
        mov eax, dword ptr [ecx]
        lock xchg dword ptr [ecx], edx
    }
}

static
_INLINE
INT atomic_set_fast_return(atomic_t *v, INT i)
{
    INT val;
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov edx, i
        mov eax, dword ptr [ecx]
        lock xchg dword ptr [ecx], edx
        mov val, eax
    }
    return val;
}

static
_INLINE
void atomic_add(INT i, atomic_t *v)
{
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov eax, i
        lock xadd dword ptr [ecx], eax
    }
}

static
_INLINE
void atomic_sub(INT i, atomic_t *v)
{
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov eax, i
        neg eax
        lock xadd dword ptr [ecx], eax
    }
}

static
_INLINE
void atomic_inc(atomic_t *v)
{
    //int val = v->counter;
    __asm
    {
        mov edx, dword ptr [v] atomic_t.counter
        lock inc dword ptr [edx]
        //emms
    }
    //InterlockedAdd((LONG volatile*)&(v->counter), 6);
    //InterlockedExchange((LONG volatile*)&(v->counter), 6);
    //InterlockedCompareExchange((LONG volatile*)&(v->counter), 5, 6);
}

static
_INLINE
void atomic_dec(atomic_t *v)
{
    __asm
    {
        mov edx, dword ptr [v] atomic_t.counter
        lock dec dword ptr [edx]
    }
}

static
_INLINE
INT atomic_add_return(INT i, atomic_t *v)
{
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov eax, i
        lock xadd dword ptr [ecx], eax
    }
    return v->counter;
}

static
_INLINE
INT atomic_sub_return(INT i, atomic_t *v)
{
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov eax, i
        neg eax
        lock xadd dword ptr [ecx], eax
    }
    return v->counter;
}

static
_INLINE
INT atomic_inc_return(atomic_t *v)
{
    __asm
    {
        mov edx, dword ptr [v] atomic_t.counter
        lock inc dword ptr [edx]
    }
    return v->counter;
}

static
_INLINE
INT atomic_dec_return(atomic_t *v)
{
    __asm
    {
        mov edx, dword ptr [v] atomic_t.counter
        lock dec dword ptr [edx]
    }
    return v->counter;
}

/********************************************************************************/
/* cmpxchg ecx, edx；如果EAX与ECX相等，则EBX送EBX且ZF置1；否则ECX送EAX，且ZF清0 */
/********************************************************************************/

static
_INLINE
INT atomic_xchg(atomic_t *v, INT i)
{
    INT val;
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov edx, i
        mov eax, dword ptr [ecx]
        lock xchg dword ptr [ecx], edx
        mov val, eax
    }
    return val;
}

static
_INLINE
INT atomic_cmpxchg(atomic_t *v, INT _old, INT _new)
{
    INT val;
    __asm
    {
        mov ecx, dword ptr [v] atomic_t.counter
        mov edx, _new
        mov eax, _old
        lock cmpxchg dword ptr [ecx], edx
        mov val, eax
    }
    return val;
}

static
_INLINE
INT atomic_add_unless(atomic_t *v, INT a, INT u)
{
    INT c, old;
    c = atomic_read(v);
    for (;;) {
        if (c == u)
            break;
        // 如果v的值等于c就加a，old=c。如果v的值不等于c，old=v的值
        old = atomic_cmpxchg(v, c, c + a);
        if (old == c)
            break;
        c = old;
    }
    return c != u;
}

static
_INLINE
INT atomic_set_mask(atomic_t *v, INT mask)
{
    UINT val = (UINT)v->counter;
    val |= mask;
    return atomic_set_return(v, (INT)val);
}

static
_INLINE
INT atomic_clear_mask(atomic_t *v, INT mask)
{
    UINT val = (UINT)v->counter;
    val &= ~mask;
    return atomic_set_return(v, (INT)val);
}

#define atomic_init(v, i)           atomic_set((v), (i))

////////////////////////////////////////////////////////////////////////
//
// test for 0
//

#define atomic_add_and_test(i, v)   (atomic_add_return((i), (v)) == 0)
#define atomic_sub_and_test(i, v)   (atomic_sub_return((i), (v)) == 0)

#define atomic_inc_and_test(v)      (atomic_inc_return(v) == 0)
#define atomic_dec_and_test(v)      (atomic_dec_return(v) == 0)

#define atomic_add_negative(i, v)   (atomic_add_return((i), (v)) < 0)
#define atomic_sub_negative(i, v)   (atomic_sub_return((i), (v)) < 0)

#define atomic_inc_negative(v)      (atomic_inc_return(v) < 0)
#define atomic_dec_negative(v)      (atomic_dec_return(v) < 0)

#define atomic_add_negative(i, v)   (atomic_add_return((i), (v)) < 0)
#define atomic_sub_negative(i, v)   (atomic_sub_return((i), (v)) < 0)

#define atomic_inc_negative(v)      (atomic_inc_return(v) < 0)
#define atomic_dec_negative(v)      (atomic_dec_return(v) < 0)

////////////////////////////////////////////////////////////////////////
//
// compare with u
//

#define atomic_add_and_cmp(i,v,u)   (atomic_add_return((i), (v)) == (u))
#define atomic_sub_and_cmp(i,v,u)   (atomic_sub_return((i), (v)) == (u))

#define atomic_inc_and_cmp(v, u)    (atomic_inc_return(v) == (u))
#define atomic_dec_and_cmp(v, u)    (atomic_dec_return(v) == (u))

#define atomic_add_and_less(i,v,u)  (atomic_add_return((i), (v)) < (u))
#define atomic_sub_and_less(i,v,u)  (atomic_sub_return((i), (v)) < (u))

#define atomic_inc_and_less(v, u)   (atomic_inc_return(v) < (u))
#define atomic_dec_and_less(v, u)   (atomic_dec_return(v) < (u))

#define atomic_add_and_more(i,v,u)  (atomic_add_return((i), (v)) > (u))
#define atomic_sub_and_more(i,v,u)  (atomic_sub_return((i), (v)) > (u))

#define atomic_inc_and_more(v, u)   (atomic_inc_return(v) > (u))
#define atomic_dec_and_more(v, u)   (atomic_dec_return(v) > (u))

////////////////////////////////////////////////////////////////////////

#define atomic_inc_not_zero(v)      atomic_add_unless((v), 1, 0)

#endif

#endif  // _ATOMIC_T_H_
