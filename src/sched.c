#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "context.h"
#include "coroutine.h"
#include "coroutine_int.h"

int schedule(struct cr *cr, job_t func, void *args)
{
    struct task_struct *new_task;

    new_task = calloc(1, sizeof(struct task_struct));
    if (!new_task)
        return -ENOMEM;
    if (rq_enqueue(&cr->rq, new_task) < 0) {
        free(new_task);
        return -ENOMEM;
    }

    new_task->cr = cr;
    new_task->tfd = cr->size++;
    new_task->job = func;
    new_task->args = args;
    new_task->context.label = NULL;
    new_task->context.wait_yield = 1;
    new_task->context.blocked = 1;

    return new_task->tfd;
}

/* The clone system call will call this function to let the
 * job being forked from the original struct cr.
 * Becarful about the context->blocked. If blocked set 1 is
 * int struct cr and all the coroutine operations are work;
 * if set 0, then the original job in the struct cr is blocked,
 * when the job scheduled is will immediately release the resource;
 * if set < 0, then it in this function.
 */
static int clone_handler(void *args)
{
    struct task_struct *task = (struct task_struct *)args;
    task->context.blocked = -1;
    // TODO call the job
    return 0;
}

/* FIFO scheduler
 */
static inline struct task_struct *fifo_pick_next_task(struct cr *cr)
{
    return rq_dequeue(&cr->rq);
}

static inline int fifo_put_prev_task(struct cr *cr, struct task_struct *prev)
{
    return rq_enqueue(&cr->rq, prev);
}

static inline int fifo_job_to_proc(struct cr *cr, struct task_struct *p)
{
    for (unsigned int i = cr->rq.in; i != cr->rq.out; i++)
        if (cr->rq.r[i & cr->rq.mask] == p) {
            p->context.blocked = 0;
            // TODO establish the clone function
            return 0;
        }

    return -EAGAIN;
}

// TODO default scheduler
void sched_init(struct cr *cr)
{
    switch (cr->flag) {
    case CR_FIFO:
        rq_init(&cr->rq);
        cr->pick_next_task = fifo_pick_next_task;
        cr->put_prev_task = fifo_put_prev_task;
        cr->job_to_proc = fifo_job_to_proc;
    }
}
