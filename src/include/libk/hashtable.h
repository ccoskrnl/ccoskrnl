#ifndef __LIBK_HASHTABLE_H__
#define __LIBK_HASHTABLE_H__

#include "../types.h"
#include "list.h"

typedef struct _hashtable_entry
{
    byte* key;
    size_t key_size;

    void* value;

    list_node_t bucket_node;

} hashtable_entry_t;

typedef struct _hashtable
{
    list_node_t *bucket_list;
    uint64_t num_of_buckets;
    uint64_t count;

} hashtable_t;

/**
 * @brief Hash a key.
 * 
 * @param key 
 * @param keylen 
 * @return uint64_t 
 */
uint64_t hash(const byte* key, size_t keylen);

/**
 * @brief Create a new hashtable.
 * 
 * @param size 
 * @return hashtable_t* 
 */
hashtable_t* hashtable_create(int size);

/**
 * @brief Insert a key-value pair into the hashtable.
 * 
 * @param ht 
 * @param key 
 * @param key_size 
 * @param value 
 * @return status_t, ST_SUCCESS if inserted successfully.
 */
status_t hashtable_insert(hashtable_t* ht, byte* key, size_t key_size, void* value);

/**
 * @brief Remove a key-value pair from the hashtable.
 * 
 * @param ht 
 * @param key 
 * @param key_size 
 * @return void* , the value associated with the key or NULL if not found
 */
void* hashtable_remove(hashtable_t* ht, byte* key, size_t key_size);



/**
 * @brief Search for a key in the hashtable.
 * 
 * @param ht 
 * @param key 
 * @param key_size 
 * @return void* , the value associated with the key or NULL if not found
 */
void* hashtable_search(hashtable_t* ht, byte* key, size_t key_size);

#endif