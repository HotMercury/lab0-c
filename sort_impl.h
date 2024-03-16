#ifndef SORT_IMPL_H
#define SORT_IMPL_H
#include <stdbool.h>
#include "list.h"

typedef int (*list_cmp_func_t)(void *priv,
                               struct list_head *,
                               struct list_head *);
void timsort(struct list_head *, list_cmp_func_t);
void __timsort(void *, struct list_head *, list_cmp_func_t);
int compare(void *priv, struct list_head *, struct list_head *);
#endif  // SORT_IMPL.h