#include <stdio.h>
#include "../src/coroutine_int.h"

int main(void)
{
    struct task_struct task[10];
    struct rq rq;
    struct task_struct *tmp;

    rq_init(&rq);
    for (int i = 0; i < 10; i++) {
        task[i].tfd = i;
        printf("enqueue %d, return %d\n", i, rq_enqueue(&rq, &task[i]));
    }

    for (int i = 0; i < 10; i++) {
        tmp = rq_dequeue(&rq);
        if (tmp)
            printf("dequeue %d\n", tmp->tfd);
        else
            printf("dequeue failed\n");
    }

    return 0;
}