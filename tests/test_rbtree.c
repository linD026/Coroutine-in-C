#include <stdio.h>
#include <stdlib.h>
#include "../src/rbtree.h"

struct test_node {
    int value;
    struct rb_node node;
};

RBTREE_CMP_INSERT_DEFINE(cmp_insert, __n1, __n2)
{
    struct test_node *n1 = container_of(__n1, struct test_node, node);
    struct test_node *n2 = container_of(__n2, struct test_node, node);

    if (n1->value < n2->value)
        return 1;
    else
        return 0;
}

RBTREE_CMP_SEARCH_DEFINE(cmp_search, __n, __d)
{
    struct test_node *n = container_of(__n, struct test_node, node);
    int *d = (int *)__d;

    if (n->value == *d)
        return RB_EQUAL;
    else if (n->value > *d)
        return RB_LEFT;
    else if (n->value < *d)
        return RB_RIGHT;
    else
        return RB_EQUAL_BREAK;
}

RBTREE_DELETE_DEFINE(delete, __n)
{
    struct test_node *n = container_of(__n, struct test_node, node);
    free(n);
}

#include <assert.h>

int main(void)
{
    struct rb_root root;
    struct test_node *tmp;

    RB_ROOT_INIT(root);
    for (int i = 0; i < 100000; i++) {
        tmp = malloc(sizeof(struct test_node));
        tmp->value = i;
        rbtree_insert(&root, &tmp->node, cmp_insert);
    }

    printf("inserted %ld\n", root.cnt);

    for (int i = 0; i < 100000; i++) {
        struct rb_node *node = rbtree_search(&root, &i, cmp_search);
        if (node == NULL || node == &root.nil) {
            printf("%d not found\n", i);
            assert(0);
        }
    }

    for (int i = 330; i < 7042; i++) {
        int ret = rbtree_delete(&root, &i, cmp_search, delete);
        assert(ret == 0);
    }

    printf("deleted\n");

    int i = 86503;
    struct rb_node *n = rbtree_search(&root, &i, cmp_search);
    if (n == NULL)
        printf("NO %d value\n", i);
    tmp = container_of(n, struct test_node, node);
    printf("search %d\n", tmp->value);
    printf("prev %d\n",
           container_of(rbtree_prev(&root, n), struct test_node, node)->value);
    printf("next %d\n",
           container_of(rbtree_next(&root, n), struct test_node, node)->value);

    rbtree_clean(&root, delete);
}
