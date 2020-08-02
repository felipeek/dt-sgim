#ifndef SAP_FUSE_WEBHDFS_HASH_MAP_H
#define SAP_FUSE_WEBHDFS_HASH_MAP_H
#include <stddef.h>

typedef int (*KeyCompareFunc)(const void *key1, const void *key2);
typedef unsigned int (*KeyHashFunc)(const void *key);
typedef void (*ForEachFunc)(const void *key, const void* value, void* custom_data);

typedef struct
{
    unsigned int capacity;
    unsigned int num_elements;
    size_t key_size;
    size_t value_size;
    KeyCompareFunc key_compare_func;
    KeyHashFunc key_hash_func;
    void *data;
} Hash_Map;

int hash_map_create(Hash_Map *hm, unsigned int initial_capacity, size_t key_size, size_t value_size,
                    KeyCompareFunc key_compare_func, KeyHashFunc key_hash_func);
int hash_map_put(Hash_Map *hm, const void *key, const void *value);
int hash_map_get(Hash_Map *hm, const void *key, void *value);
int hash_map_delete(Hash_Map *hm, const void *key);
void hash_map_destroy(Hash_Map *hm);
void hash_map_log(Hash_Map *hm);

// @NOTE: Do not put or delete elements in the for_each_func. Putting/deleting elements might alter the hash table internal
// data structure, which will cause unexpected behavior in this function.
void hash_map_for_each_entry(Hash_Map *hm, ForEachFunc for_each_func, void *custom_data);

#endif