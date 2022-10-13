#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "context.h"
#include "coroutine.h"
#include "coroutine_int.h"

static struct cr_struct crt = { 0 };

int coroutine_create(int flags)
{
    int ret = -ENOMEM;

    if (!(flags & CR_SCHED_MASK))
        return -EFAULT;

    if (crt.size >= MAX_CR_TABLE_SIZE)
        return -ENOMEM;

    for (int i = 0; i < MAX_CR_TABLE_SIZE; i++) {
        if (!crt.table[i]) {
            crt.table[i] = calloc(1, sizeof(struct cr));
            if (!crt.table[i])
                return ret;
            crt.table[i]->crfd = i;
            crt.table[i]->flags = flags;
            sched_init(crt.table[i]);
            crt.size++;
            ret = i;
            break;
        }
    }

    return ret;
}

/*
 * Return the task fd
 */
int coroutine_add(int crfd, job_t func, void *args)
{
    if (!crt.table[crfd] && func)
        return -EFAULT;

    return crt.table[crfd]->schedule(crt.table[crfd], func, args);
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
        return -EFAULT;
    cr = crt.table[crfd];

    do {
        cr->current = cr->pick_next_task(cr);

        if (!cr->current)
            goto done;
        status = cr->current->job(&(cr->current->context), cr->current->args);

        switch (status) {
        case CR_CLONE_EXIT:
            free(cr->current);
            while ((cr->current = cr->pick_next_task(cr))) {
                if (!cr->current)
                    goto done;
                for (int i = 0; i < cr->current->context.local_size; i++)
                    free(cr->current->context.local[i]);
                free(cr->current);
            }
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
        return -EFAULT;

    free(crt.table[crfd]);
    crt.table[crfd] = NULL;
    crt.size--;
    return 0;
}

/* The clone system call will call this function to let the
 * job being forked from the original struct cr.
 * Becarful about the context->blocked. If blocked set 1 is
 * int struct cr and all the coroutine operations are work;
 * if set 0, then the original job in the struct cr is blocked,
 * when the job scheduled is will immediately release the resource;
 * if set < 0, then it in this function.
 */
int __cr_to_proc(struct context *context, int flags)
{
    struct task_struct *task = task_of(context);
    struct cr *cr = task->cr;

    task->context.blocked = 0;
    switch (fork()) {
    case -1: /* failed */
        return -EAGAIN;
    case 0: /* child */
        /* All the tasks will release after forked job finish. */
        task->context.blocked = -1;
        return CR_CLONE_EXIT;
    default: /* parent */
        return CR_EXIT;
    }
}
