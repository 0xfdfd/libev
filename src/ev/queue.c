#include <stdlib.h>

#define EV_QUEUE_NEXT(node)         ((node)->p_next)
#define EV_QUEUE_PREV(node)         ((node)->p_prev)
#define EV_QUEUE_PREV_NEXT(node)    (EV_QUEUE_NEXT(EV_QUEUE_PREV(node)))
#define EV_QUEUE_NEXT_PREV(node)    (EV_QUEUE_PREV(EV_QUEUE_NEXT(node)))

void ev_queue_init(ev_queue_node_t* head)
{
    EV_QUEUE_NEXT(head) = head;
    EV_QUEUE_PREV(head) = head;
}

void ev_queue_push_back(ev_queue_node_t* head, ev_queue_node_t* node)
{
    EV_QUEUE_NEXT(node) = head;
    EV_QUEUE_PREV(node) = EV_QUEUE_PREV(head);
    EV_QUEUE_PREV_NEXT(node) = node;
    EV_QUEUE_PREV(head) = node;
}

void ev_queue_push_front(ev_queue_node_t* head, ev_queue_node_t* node)
{
    EV_QUEUE_NEXT(node) = EV_QUEUE_NEXT(head);
    EV_QUEUE_PREV(node) = head;
    EV_QUEUE_NEXT_PREV(node) = node;
    EV_QUEUE_NEXT(head) = node;
}

void ev_queue_erase(ev_queue_node_t* node)
{
    EV_QUEUE_PREV_NEXT(node) = EV_QUEUE_NEXT(node);
    EV_QUEUE_NEXT_PREV(node) = EV_QUEUE_PREV(node);
}

ev_queue_node_t* ev_queue_pop_front(ev_queue_node_t* head)
{
    ev_queue_node_t* node = ev_queue_head(head);
    if (node == NULL)
    {
        return NULL;
    }

    ev_queue_erase(node);
    return node;
}

ev_queue_node_t* ev_queue_pop_back(ev_queue_node_t* head)
{
    ev_queue_node_t* node = EV_QUEUE_PREV(head);
    if (node == head)
    {
        return NULL;
    }

    ev_queue_erase(node);
    return node;
}

ev_queue_node_t* ev_queue_head(ev_queue_node_t* head)
{
    ev_queue_node_t* node = EV_QUEUE_NEXT(head);
    return node == head ? NULL : node;
}

ev_queue_node_t* ev_queue_next(ev_queue_node_t* head, ev_queue_node_t* node)
{
    ev_queue_node_t* next = EV_QUEUE_NEXT(node);
    return next == head ? NULL : next;
}

int ev_queue_empty(const ev_queue_node_t* node)
{
    return EV_QUEUE_NEXT(node) == node;
}
