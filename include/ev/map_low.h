#ifndef __EV_MAP_LOW_H__
#define __EV_MAP_LOW_H__
#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/**
 * @brief Static initializer for #eaf_map_low_t
 * @see eaf_map_low_t
 */
#define EV_MAP_LOW_INITIALIZER			{ NULL }

/**
 * @brief Static initializer for #eaf_map_low_node_t
 * @see eaf_map_low_node_t
 */
#define EV_MAP_LOW_NODE_INITIALIZER	{ NULL, NULL, NULL }

/**
 * @brief find helper
 * @param ret			result
 * @param p_table		eaf_map_low_t*
 * @param USER_TYPE		user type
 * @param user			user data
 * @param user_vs_orig	compare rule
 */
#define EV_MAP_LOW_FIND_HELPER(ret, p_table, USER_TYPE, user, user_vs_orig)	\
	do {\
		int flag_success = 0;\
		ev_map_low_t* __table = p_table;\
		ev_map_low_node_t* node = __table->rb_root;\
		ret = NULL;\
		while (node) {\
			USER_TYPE* orig = EAF_CONTAINER_OF(node, USER_TYPE, node);\
			int cmp_ret = user_vs_orig;\
			if (cmp_ret < 0) {\
				node = node->rb_left;\
			} else if (cmp_ret > 0) {\
				node = node->rb_right;\
			} else {\
				flag_success = 1;\
				ret = orig;\
				break;\
			}\
		}\
		if (flag_success) {\
			break;\
		}\
	} while (0)

/**
 * @brief insert helper
 * @param ret				result
 * @param p_table			eaf_map_low_t*
 * @param USER_TYPE			user type
 * @param user				address
 * @param user_vs_orig		compare rule
 */
#define EV_MAP_LOW_INSERT_HELPER(ret, p_table, USER_TYPE, user, user_vs_orig)	\
	do {\
		int flag_failed = 0;\
		ev_map_low_t* __table = p_table;\
		ev_map_low_node_t **new_node = &(__table->rb_root), *parent = NULL;\
		ret = eaf_errno_success;\
		while (*new_node) {\
			USER_TYPE* orig = EAF_CONTAINER_OF(*new_node, USER_TYPE, node);\
			int cmp_ret = user_vs_orig;\
			parent = *new_node;\
			if (cmp_ret < 0) {\
				new_node = &((*new_node)->rb_left);\
			} else if (cmp_ret > 0) {\
				new_node = &((*new_node)->rb_right);\
			} else {\
				flag_failed = 1;\
				ret = eaf_errno_duplicate;\
				break;\
			}\
		}\
		if (flag_failed) {\
			break;\
		}\
		eaf_map_low_link_node(&(user)->node, parent, new_node);\
		eaf_map_low_insert_color(&(user)->node, __table);\
	} while (0)

/**
 * @brief eaf_map_low node
 * @see EAF_MAP_LOW_NODE_INITIALIZER
 */
typedef struct ev_map_low_node
{
	struct ev_map_low_node*	__rb_parent_color;	/**< parent node | color */
	struct ev_map_low_node*	rb_right;			/**< right node */
	struct ev_map_low_node*	rb_left;			/**< left node */
}ev_map_low_node_t;

/**
 * @brief red-black tree
 * @see EAF_MAP_LOW_INITIALIZER
 */
typedef struct ev_map_low
{
	ev_map_low_node_t*			rb_root;			/**< root node */
}ev_map_low_t;

/**
 * @brief Returns an iterator to the beginning
 * @param root		The pointer to the map
 * @return			An iterator
 */
ev_map_low_node_t* eaf_map_low_first(const ev_map_low_t* root);

/**
 * @brief Returns an iterator to the end
 * @param root		The pointer to the map
 * @return			An iterator
 */
ev_map_low_node_t* eaf_map_low_last(const ev_map_low_t* root);

/**
 * @brief Get an iterator next to the given one.
 * @param node		Current iterator
 * @return			Next iterator
 */
ev_map_low_node_t* eaf_map_low_next(const ev_map_low_node_t* node);

/**
 * @brief Get an iterator before the given one.
 * @param node		Current iterator
 * @return			Previous iterator
 */
ev_map_low_node_t* eaf_map_low_prev(const ev_map_low_node_t* node);

/**
 * @brief Inserting data into the tree.
 *
 * The insert instead must be implemented
 * in two steps: First, the code must insert the element in order as a red leaf
 * in the tree, and then the support library function #eaf_map_low_insert_color
 * must be called.
 *
 * @param node		The node you want to insert
 * @param parent	The position you want to insert
 * @param rb_link	Will be set to `node`
 * @see eaf_map_low_insert_color
 */
void eaf_map_low_link_node(ev_map_low_node_t* node,
	ev_map_low_node_t* parent, ev_map_low_node_t** rb_link);

/**
 * @brief re-balancing ("recoloring") the tree.
 * @param node		The node just linked
 * @param root		The map
 * @see eaf_map_low_link_node
 */
void eaf_map_low_insert_color(ev_map_low_node_t* node,
	ev_map_low_t* root);

/**
 * @brief Delete the node from the map.
 * @warning The node must already in the map.
 * @param root		The pointer to the map
 * @param node		The node
 */
void eaf_map_low_erase(ev_map_low_t* root,
	ev_map_low_node_t* node);

/**
 * @}
 */

#ifdef __cplusplus
};
#endif	/* __cplusplus */
#endif
