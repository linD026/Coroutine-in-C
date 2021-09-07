#include <errno.h>
#include <stdlib.h>
#include "coroutine_int.h"

void rq_init(struct rq *rq)
{
    // setup the ring buffer
    rq->out = 0;
    rq->in = 0;
    rq->mask = RINGBUFFER_SIZE - 1;
}

static inline unsigned int __ringbuffer_unused(struct rq *rq)
{
    return rq->mask + 1 - (rq->in - rq->out);
}

// enqueue at in
int rq_enqueue(struct rq *rq, struct task_struct *task)
{
    if (!__ringbuffer_unused(rq))
        return -EAGAIN;

    rq->r[rq->in & rq->mask] = task;
    rq->in++;

    return 0;
}

// dequeue at out
struct task_struct *rq_dequeue(struct rq *rq)
{
    struct task_struct *rev;

    if (rq->in == rq->out)
        return NULL;

    rev = rq->r[rq->out & rq->mask];
    rq->out++;

    return rev;
}
