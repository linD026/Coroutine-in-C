#include <stdio.h>
#include <unistd.h>
#include "../coroutine.h"

#define UNUSED 0

COROUTINE_DEFINE(job)
{
    VAR_DEFINE2(int, i, j);
    VAR_DEFINE3(double, k, l, m);
    ARRAY_DEFINE(int, arr, 20);
    cr_begin();
    cr_set(i, 1);
    cr_set(j, 2);
    cr_set(k, 2.2);
    cr_set(arr, 2, 4/* index */);
    printf("[@ job %d] %d %d\n", *(int *)args, cr_dref(i), cr_dref(j));

    cr_to_proc(UNUSED);
    printf("pid %d\n", getpid());
    
    cr_yield();

    cr_set(i, cr_dref(i) + 1);
    if (cr_dref(arr, 4/* index */) == 2)
        printf("array success\n");
    printf("[# job %d] %d %d\n", *(int *)args, cr_dref(i), cr_dref(j));
    if (cr_dref(k) == 2.2)
        printf("variable success\n");

    cr_end();
}

int main(void)
{
    int crfd, tfd[10];

    crfd = coroutine_create(CR_DEFAULT);

    for (int i = 0; i < 10; i++) {
        tfd[i] = i;
        printf("[%d tfd] %d added, %d\n", coroutine_add(crfd, job, &tfd[i]), i,
               tfd[i]);
    }

    coroutine_start(crfd);

    coroutine_join(crfd);
    return 0;
}
