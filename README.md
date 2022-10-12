# Coroutine in C

This project is the Unix style of coroutine mechanism in C language.
It uses the file descriptor number to control the coroutine system and the job function.

Building
---
To build the static library `coroutine.a`:
```shell
$ make
```

How to use
---
In your project, include the `coroutine.h` header for the API:
```cpp
#include "coroutine.h"
```

Then compile the project with `coroutine.a`. For example:
```shell
$ gcc -o main main.c coroutine.a -g
```

The routine
---
First of all, define the job function you want to work.
The variable is used in the job function which needs to be declared by `VAR_DEFINEn()` macro,
The `n` is the number of the variable you want to declare.
Also, the variable defines must happen before the `cr_begin` macro.

```cpp
COROUTINE_DEFINE(myjob)
{
    VAR_DEFINE2(int, val1, val2);
    ARRAY_DEFINE(int, arr, 20);
    cr_begin();
    cr_set(val1, 2 /* value */);
    cr_set(arr, 2 /* value */, 3 /* index */);
    cr_yield();
    int temp = cr_dref(val1);
    temp = cr_dref(arr, 3 /* index */);
    cr_end();
}
```

Second, create the coroutine with `coroutine_create()` function,
it will return the fd number of the coroutine you created.
Then, add the job function into the coroutine by the `coroutine_add` function and start working.
At least, use the `coroutine_join` to join with the terminated coroutine.

```cpp
int main(void)
{
  int crfd, tfd;
  
  crfd = coroutine_create(CR_FIFO);
  tfd = coroutine_add(crfd, myjob, NULL);
  coroutine_start(crfd);
  coroutine_join(crfd);
  
  return 0;
}
```
