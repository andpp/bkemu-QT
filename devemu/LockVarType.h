#pragma once
#include "pch.h"

#ifdef _LINUX

#ifdef _WIN64
#define DWORD_ALIGN    __declspec(align(8))
#else
#define DWORD_ALIGN    __declspec(align(4))
#endif
#else
#define DWORD_ALIGN alignas(8)
#endif

#define LOCK_VAR_TYPE  DWORD_ALIGN long volatile

inline long InterlockedExchangeAdd( long* Addend, long Increment )
{
long ret;
__asm__ (
/* lock for SMP systems */
"lock\n\t"
"xaddl %0,(%1)"
:"=r" (ret)
:"r" (Addend), "0" (Increment)
:"memory" );
return ret;
}

inline long InterlockedIncrement( long* Addend )
{
    return InterlockedExchangeAdd( Addend, 1 );
}

inline long InterlockedDecrement( long* Addend )
{
    return InterlockedExchangeAdd( Addend, -1 );
}

inline long InterlockedCompareExchange(volatile long *dest, long exch, long comp)
{
    long old;

    __asm__ __volatile__ ("lock; cmpxchg %2, %0"
        : "=m" (*dest), "=a" (old)
        : "r" (exch), "m" (*dest), "a" (comp));
    return(old);
}

inline long InterlockedExchange(long* volatile Target, long Value)
{
    long ReturnValue;

    __asm __volatile("lock xchgl %2,(%1)"
        : "=r" (ReturnValue)
        : "r" (Target), "0" (Value)
        : "memory");

    return ReturnValue;
}

// Вот пример класса LockVarType, создающего объекты быстрой межпоточной синхронизации размером в одно слово.
// by Patron

class LockVarType
{
	public:

		LockVarType() : dwLockVar(0)
		{
		}

		inline DWORD IsLocked()
		{
			return dwLockVar;
		}

		inline void Lock(int nSleepMS = 0)
		{
          while (InterlockedCompareExchange(&dwLockVar, 1, 0))
//            while (std::atomic_compare_exchange_strong(&dwLockVar, 1, 0))
            {
				Sleep(nSleepMS);
			}
		}

		inline void UnLock()
		{
			dwLockVar = 0;
		}

		inline BOOL TimedLock(DWORD uTimeOut_MS, int nSleepMS = 0)
		{
			DWORD uTC = GetTickCount();

            while (InterlockedCompareExchange(&dwLockVar, 1, 0))
//            while (std::atomic_compare_exchange_strong(&dwLockVar, 1, 0))
            {
				if (GetTickCount() - uTC >= uTimeOut_MS)
				{
					return FALSE;
				}

				Sleep(nSleepMS);
			}

			return TRUE;
		}

		inline BOOL TryLock()
		{
            return !InterlockedCompareExchange(&dwLockVar, 1, 0);
//            return (std::atomic_compare_exchange_strong(&dwLockVar, 1, 0))
        }

	protected:
        LOCK_VAR_TYPE dwLockVar;

};
