#include "ev/map.h"

void ev_map_init(ev_map_t* handler, ev_map_cmp_fn cmp, void* arg)
{
	handler->map_low = (ev_map_low_t)EV_MAP_LOW_INIT;
	handler->cmp.cmp = cmp;
	handler->cmp.arg = arg;
	handler->size = 0;
}

int ev_map_insert(ev_map_t* handler, ev_map_node_t* node)
{
	ev_map_low_node_t **new_node = &(handler->map_low.rb_root), *parent = NULL;

	/* Figure out where to put new node */
	while (*new_node)
	{
		int result = handler->cmp.cmp(node, *new_node, handler->cmp.arg);

		parent = *new_node;
		if (result < 0)
		{
			new_node = &((*new_node)->rb_left);
		}
		else if (result > 0)
		{
			new_node = &((*new_node)->rb_right);
		}
		else
		{
			return -1;
		}
	}

	handler->size++;
	ev_map_low_link_node(node, parent, new_node);
	ev_map_low_insert_color(node, &handler->map_low);

	return 0;
}

void ev_map_erase(ev_map_t* handler, ev_map_node_t* node)
{
	handler->size--;
	ev_map_low_erase(&handler->map_low, node);
}

size_t ev_map_size(const ev_map_t* handler)
{
	return handler->size;
}

ev_map_node_t* ev_map_find(const ev_map_t* handler, const ev_map_node_t* key)
{
	ev_map_low_node_t* node = handler->map_low.rb_root;

	while (node)
	{
		int result = handler->cmp.cmp(key, node, handler->cmp.arg);

		if (result < 0)
		{
			node = node->rb_left;
		}
		else if (result > 0)
		{
			node = node->rb_right;
		}
		else
		{
			return node;
		}
	}

	return NULL;
}

ev_map_node_t* ev_map_find_lower(const ev_map_t* handler, const ev_map_node_t* key)
{
	ev_map_node_t* lower_node = NULL;
	ev_map_node_t* node = handler->map_low.rb_root;
	while (node)
	{
		int result = handler->cmp.cmp(key, node, handler->cmp.arg);
		if (result < 0)
		{
			node = node->rb_left;
		}
		else if (result > 0)
		{
			lower_node = node;
			node = node->rb_right;
		}
		else
		{
			return node;
		}
	}

	return lower_node;
}

ev_map_node_t* ev_map_find_upper(const ev_map_t* handler, const ev_map_node_t* key)
{
	ev_map_node_t* upper_node = NULL;
	ev_map_node_t* node = handler->map_low.rb_root;

	while (node)
	{
		int result = handler->cmp.cmp(key, node, handler->cmp.arg);

		if (result < 0)
		{
			upper_node = node;
			node = node->rb_left;
		}
		else if (result > 0)
		{
			node = node->rb_right;
		}
		else
		{
			if (upper_node == NULL)
			{
				upper_node = node->rb_right;
			}
			break;
		}
	}

	return upper_node;
}

ev_map_node_t* ev_map_begin(const ev_map_t* handler)
{
	return ev_map_low_first(&handler->map_low);
}

ev_map_node_t* ev_map_end(const ev_map_t* handler)
{
	return ev_map_low_last(&handler->map_low);
}

ev_map_node_t* ev_map_next(const ev_map_node_t* node)
{
	return ev_map_low_next(node);
}

ev_map_node_t* ev_map_prev(const ev_map_node_t* node)
{
	return ev_map_low_prev(node);
}
