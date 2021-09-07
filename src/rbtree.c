#include <stdlib.h>

#include "rbtree.h"

/**
 *          |                                    |
 *          y             left_rotate            x
 *         / \          <--------------         / \
 *        x   r         --------------->       a   y
 *       / \              right_rotate            / \
 *      a   b                                    b   r
 * 
 */
static inline void __rb_left_rotate(struct rb_root *root, struct rb_node *x)
{
    struct rb_node *y = x->rightC;
    x->rightC = y->leftC;
    if (y->leftC != &root->nil)
        rb_set_parent(y->leftC, x);
    rb_set_parent(y, rb_parent(x));
    if (rb_parent(x) == &root->nil)
        root->head = y;
    else if (x == rb_parent(x)->leftC)
        rb_parent(x)->leftC = y;
    else
        rb_parent(x)->rightC = y;
    y->leftC = x;
    rb_set_parent(x, y);
}

static inline void __rb_right_rotate(struct rb_root *root, struct rb_node *y)
{
    struct rb_node *x = y->leftC;
    y->leftC = x->rightC;
    if (x->rightC != &root->nil)
        rb_set_parent(x->rightC, y);
    rb_set_parent(x, rb_parent(y));
    if (rb_parent(y) == &root->nil)
        root->head = x;
    else if (y == rb_parent(y)->rightC)
        rb_parent(y)->rightC = x;
    else
        rb_parent(y)->leftC = x;
    x->rightC = y;
    rb_set_parent(y, x);
}

static void __rb_insert_fixup(struct rb_root *root, struct rb_node *node)
{
    while (rb_color(rb_parent(node)) == RED) {
        if (rb_parent(node) == rb_parent(rb_parent(node))->leftC) {
            struct rb_node *y = rb_parent(rb_parent(node))->rightC;
            if (rb_color(y) == RED) {
                rb_set_black(rb_parent(node));
                rb_set_black(y);
                rb_set_red(rb_parent(rb_parent(node)));
                node = rb_parent(rb_parent(node));
            } else {
                if (node == rb_parent(node)->rightC) {
                    node = rb_parent(node);
                    __rb_left_rotate(root, node);
                }
                rb_set_black(rb_parent(node));
                rb_set_red(rb_parent(rb_parent(node)));
                __rb_right_rotate(root, rb_parent(rb_parent(node)));
            }
        } else {
            struct rb_node *y = rb_parent(rb_parent(node))->leftC;
            if (rb_color(y) == RED) {
                rb_set_black(rb_parent(node));
                rb_set_black(y);
                rb_set_red(rb_parent(rb_parent(node)));
                node = rb_parent(rb_parent(node));
            } else {
                if (node == rb_parent(node)->leftC) {
                    node = rb_parent(node);
                    __rb_right_rotate(root, node);
                }
                rb_set_black(rb_parent(node));
                rb_set_red(rb_parent(rb_parent(node)));
                __rb_left_rotate(root, rb_parent(rb_parent(node)));
            }
        }
    }
    rb_set_black(root->head);
}

void rbtree_insert(struct rb_root *root, struct rb_node *node,
                   int (*cmp)(struct rb_node *, struct rb_node *))
{
    struct rb_node *y = &root->nil;
    struct rb_node *x = root->head;

    while (x != &root->nil) {
        y = x;
        // node < x
        if (cmp(node, x))
            x = x->leftC;
        else
            x = x->rightC;
    }
    rb_set_parent(node, y);
    if (y == &root->nil)
        root->head = node;
    // node < y
    else if (cmp(node, y))
        y->leftC = node;
    else
        y->rightC = node;

    node->rightC = &root->nil;
    node->leftC = &root->nil;
    rb_set_red(node);
    __rb_insert_fixup(root, node);

    root->cnt++;
}

static inline void __rbtree_clean(struct rb_root *root, struct rb_node *node,
                                  void (*freefunc)(struct rb_node *))
{
    if (node != &root->nil) {
        struct rb_node *left = node->leftC;
        struct rb_node *right = node->rightC;
        freefunc(node);
        __rbtree_clean(root, left, freefunc);
        __rbtree_clean(root, right, freefunc);
    }
}

void rbtree_clean(struct rb_root *root, void (*freefunc)(struct rb_node *))
{
    __rbtree_clean(root, root->head, freefunc);
}

struct rb_node *rbtree_search(struct rb_root *root, void *key,
                              int (*cmp)(struct rb_node *, void *))
{
    struct rb_node *temp = root->head;

    while (temp != &root->nil) {
        switch (cmp(temp, key)) {
        case RB_EQUAL:
            goto done;
        case RB_LEFT:
            temp = temp->leftC;
            break;
        case RB_RIGHT:
            temp = temp->rightC;
            break;
        case RB_EQUAL_BREAK:
            goto out;
        }
    }
    return NULL;
out:
    return &root->nil;
done:
    return temp;
}

static void __rb_delete_fixup(struct rb_root *root, struct rb_node *node)
{
    struct rb_node *w;

    while (node != root->head && rb_is_black(node)) {
        if (node == rb_parent(node)->leftC) {
            w = rb_parent(node)->rightC;
            if (rb_is_red(w)) {
                rb_set_black(w);
                rb_set_red(rb_parent(node));
                __rb_left_rotate(root, rb_parent(node));
                w = rb_parent(node)->rightC;
            }
            if (rb_is_black(w->leftC) && rb_is_black(w->rightC)) {
                rb_set_red(w);
                node = rb_parent(node);
            } else {
                if (rb_is_black(w->rightC)) {
                    rb_set_black(w->leftC);
                    rb_set_red(w);
                    __rb_right_rotate(root, w);
                    w = rb_parent(node)->rightC;
                }
                rb_set_color(w, rb_parent(node));
                rb_set_black(rb_parent(node));
                rb_set_black(w->rightC);
                __rb_left_rotate(root, rb_parent(node));
                node = root->head;
            }
        } else {
            w = rb_parent(node)->leftC;
            if (rb_is_red(w)) {
                rb_set_black(w);
                rb_set_red(rb_parent(node));
                __rb_right_rotate(root, rb_parent(node));
                w = rb_parent(node)->leftC;
            }
            if (rb_is_black(w->rightC) && rb_is_black(w->leftC)) {
                rb_set_red(w);
                node = rb_parent(node);
            } else {
                if (rb_is_black(w->leftC)) {
                    rb_set_black(w->rightC);
                    rb_set_red(w);
                    __rb_left_rotate(root, w);
                    w = rb_parent(node)->leftC;
                }
                rb_set_color(w, rb_parent(node));
                rb_set_black(rb_parent(node));
                rb_set_black(w->leftC);
                __rb_right_rotate(root, rb_parent(node));
                node = root->head;
            }
        }
    }
    rb_set_black(node);
}

static inline void __rbtree_transplant(struct rb_root *root, struct rb_node *u,
                                       struct rb_node *v)
{
    if (rb_parent(u) == &root->nil)
        root->head = v;
    else if (u == rb_parent(u)->leftC)
        rb_parent(u)->leftC = v;
    else
        rb_parent(u)->rightC = v;
    rb_set_parent(v, rb_parent(u));
}

#include <stdbool.h>

void __rbtree_delete(struct rb_root *root, struct rb_node *node)
{
    struct rb_node *y = node;
    bool yoc = rb_color(y);
    struct rb_node *x;

    if (node->leftC == &root->nil) {
        x = node->rightC;
        __rbtree_transplant(root, node, node->rightC);
    } else if (node->rightC == &root->nil) {
        x = node->leftC;
        __rbtree_transplant(root, node, node->leftC);
    } else {
        y = __rbtree_min(root, node->rightC);
        yoc = rb_color(y);
        x = y->rightC;
        if (rb_parent(y) == node)
            rb_set_parent(x, y);
        else {
            __rbtree_transplant(root, y, y->rightC);
            y->rightC = node->rightC;
            rb_set_parent(y->rightC, y);
        }
        __rbtree_transplant(root, node, y);
        y->leftC = node->leftC;
        rb_set_parent(y->leftC, y);
        rb_set_color(y, node);
    }
    if (yoc == BLACK)
        __rb_delete_fixup(root, x);
}

int rbtree_delete(struct rb_root *root, void *key,
                  int (*cmp)(struct rb_node *, void *),
                  void (*deletefunc)(struct rb_node *))
{
    struct rb_node *z = rbtree_search(root, key, cmp);
    if (z == NULL)
        return 1;
    __rbtree_delete(root, z);
    deletefunc(z);
    root->cnt--;
    return 0;
}
