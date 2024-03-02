#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

void move_behind(struct list_head *node, struct list_head *front)
{
    if (!node || !front)
        return;

    list_del_init(node);

    node->next = front->next;
    node->prev = front;
    node->next->prev = node;
    node->prev->next = node;
}

void move_front(struct list_head *node, struct list_head *behind)
{
    if (!node || !behind)
        return;

    list_del_init(node);

    node->next = behind;
    node->prev = behind->prev;
    behind->prev = node;
    node->prev->next = node;
}

void add_behind_all_init(struct list_head *list, struct list_head *front)
{
    if (!list || !front)
        return;

    list->prev->next = front->next;
    list->next->prev = front;
    front->next->prev = list->prev;
    front->next = list->next;

    INIT_LIST_HEAD(list);
}

bool entry_is_greater(element_t *node1, element_t *node2)
{
    if (strcmp(node1->value, node2->value) > 0)
        return true;

    return false;
}

bool list_is_greater(struct list_head *node1, struct list_head *node2)
{
    element_t *element1 = container_of(node1, element_t, list),
              *element2 = container_of(node2, element_t, list);

    if (strcmp(element1->value, element2->value) > 0)
        return true;

    return false;
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *queue_head = malloc(sizeof(struct list_head));
    if (!queue_head) {
        free(queue_head);
        return NULL;
    }

    INIT_LIST_HEAD(queue_head);

    return queue_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    struct list_head *node = NULL, *safe = NULL;
    list_for_each_safe (node, safe, l) {
        q_release_element(container_of(node, element_t, list));
    }

    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element) {
        free(new_element);
        return false;
    }

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add(&new_element->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element) {
        free(new_element);
        return false;
    }

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add_tail(&new_element->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *remove_element = list_first_entry(head, element_t, list);
    list_del(&remove_element->list);

    if (sp != NULL) {
        strncpy(sp, remove_element->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';
    }

    return remove_element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *remove_element = list_last_entry(head, element_t, list);
    list_del(&remove_element->list);

    if (sp != NULL) {
        strncpy(sp, remove_element->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';
    }

    return remove_element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    int length = 0;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        length++;
    }

    return length;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *first = head->next, *tail = head->prev;
    while ((first != tail) && (tail != first->prev)) {
        first = first->next;
        tail = tail->prev;
    }

    element_t *middle = container_of(first, element_t, list);
    list_del(first);
    q_release_element(middle);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    bool dup = false;
    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, head, list) {
        if ((&node->list != head && !strcmp(node->value, safe->value)) || dup) {
            list_del(&node->list);
            q_release_element(node);
        }

        dup = (&node->list != head && !strcmp(node->value, safe->value));
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *first = head->next, *second = head->next->next;
    for (; first != head && second != head;
         first = first->next->next, second = second->next->next) {
        first->prev->next = second;
        second->prev = first->prev;

        second->next->prev = first;
        first->next = second->next;

        first->prev = second;
        second->next = first;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *first = head->next, *tail = head->prev, *cut_f, *cut_t;
    while ((first != tail) && (tail != first->prev)) {
        cut_f = first;
        cut_t = tail;
        first = first->prev;
        tail = tail->next;

        move_behind(cut_t, first);
        move_front(cut_f, tail);

        first = cut_t->next;
        tail = cut_f->prev;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k < 2)
        return;

    struct list_head *temp_head = malloc(sizeof(struct list_head));

    int count_k = 0;
    struct list_head *cur = head->next, *first = cur, *cut_p, *cut_next, *tail;
    while (cur != head) {
        if (count_k == k) {
            tail = cur;
            count_k = 0;

            cut_p = first;
            cut_next = first;
            first = first->prev;
            while (cut_p != tail) {
                cut_next = cut_next->next;

                list_move(cut_p, temp_head);
                cut_p = cut_next;
            }

            add_behind_all_init(temp_head, first);
            first = tail;
        }

        count_k++;
        cur = cur->next;
    }

    free(temp_head);
}

void sort_descend(struct list_head *head)
{
    if (list_empty(head) || list_is_singular(head))
        return;

    struct list_head list_left, list_right;
    INIT_LIST_HEAD(&list_left);
    INIT_LIST_HEAD(&list_right);

    element_t *pivot = list_first_entry(head, element_t, list);
    list_del(&pivot->list);

    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, head, list) {
        if (entry_is_greater(node, pivot))
            list_move(&node->list, &list_left);
        else
            list_move(&node->list, &list_right);
    }

    sort_descend(&list_left);
    sort_descend(&list_right);

    list_add(&pivot->list, head);
    list_splice(&list_left, head);
    list_splice_tail(&list_right, head);
}

void sort_ascend(struct list_head *head)
{
    if (list_empty(head) || list_is_singular(head))
        return;

    struct list_head list_left, list_right;
    INIT_LIST_HEAD(&list_left);
    INIT_LIST_HEAD(&list_right);

    element_t *pivot = list_first_entry(head, element_t, list);
    list_del(&pivot->list);

    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, head, list) {
        if (!entry_is_greater(node, pivot))
            list_move(&node->list, &list_left);
        else
            list_move(&node->list, &list_right);
    }

    sort_ascend(&list_left);
    sort_ascend(&list_right);

    list_add(&pivot->list, head);
    list_splice(&list_left, head);
    list_splice_tail(&list_right, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;

    if (descend)
        sort_descend(head);
    else
        sort_ascend(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    int count = 1;
    struct list_head *pivot = head->prev, *node = pivot->prev, *remove_node;
    while (node != head) {
        if (!list_is_greater(pivot, node)) {
            remove_node = node;
            node = node->prev;

            list_del(remove_node);
            q_release_element(container_of(remove_node, element_t, list));
        } else {
            count++;
            pivot = node;
            node = node->prev;
        }
    }

    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    int count = 1;
    struct list_head *pivot = head->prev, *node = pivot->prev, *remove_node;
    while (node != head) {
        if (list_is_greater(pivot, node)) {
            remove_node = node;
            node = node->prev;

            list_del(remove_node);
            q_release_element(container_of(remove_node, element_t, list));
        } else {
            count++;
            pivot = node;
            node = node->prev;
        }
    }

    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}