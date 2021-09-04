#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include "coroutine.h"
#include "coroutine_int.h"

static struct cr_struct crt = { 0 };

int coroutine_create(int flag)
{
    int ret = -ENOMEM;

    if (!crt.table[crt.size]) {
        crt.table[crt.size] = calloc(1, sizeof(struct cr));
        if (!crt.table[crt.size])
            return -ENOMEM;
        crt.table[crt.size]->crfd = crt.size;
        crt.table[crt.size]->flag = flag; // set the sched flag
        sched_init(crt.table[crt.size]);
        ret = crt.size++;
    }

    return ret;
}

/*
 * Return the task fd
 */
int coroutine_add(int crfd, job_t func, void *args)
{
    if (crt.table[crfd] == NULL && func)
        return -EAGAIN;

    return schedule(crt.table[crfd], func, args);
}

/*
 * When cr is in the CR_NORMAL it only work on main thread.
 * It need to send the address of context to job function.
 * The function type is "void name(struct context *__context, void *args)"
 */
int coroutine_start(int crfd)
{
    struct cr *cr;
    int status;

    if (!crt.table[crfd])
        return -EAGAIN;
    cr = crt.table[crfd];

    do {
        cr->current = cr->pick_next_task(cr);

        if (!cr->current)
            goto done;
        status = cr->current->job(&(cr->current->context), cr->current->args);

        switch (status) {
        case CR_WAIT:
            break;
        case CR_YIELD:
            cr->current->context.wait_yield = 1;
            cr->put_prev_task(cr, cr->current);
            break;
        case CR_EXIT:
            free(cr->current);
            break;
        }

    } while (1);

done:
    /* all the jobs are done */
    return 0;
}

int coroutine_join(int crfd)
{
    if (!crt.table[crfd])
        return -EAGAIN;

    free(crt.table[crfd]);
    memmove(&crt.table[crfd], &crt.table[--crt.size], sizeof(void *));
    return 0;
}

// TODO, see coroutine.h comments
int __cr_to_proc(struct context *context, int flag)
{
    struct task_struct *task = task_of(context);
    struct cr *cr = task->cr;

    return cr->job_to_proc(cr, task);
}

