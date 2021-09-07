#include <stdio.h>
#include "../coroutine.h"

COROUTINE_DEFINE(job)
{
    VAR_DEFINE2(int, i, j);
    VAR_DEFINE3(double, k, l, m);
    cr_begin();
    *i = 1;
    *j = 2;
    *k = 2.2;
    printf("[@ job %d] %d %d\n", *(int *)args, *i, *j);

    cr_yield();

    printf("[# job %d] %d %d\n", *(int *)args, *i, *j);
    if (*k == 2.2)
        printf("variable success\n");

    cr_end();
}


int main(void)
{
    int crfd, tfd[10];

    crfd = coroutine_create(CR_DEFAULT);

    for (int i = 0;i < 10;i++) {
        tfd[i] = i;
        printf("[%d tfd] %d added, %d\n", coroutine_add(crfd, job, &tfd[i]), i, tfd[i]);
    }

    coroutine_start(crfd);

    coroutine_join(crfd);
    return 0;
}
