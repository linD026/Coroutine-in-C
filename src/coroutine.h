/*
 * coroutine API
 * TODO:
 * 1. use array of pointer to stoe the stack variables. So it may change the
 *    coroutine variable (in heap) to stack at function scope.
 * 2. Use singal() to interrupt the task
 */
#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stddef.h>
#include <stdlib.h>

#ifndef __CONTEXT_H__
#include "context.h"
#endif

// compiler memory barrier
#define cr_cmb() asm volatile("" : : : "memory")

/* The flags for deciding the scheduler of coroutine 
 */
#define CR_DEFAULT 0x0001
#define CR_FIFO 0x0002

#define CR_SCHED_MASK (CR_DEFAULT | CR_FIFO)

/**
 * coroutine_create - Create the coroutine
 * @flags: The bitmask for modifying the behavior of coroutine
 * 
 * The flags can be setted by CR_DEFAULT, CR_FIFO for the scheduler decision.
 * The return value is the fd number of coroutine.
 * When the return value is < 0, its failed.
 */
int coroutine_create(int flags);

/**
 * coroutine_start - Start working the job which added in the coroutine
 * @crfd: The fd of the coroutine want to run.
 *
 * This is blocking function.
 * Return 0 if all the jobs are done.
 */
int coroutine_start(int crfd);

/**
 * coroutine_add - Add the task into the coroutine
 * @crfd: The fd of the coroutine you want to add.
 * @func: The function declared by COROUTINE_DEFINE which want to add in.
 * @args: The arguments are passed into the func.
 *
 * Return value is the fd number of the job.
 * When the return value is < 0, its failed.
 */
int coroutine_add(int crfd, int (*func)(struct context *__context, void *args),
                  void *args);

/**
 * coroutine_join - Join with a terminated coroutine
 * @crfd: The fd of the terminated coroutine 
 * 
 * Return 0 if success.
 */
int coroutine_join(int crfd);

/* The prototype of the job */
#define COROUTINE_DEFINE(name) int name(struct context *__context, void *args)

#define __VAR_DEFINE(type, name, size)                                \
    do {                                                              \
        if (!__context->local[__context->local_offset]) {             \
            name = (type *)malloc(sizeof(type) * size);               \
            __context->local[__context->local_offset] = (void *)name; \
            __context->local_size++;                                  \
        } else                                                        \
            name = (type *)__context->local[__context->local_offset]; \
        __context->local_offset++;                                    \
    } while (0)

/*
 * The marco to define the variable of job function
 * It must happens before the cr_begin marco.
 */
#define VAR_DEFINE(type, name) \
    type *name;                \
    __VAR_DEFINE(type, name, 1)

/*
 * The marco to define the two variables of job function
 * It must happens before the cr_begin marco.
 */
#define VAR_DEFINE2(type, n1, n2) \
    type *n1, *n2;                \
    __VAR_DEFINE(type, n1, 1);    \
    __VAR_DEFINE(type, n2, 1)

/*
 * The marco to define the three variables of job function
 * It must happens before the cr_begin marco.
 */
#define VAR_DEFINE3(type, n1, n2, n3) \
    type *n1, *n2, *n3;               \
    __VAR_DEFINE(type, n1, 1);        \
    __VAR_DEFINE(type, n2, 1);        \
    __VAR_DEFINE(type, n3, 1)

#define ARRAY_DEFINE(type, name, size) \
    type *name;                        \
    __VAR_DEFINE(type, name, size)

#define __VAR_RELEASE()                                       \
    do {                                                      \
        for (int __i = 0; __i < __context->local_size; __i++) \
            free(__context->local[__i]);                      \
    } while (0)

static inline int ____args_count(int cnt, int index, ...)
{
    return cnt ? index : 0;
}

#define ___args_count(...) (sizeof((int[]){ 0, __VA_ARGS__ }) / sizeof(int) - 1)

#define __args_count(...) \
    ____args_count(___args_count(__VA_ARGS__), ##__VA_ARGS__, 0)

#define cr_set(p, val, ...)                         \
    do {                                            \
        int __args_cnt = __args_count(__VA_ARGS__); \
        p[__args_cnt] = (val);                      \
    } while (0)

#define cr_dref(p, ...)                             \
    ({                                              \
        int __args_cnt = __args_count(__VA_ARGS__); \
        p[__args_cnt];                              \
    })

/* Return state */
enum {
    CR_INIT = 0x0000,
    CR_EXIT = ~0x0000,
    CR_YIELD = 0x0001,
    CR_WAIT = 0x0002,
    CR_RETURN_JOB = 0x0004,
    CR_ENALBED = 0x0007,
    CR_DISALBLED = 0x0008,
    CR_CLONE_EXIT = 0x0010,
};

#define ___cr_line(name, line) __cr_##name##line
#define __cr_line(name) ___cr_line(name, __LINE__)

#define __cr_label(state)                                \
    do {                                                 \
        __cr_line(state)                                 \
                : __context->label = &&__cr_line(state); \
    } while (0)

/*
 * Initailizing the job function of coroutine
 *
 * The variable, which defined by CR_DEFINEn() marco, need to be
 * declared before this marco.
 */
#define cr_begin()                   \
    do {                             \
        cr_cmb();                    \
        if (__context->blocked == 0) \
            goto __cr_exit;          \
        __context->local_offset = 0; \
        if (__context->label)        \
            goto * __context->label; \
    } while (0)

#define cr_yield()                                                             \
    do {                                                                       \
        if (__context->blocked >= 0) { /* if >= 0, doesn't in clone handler */ \
            __context->wait_yield = 0;                                         \
            cr_cmb();                                                          \
            __cr_label(CR_YIELD);                                              \
            if (__context->wait_yield == 0)                                    \
                return CR_YIELD;                                               \
        }                                                                      \
    } while (0)

#define cr_wait(cond)                                                          \
    do {                                                                       \
        if (__context->blocked >= 0) { /* if >= 0, doesn't in clone handler */ \
            __context->wait_yield = 0;                                         \
            cr_cmb();                                                          \
            __cr_label(CR_WAIT);                                               \
            if (__context->wait_yield == 0)                                    \
                return CR_WAIT;                                                \
            /* TODO */                                                         \
        }                                                                      \
    } while (0)

#define cr_end()                    \
    do {                            \
    __cr_exit:                      \
        __VAR_RELEASE();            \
        if (__context->blocked < 0) \
            return CR_CLONE_EXIT;   \
        return CR_EXIT;             \
    } while (0)

// TODO
#define cr_enter(tfd)

// TODO
/* 
 * semaphore (lock) API
 */
typedef struct cr_lock {
    volatile unsigned int count;
} cr_lock_t;

#define LOCK_DEFINE

#define cr_lock(p)                   \
    do {                             \
        while (!((p)->count & 0x01)) \
            cr_yield();              \
        (p)->count++;                \
    } while (0)

#define cr_unlock(p)  \
    do {              \
        (p)->count--; \
    } while (0)

/* If set the context->blocked flags, the cr or job called by *_to_proc
 *  will not activite in original process.
 */
int __cr_to_proc(struct context *__context, int flags);

#define cr_to_proc(flags)                              \
    do {                                               \
        if (__cr_to_proc(__context, flags) == CR_EXIT) \
            goto __cr_exit;                            \
    } while (0)

#endif /* __COROUTINE_H__ */
