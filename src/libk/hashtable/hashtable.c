#include "../../include/types.h"
#include "../../include/libk/hashtable.h"
#include "../../include/libk/stdlib.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325ULL
#define FNV_PRIME 0x100000001b3ULL

uint64_t fnv1a_hash(const uint8_t* key, size_t keylen)
{
    const uint8_t* p = key;
    uint64_t hash = FNV_OFFSET_BASIS;
    size_t i;

    for (i = 0; i < keylen; i++) 
    {
        hash ^= (uint64_t)(p[i]);
        hash *= FNV_PRIME;
    }

    return hash;
}

/**
 * @brief Hash a key.
 * 
 * @param key 
 * @param keylen 
 * @return uint64_t 
 */
uint64_t hash(const byte* key, size_t keylen)
{
    return fnv1a_hash(key, keylen);
}

/**
 * @brief Create a new hashtable.
 * 
 * @param size 
 * @return hashtable_t* 
 */
hashtable_t* hashtable_create(int size)
{
    hashtable_t* ht = malloc(sizeof(hashtable_t));
    if (!ht) return NULL;

    ht->num_of_buckets = size;
    ht->count = 0;
    ht->bucket_list = (list_node_t*)calloc(sizeof(list_node_t) * size);

    if (!ht->bucket_list) {
        free(ht);
        return NULL;
    }

    return ht;
}

/**
 * @brief Insert a key-value pair into the hashtable.
 * 
 * @param ht 
 * @param key 
 * @param key_size 
 * @param value 
 * @return status_t 
 */
status_t hashtable_insert(hashtable_t* ht, byte* key, size_t key_size, void* value)
{

    if (!ht || !key || key_size == 0) return ST_FAILED;

    status_t status = ST_SUCCESS;

    uint64_t index = hash(key, key_size) % ht->num_of_buckets;

    hashtable_entry_t *new_entry = (hashtable_entry_t*)malloc(sizeof(hashtable_entry_t));

    if (new_entry == NULL)
    {
        return ST_FAILED;
    }

    new_entry->key = (byte*)malloc(key_size);
    memcpy(new_entry->key, key, key_size);
    new_entry->key_size = key_size;

    new_entry->value = value;

    list_node_t* corresponding_list = &ht->bucket_list[index];

    _list_push(corresponding_list, &new_entry->bucket_node);


    return status;
}


/**
 * @brief Remove a key-value pair from the hashtable.
 * 
 * @param ht 
 * @param key 
 * @param key_size 
 * @return void* , the value associated with the key or NULL if not found
 */
void* hashtable_remove(hashtable_t* ht, byte* key, size_t key_size)
{
    hashtable_entry_t* target = NULL;
    void* value = NULL;

    if (!ht || !key || key_size == 0) return target;

    uint64_t index = hash(key, key_size) % ht->num_of_buckets;
    list_node_t* corresponding_list = &ht->bucket_list[index];

    for (list_node_t* next_node = corresponding_list->flink; next_node; next_node = next_node->flink)
    {
        hashtable_entry_t* entry = (hashtable_entry_t*)struct_base(hashtable_entry_t, bucket_node, next_node);
        if (entry->key_size == key_size && !memcmp(entry->key, key, key_size))
        {
            target = entry;
            value = target->value;
            _list_remove(corresponding_list, next_node);
            break;
        }
    }

    free(target->key);
    free(target);
    return value;
}



/**
 * @brief Search for a key in the hashtable.
 * 
 * @param ht 
 * @param key 
 * @param key_size 
 * @return void* , the value associated with the key or NULL if not found
 */
void* hashtable_search(hashtable_t* ht, byte* key, size_t key_size)
{
    void* target = NULL;

    if (!ht || !key || key_size == 0) return target;

    uint64_t index = hash(key, key_size) % ht->num_of_buckets;
    list_node_t* corresponding_list = &ht->bucket_list[index];

    for (list_node_t* next_node = corresponding_list->flink; next_node; next_node = next_node->flink)
    {
        hashtable_entry_t* entry = (hashtable_entry_t*)struct_base(hashtable_entry_t, bucket_node, next_node);
        if (entry->key_size == key_size && !memcmp(entry->key, key, key_size))
        {
            target = entry->value;
            break;
        }
    }

    return target;
}