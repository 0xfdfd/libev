#ifndef __EV_MAP_H__
#define __EV_MAP_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup EAF-Utils
 * @defgroup EAF-Map Map
 * @{
 */

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include "map_low.h"

/**
 * @brief Static initializer for #eaf_map_t
 * @see eaf_map_t
 * @param[in] cmp	Compare function
 * @param[in] arg	Argument for compare function
 */
#define EAF_MAP_INIT(cmp, arg)	{ EAF_MAP_LOW_INITIALIZER, { cmp, arg }, 0 }

/**
 * @brief Static initializer for #eaf_map_node_t
 * @see eaf_map_node_t
 */
#define EAF_MAP_NODE_INIT		EAF_MAP_LOW_NODE_INITIALIZER

/**
 * @brief The node for map
 * @see eaf_map_t
 * @see EAF_MAP_NODE_INITIALIZER
 */
typedef eaf_map_low_node_t eaf_map_node_t;

/**
 * @brief Compare function.
 * @param key1	The key in the map
 * @param key2	The key user given
 * @param arg	User defined argument
 * @return		-1 if key1 < key2. 1 if key1 > key2. 0 if key1 == key2.
 */
typedef int(*eaf_map_cmp_fn)(const eaf_map_node_t* key1, const eaf_map_node_t* key2, void* arg);

/**
 * @brief Map implemented as red-black tree
 * @see EAF_MAP_INITIALIZER
 */
typedef struct eaf_map
{
	eaf_map_low_t		map_low;	/**< Underlying structure */

	struct
	{
		eaf_map_cmp_fn	cmp;		/**< Pointer to compare function */
		void*			arg;		/**< User defined argument, which will passed to compare function */
	}cmp;							/**< Compare function data */

	size_t				size;		/**< The number of nodes */
}eaf_map_t;

/**
 * @brief Initialize the map referenced by handler.
 * @param handler	The pointer to the map
 * @param cmp		The compare function. Must not NULL
 * @param arg		User defined argument. Can be anything
 */
void eaf_map_init(eaf_map_t* handler, eaf_map_cmp_fn cmp, void* arg);

/**
 * @brief Insert the node into map.
 * @warning the node must not exist in any map.
 * @param handler	The pointer to the map
 * @param node		The node
 * @return			0 if success, -1 otherwise
 */
int eaf_map_insert(eaf_map_t* handler, eaf_map_node_t* node);

/**
 * @brief Delete the node from the map.
 * @warning The node must already in the map.
 * @param handler	The pointer to the map
 * @param node		The node
 */
void eaf_map_erase(eaf_map_t* handler, eaf_map_node_t* node);

/**
 * @brief Get the number of nodes in the map.
 * @param handler	The pointer to the map
 * @return			The number of nodes
 */
size_t eaf_map_size(const eaf_map_t* handler);

/**
 * @brief Finds element with specific key
 * @param handler	The pointer to the map
 * @param key		The key
 * @return			An iterator point to the found node
 */
eaf_map_node_t* eaf_map_find(const eaf_map_t* handler,
	const eaf_map_node_t* key);

/**
 * @brief Returns an iterator to the first element not less than the given key
 * @param handler	The pointer to the map
 * @param key		The key
 * @return			An iterator point to the found node
 */
eaf_map_node_t* eaf_map_find_lower(const eaf_map_t* handler,
	const eaf_map_node_t* key);

/**
 * @brief Returns an iterator to the first element greater than the given key
 * @param handler	The pointer to the map
 * @param key		The key
 * @return			An iterator point to the found node
 */
eaf_map_node_t* eaf_map_find_upper(const eaf_map_t* handler,
	const eaf_map_node_t* key);

/**
 * @brief Returns an iterator to the beginning
 * @param handler	The pointer to the map
 * @return			An iterator
 */
eaf_map_node_t* eaf_map_begin(const eaf_map_t* handler);

/**
 * @brief Returns an iterator to the end
 * @param handler	The pointer to the map
 * @return			An iterator
 */
eaf_map_node_t* eaf_map_end(const eaf_map_t* handler);

/**
 * @brief Get an iterator next to the given one.
 * @param node		Current iterator
 * @return			Next iterator
 */
eaf_map_node_t* eaf_map_next(const eaf_map_node_t* node);

/**
 * @brief Get an iterator before the given one.
 * @param node		Current iterator
 * @return			Previous iterator
 */
eaf_map_node_t* eaf_map_prev(const eaf_map_node_t* node);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
