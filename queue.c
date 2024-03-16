#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "sort_impl.h"
/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */
typedef void (*sort_fn)(struct list_head *head, list_cmp_func_t cmp);
struct task {
    sort_fn sort;
};
struct task current_task;
void sort_init()
{
    current_task.sort = timsort;
}

int compare(void *priv, struct list_head *q1, struct list_head *q2)
{
    if (q1 == q2)
        return 0;
    element_t *e1 = list_entry(q1, element_t, list);
    element_t *e2 = list_entry(q2, element_t, list);
    if (priv)
        *((int *) priv) += 1;
    return strcmp(e1->value, e2->value);
}

int merge_two_queues(struct list_head *q1, struct list_head *q2, bool descend);
void q_node_free(struct list_head *node)
{
    if (node) {
        element_t *tmp = list_entry(node, element_t, list);
        free(tmp->value);
        free(tmp);
    }
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (!head) {
        return NULL;
    }
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list) {
        free(entry->value);
        free(entry);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    int len = strlen(s) + 1;
    element_t *node = (element_t *) malloc(sizeof(element_t));
    if (!node)
        return false;

    node->value = malloc(len);
    if (!node->value) {
        free(node);
        return false;
    }
    strncpy(node->value, s, len);
    if (list_empty(head)) {
        list_add_tail(&node->list, head);
    } else {
        list_add(&node->list, head);
    }

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    int len = strlen(s) + 1;
    element_t *node = (element_t *) malloc(sizeof(element_t));
    if (!node)
        return false;
    node->value = malloc(len);
    if (!node->value) {
        free(node);
        return false;
    }
    strncpy(node->value, s, len);
    list_add_tail(&node->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *first_elem = list_first_entry(head, element_t, list);
    list_del(&first_elem->list);
    if (sp && first_elem->value) {
        strncpy(sp, first_elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return first_elem;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *last_elem = list_last_entry(head, element_t, list);
    list_del(&last_elem->list);
    if (sp && last_elem->value) {
        strncpy(sp, last_elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return last_elem;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int sum = 0;
    struct list_head *node;
    list_for_each (node, head) {
        sum++;
    }
    return sum;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head *slow, *fast;
    slow = fast = head->next;
    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    if (fast->next != head) {
        slow = slow->next;
    }
    list_del(slow);
    element_t *entry = container_of(slow, element_t, list);
    free(entry->value);
    free(entry);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head)) {
        return false;
    }
    element_t *entry, *safe;
    struct list_head *tmp;
    struct list_head *pending = q_new();
    list_for_each_entry_safe (entry, safe, head, list) {
        while (entry->list.next != head &&
               !strcmp(entry->value,
                       list_entry(entry->list.next, element_t, list)->value)) {
            tmp = entry->list.next;
            list_move(tmp, pending);
        }
        if (entry->list.next != &safe->list) {
            safe = list_entry(entry->list.next, element_t, list);
            list_move(&entry->list, pending);
        }
    }
    q_free(pending);
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head)) {
        return;
    }
    struct list_head *left, *right;
    list_for_each_safe (left, right, head) {
        if (right != head) {
            list_move(right, left->prev);
            right = left->next;
        }
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return;
    }
    struct list_head *last = head->prev;
    while (head->next != last) {
        list_move(head->next, last);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head)) {
        return;
    }
    struct list_head *node;
    list_for_each (node, head) {
        struct list_head *insert_point;
        insert_point = node->prev;
        for (int i = 0; i < k - 1; i++) {
            if (node->next != head) {
                list_move(node->next, insert_point);
            }
        }
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }
    sort_init();
    current_task.sort(head, compare);
    if (descend) {
        q_reverse(head);
    }
}
/* Sort elements of queue in ascending/descending order */
/*
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }
    // find mid list
    struct list_head *slow, *fast;
    slow = fast = head->next;
    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    struct list_head q2;
    list_cut_position(&q2, head, slow);
    q_sort(head, descend);
    q_sort(&q2, descend);
    merge_two_queues(head, &q2, descend);
}
*/

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head)) {
        return 0;
    }
    element_t *left, *right;
    struct list_head *pending = q_new();
    int count = 0;

    list_for_each_entry_safe (left, right, head, list) {
        count++;
        if (&right->list != head && strcmp(left->value, right->value) > 0) {
            list_move(&right->list, pending);
            right = left;
            count--;
        }
    }
    q_free(pending);
    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head)) {
        return 0;
    }
    int count = 0;
    struct list_head *node, *pending;
    node = head->prev;
    count = q_size(head);
    while (node->prev != head) {
        if (strcmp(list_entry(node, element_t, list)->value,
                   list_entry(node->prev, element_t, list)->value) > 0) {
            pending = node->prev;
            list_del(pending);
            q_node_free(pending);
            count--;
        } else {
            node = node->prev;
        }
    }
    return count;
}


int merge_two_queues(struct list_head *q1, struct list_head *q2, bool descend)
{
    element_t *entry, *safe;
    struct list_head *q1_head, *q2_head;
    q1_head = q1;
    q2_head = q2;
    int count = q_size(q1) + q_size(q2);
    if (list_empty(q2)) {
        return count;
    }
    q2 = q2->next;
    list_for_each_entry_safe (entry, safe, q1, list) {
        if (descend) {
            while (strcmp(entry->value,
                          list_entry(q2, element_t, list)->value) < 0) {
                if (q2->next == q2_head) {
                    q2 = q2->next;
                    list_move(q2->prev, entry->list.prev);
                    goto out;
                } else {
                    q2 = q2->next;
                    list_move(q2->prev, entry->list.prev);
                }
            }
        } else {
            while (strcmp(entry->value,
                          list_entry(q2, element_t, list)->value) > 0) {
                if (q2->next == q2_head) {
                    q2 = q2->next;
                    list_move(q2->prev, entry->list.prev);
                    goto out;
                } else {
                    q2 = q2->next;
                    list_move(q2->prev, entry->list.prev);
                }
            }
        }
    }
out:
    list_splice_tail_init(q2_head, q1_head);
    return count;
}
/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head)) {
        return 0;
    }
    if (list_is_singular(head)) {
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);
    }
    queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    struct list_head *entry, *safe;
    int count = 0;
    list_for_each_safe (entry, safe, head) {
        if (safe != head) {
            queue_contex_t *comp = list_entry(safe, queue_contex_t, chain);
            count = merge_two_queues(first->q, comp->q, descend);
        }
    }
    return count;
}

void q_shuffle(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }
    int size = q_size(head);
    for (struct list_head *node = head->prev; node != head && size;
         node = node->prev, size--) {
        struct list_head *it = head->next;
        // find random node
        for (int r = rand() % size; r > 0; r--) {
            it = it->next;
        }
        if (it == node) {
            continue;
        }
        struct list_head *tmp = head->prev;
        list_move(it, tmp);
    }
}
