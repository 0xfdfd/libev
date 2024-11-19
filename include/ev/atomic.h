#ifndef __EV_ATOMIC_H__
#define __EV_ATOMIC_H__

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
/*
 * For compiler that support C11 and atomic, just use the standard header.
 */
#include <stdatomic.h>

typedef atomic_int_fast32_t ev_atomic32_t;
typedef atomic_int_fast64_t ev_atomic64_t;

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_init
 * @{
 */
#define ev_atomic32_init(obj, desired)  atomic_init(obj, desired)
#define ev_atomic64_init(obj, desired)  atomic_init(obj, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_store
 * @{
 */
#define ev_atomic32_store(obj, desired) atomic_store(obj, desired)
#define ev_atomic64_store(obj, desired) atomic_store(obj, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_load
 * @{
 */
#define ev_atomic32_load(obj) atomic_load(obj)
#define ev_atomic64_load(obj) atomic_load(obj)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_exchange
 * @{
 */
#define ev_atomic32_exchange(obj, desired) atomic_exchange(obj, desired)
#define ev_atomic64_exchange(obj, desired) atomic_exchange(obj, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_compare_exchange
 * @{
 */
#define ev_atomic32_compare_exchange_strong(obj, expected, desired) atomic_compare_exchange_strong(obj, expected, desired)  
#define ev_atomic64_compare_exchange_strong(obj, expected, desired) atomic_compare_exchange_strong(obj, expected, desired)
#define ev_atomic32_compare_exchange_weak(obj, expected, desired) atomic_compare_exchange_weak(obj, expected, desired)
#define ev_atomic64_compare_exchange_weak(obj, expected, desired) atomic_compare_exchange_weak(obj, expected, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_add
 * @{
 */
#define ev_atomic32_fetch_add(obj, arg) atomic_fetch_add(obj, arg)
#define ev_atomic64_fetch_add(obj, arg) atomic_fetch_add(obj, arg)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_sub
 * @{
 */
#define ev_atomic32_fetch_sub(obj, arg) atomic_fetch_sub(obj, arg)
#define ev_atomic64_fetch_sub(obj, arg) atomic_fetch_sub(obj, arg)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_or
 * @{
 */
#define ev_atomic32_fetch_or(obj, arg)  atomic_fetch_or(obj, arg)
#define ev_atomic64_fetch_or(obj, arg)  atomic_fetch_or(obj, arg)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_xor
 * @{
 */
#define ev_atomic32_fetch_xor(obj, arg) atomic_fetch_xor(obj, arg)
#define ev_atomic64_fetch_xor(obj, arg) atomic_fetch_xor(obj, arg)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_and
 * @{
 */
#define ev_atomic32_fetch_and(obj, arg) atomic_fetch_and(obj, arg)
#define ev_atomic64_fetch_and(obj, arg) atomic_fetch_and(obj, arg)
/**
 * @}
 */

#elif defined(_WIN32)

#define EV_ATOMIC_WIN32 1

#include <Windows.h>

typedef LONG ev_atomic32_t;
typedef LONG64 ev_atomic64_t;

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_init
 * @{
 */
#define ev_atomic32_init(obj, desired)  (*(obj) = (desired))
#define ev_atomic64_init(obj, desired)  (*(obj) = (desired))
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_store
 * @{
 */
#define ev_atomic32_store(obj, desired) ((void)InterlockedExchange(obj, desired))
#define ev_atomic64_store(obj, desired) ((void)InterlockedExchange64(obj, desired))
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_load
 * @{
 */
#define ev_atomic32_load(obj) InerlockedOr(obj, 0)
#define ev_atomic64_load(obj) InerlockedOr64(obj, 0)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_exchange
 * @{
 */
#define ev_atomic32_exchange(obj, desired) InterlockedExchange(obj, desired)
#define ev_atomic64_exchange(obj, desired) InterlockedExchange64(obj, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_compare_exchange
 * @{
 */
EV_API int ev_atomic32_compare_exchange_strong(volatile ev_atomic32_t* obj, int32_t* expected, int32_t desired);
EV_API int ev_atomic64_compare_exchange_strong(volatile ev_atomic64_t* obj, int64_t* expected, int64_t desired);
#define ev_atomic32_compare_exchange_weak(obj, expected, desired) ev_atomic32_compare_exchange_strong(obj, expected, desired)
#define ev_atomic64_compare_exchange_weak(obj, expected, desired) ev_atomic64_compare_exchange_strong(obj, expected, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_add
 * @{
 */
#define ev_atomic32_fetch_add(obj, arg) (InterlockedAdd((obj), (arg)) - (arg))
#define ev_atomic64_fetch_add(obj, arg) (InterlockedAdd64((obj), (arg)) - (arg))
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_sub
 * @{
 */
#define ev_atomic32_fetch_sub(obj, arg) (InterlockedAdd(obj, -(arg)) + (arg))
#define ev_atomic64_fetch_sub(obj, arg) (InterlockedAdd64(obj, -(arg)) + (arg))
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_or
 * @{
 */
#define ev_atomic32_fetch_or(obj, arg)  InterlockedOr(obj, arg)
#define ev_atomic64_fetch_or(obj, arg)  InterlockedOr64(obj, arg)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_xor
 * @{
 */
#define ev_atomic32_fetch_xor(obj, arg) InterlockedXor(obj, arg)
#define ev_atomic64_fetch_xor(obj, arg) InterlockedXor64(obj, arg)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_and
 * @{
 */
#define ev_atomic32_fetch_and(obj, arg) InterlockedAnd(obj, arg)
#define ev_atomic64_fetch_and(obj, arg) InterlockedAnd64(obj, arg)
/**
 * @}
 */

#elif defined(__GNUC__) || defined(__clang__)

typedef int32_t ev_atomic32_t;
typedef int64_t ev_atomic64_t;

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_init
 * @{
 */
#define ev_atomic32_init(obj, desired)  ev_atomic32_store(obj, desired)
#define ev_atomic64_init(obj, desired)  ev_atomic64_store(obj, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_store
 * @{
 */
#define ev_atomic32_store(obj, desired) __atomic_store_n(obj, desired, __ATOMIC_SEQ_CST)
#define ev_atomic64_store(obj, desired) __atomic_store_n(obj, desired, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_load
 * @{
 */
#define ev_atomic32_load(obj) __atomic_load_n(obj, __ATOMIC_SEQ_CST)
#define ev_atomic64_load(obj) __atomic_load_n(obj, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_exchange
 * @{
 */
#define ev_atomic32_exchange(obj, desired) __atomic_exchange_n(obj, desired, __ATOMIC_SEQ_CST)
#define ev_atomic64_exchange(obj, desired) __atomic_exchange_n(obj, desired, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_compare_exchange
 * @{
 */
#define ev_atomic32_compare_exchange_strong(obj, expected, desired) __atomic_compare_exchange_n(obj, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define ev_atomic64_compare_exchange_strong(obj, expected, desired) __atomic_compare_exchange_n(obj, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define ev_atomic32_compare_exchange_weak(obj, expected, desired)   __atomic_compare_exchange_n(obj, expected, desired, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define ev_atomic64_compare_exchange_weak(obj, expected, desired)   __atomic_compare_exchange_n(obj, expected, desired, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_add
 * @{
 */
#define ev_atomic32_fetch_add(obj, arg) __atomic_fetch_add(obj, arg, __ATOMIC_SEQ_CST)
#define ev_atomic64_fetch_add(obj, arg) __atomic_fetch_add(obj, arg, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_sub
 * @{
 */
#define ev_atomic32_fetch_sub(obj, arg) __atomic_fetch_sub(obj, arg, __ATOMIC_SEQ_CST)
#define ev_atomic64_fetch_sub(obj, arg) __atomic_fetch_sub(obj, arg, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_or
 * @{
 */
#define ev_atomic32_fetch_or(obj, arg)  __atomic_fetch_or(obj, arg, __ATOMIC_SEQ_CST)
#define ev_atomic64_fetch_or(obj, arg)  __atomic_fetch_or(obj, arg, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_xor
 * @{
 */
#define ev_atomic32_fetch_xor(obj, arg) __atomic_fetch_xor(obj, arg, __ATOMIC_SEQ_CST)
#define ev_atomic64_fetch_xor(obj, arg) __atomic_fetch_xor(obj, arg, __ATOMIC_SEQ_CST)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_and
 * @{
 */
#define ev_atomic32_fetch_and(obj, arg) __atomic_fetch_and(obj, arg, __ATOMIC_SEQ_CST)
#define ev_atomic64_fetch_and(obj, arg) __atomic_fetch_and(obj, arg, __ATOMIC_SEQ_CST)
/**
 * @}
 */

#else

#define EV_ATOMIC_LOCK_SIMULATION   1

typedef int32_t ev_atomic32_t;
typedef int64_t ev_atomic64_t;

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_init
 * @{
 */
EV_API void ev_atomic32_init(volatile ev_atomic32_t* obj, int32_t desired);
EV_API void ev_atomic64_init(volatile ev_atomic64_t* obj, int64_t desired);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_store
 * @{
 */
EV_API void ev_atomic32_store(volatile ev_atomic32_t* obj, int32_t desired);
EV_API void ev_atomic64_store(volatile ev_atomic64_t* obj, int64_t desired);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_load
 * @{
 */
EV_API int32_t ev_atomic32_load(volatile ev_atomic32_t* obj);
EV_API int64_t ev_atomic64_load(volatile ev_atomic64_t* obj);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_exchange
 * @{
 */
EV_API int32_t ev_atomic32_exchange(volatile ev_atomic32_t* obj, int32_t desired);
EV_API int64_t ev_atomic64_exchange(volatile ev_atomic64_t* obj, int64_t desired);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_compare_exchange
 * @{
 */
EV_API int ev_atomic32_compare_exchange_strong(volatile ev_atomic32_t* obj, int32_t* expected, int32_t desired);
EV_API int ev_atomic64_compare_exchange_strong(volatile ev_atomic64_t* obj, int64_t* expected, int64_t desired);
#define ev_atomic32_compare_exchange_weak(obj, expected, desired) ev_atomic32_compare_exchange_strong(obj, expected, desired)
#define ev_atomic64_compare_exchange_weak(obj, expected, desired) ev_atomic64_compare_exchange_strong(obj, expected, desired)
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_add
 * @{
 */
EV_API int32_t ev_atomic32_fetch_add(volatile ev_atomic32_t* obj, int32_t arg);
EV_API int64_t ev_atomic64_fetch_add(volatile ev_atomic64_t* obj, int64_t arg);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_sub
 * @{
 */
EV_API int32_t ev_atomic32_fetch_sub(volatile ev_atomic32_t* obj, int32_t arg);
EV_API int64_t ev_atomic64_fetch_sub(volatile ev_atomic64_t* obj, int64_t arg);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_or
 * @{
 */
EV_API int32_t ev_atomic32_fetch_or(volatile ev_atomic32_t* obj, int32_t arg);
EV_API int64_t ev_atomic64_fetch_or(volatile ev_atomic64_t* obj, int64_t arg);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_xor
 * @{
 */
EV_API int32_t ev_atomic32_fetch_xor(volatile ev_atomic32_t* obj, int32_t arg);
EV_API int64_t ev_atomic32_fetch_xor(volatile ev_atomic64_t* obj, int64_t arg);
/**
 * @}
 */

/**
 * @see https://en.cppreference.com/w/c/atomic/atomic_fetch_and
 * @{
 */
EV_API int32_t ev_atomic32_fetch_and(volatile ev_atomic32_t* obj, int32_t arg);
EV_API int64_t ev_atomic64_fetch_and(volatile ev_atomic64_t* obj, int64_t arg);
/**
 * @}
 */

#endif

#endif
