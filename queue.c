#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


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
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
