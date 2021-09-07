#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define LOCAL_SIZE 16

struct context {
    unsigned int local_size;
    unsigned int local_offset;
    void *local[LOCAL_SIZE];
    void *label;
    int wait_yield; /* set 0 if waiting for someone or yield */
    int blocked; /* set 0 if cr_to_proce called, this will
                     let the original task which in the
                     struct cr being blocked; set 1 for default.
                     set < 0 for the clone.*/
};

#endif /* __CONTEXT_H__ */
