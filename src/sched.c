#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sched.h>

#include "rbtree.h"
#include "context.h"
#include "coroutine.h"
#include "coroutine_int.h"

/* FIFO scheduler */

static inline int fifo_schedule(struct cr *cr, job_t func, void *args)
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

static inline struct task_struct *fifo_pick_next_task(struct cr *cr)
{
    return rq_dequeue(&cr->rq);
}

static inline int fifo_put_prev_task(struct cr *cr, struct task_struct *prev)
{
    return rq_enqueue(&cr->rq, prev);
}

/* Default scheduler */

static RBTREE_CMP_INSERT_DEFINE(rb_cmp_insert, _n1, _n2)
{
    struct task_struct *n1 = container_of(_n1, struct task_struct, node);
    struct task_struct *n2 = container_of(_n2, struct task_struct, node);
    if (n1->sum_exec_runtime < n2->sum_exec_runtime)
        return 1;
    else {
        if (n1->sum_exec_runtime == n2->sum_exec_runtime)
            n1->sum_exec_runtime++;
        return 0;
    }
}

static RBTREE_CMP_SEARCH_DEFINE(rb_cmp_search, _n1, key)
{
    struct task_struct *n1 = container_of(_n1, struct task_struct, node);

    if (n1->sum_exec_runtime == *(long *)key)
        return RB_EQUAL;
    else if (n1->sum_exec_runtime > *(long *)key)
        return RB_RIGHT;
    else
        return RB_LEFT;
}

#define time_diff(start, end) \
    (end - start < 0 ? (1000000000 + end - start) : (end - start))

static inline int default_schedule(struct cr *cr, job_t func, void *args)
{
    struct task_struct *new_task;
    static long exec_base = 0;

    new_task = calloc(1, sizeof(struct task_struct));
    if (!new_task)
        return -ENOMEM;

    new_task->sum_exec_runtime = exec_base;
    rbtree_insert(&cr->root, &new_task->node, rb_cmp_insert);

    new_task->cr = cr;
    new_task->tfd = cr->size++;
    new_task->job = func;
    new_task->args = args;
    new_task->context.label = NULL;
    new_task->context.wait_yield = 1;
    new_task->context.blocked = 1;

    return new_task->tfd;
}

static inline struct task_struct *default_pick_next_task(struct cr *cr)
{
    struct rb_node *node = rbtree_min(&cr->root);
    struct task_struct *task = container_of(node, struct task_struct, node);
    struct timespec start;

    if (node == NULL)
        return NULL;
    __rbtree_delete(&cr->root, node);
    clock_gettime(CLOCK_MONOTONIC, &start);
    task->exec_start = start.tv_nsec;

    return task;
}

static inline int default_put_prev_task(struct cr *cr, struct task_struct *prev)
{
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &end);
    prev->sum_exec_runtime += time_diff(prev->exec_start, end.tv_nsec);
    rbtree_insert(&cr->root, &prev->node, rb_cmp_insert);

    return 0;
}

void sched_init(struct cr *cr)
{
    switch (cr->flags) {
    case CR_DEFAULT:
        RB_ROOT_INIT(cr->root);
        cr->schedule = default_schedule;
        cr->pick_next_task = default_pick_next_task;
        cr->put_prev_task = default_put_prev_task;
        return;
    case CR_FIFO:
        rq_init(&cr->rq);
        cr->schedule = fifo_schedule;
        cr->pick_next_task = fifo_pick_next_task;
        cr->put_prev_task = fifo_put_prev_task;
    }
}
