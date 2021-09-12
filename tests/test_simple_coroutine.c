#include <stdio.h>
#include "../src/coroutine.h"

jmp_buf m;

int job1(volatile struct context *__context, volatile int i)
{
    printf("[job 1 - 1] %d\n", i++);
    cr_yield();
    printf("[job 1 - 2] %d\n", i++);

    longjmp(m, 1);
}

int job2(volatile struct context *__context, volatile int i)
{
    printf("[job 2 - 1] %d\n", i++);
    cr_yield();
    printf("[job 2 - 1] %d\n", i++);

    longjmp(m, 2);
}

void test_context_swith(void)
{
    struct context j1;
    struct context j2;

    job1(&j1, 0);
    job2(&j2, 0);

    switch (setjmp(m)) {
    case 0:
        longjmp(j1.context, CR_RETURN_JOB);
    case 1:
        longjmp(j2.context, CR_RETURN_JOB);
    }

    printf("exit\n");
}

int main(void)
{
    struct context j1;
    j1.local_size = 0;
    struct context *__context = (unsigned long)&j1;
    VAR_DEFINE(int, val1);
    val1 = 1;
    VAR_SAVE(int, val1);
    __coro_val1 = 3;
    VAR_STORE(int, val1);

    return 0;
}