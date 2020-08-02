#include "hash_map.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    int valid;
} Hash_Map_Element_Information;

static Hash_Map_Element_Information *get_element_information(Hash_Map *hm, unsigned int index)
{
    return (Hash_Map_Element_Information *)((unsigned char *)hm->data +
                                            index * (sizeof(Hash_Map_Element_Information) + hm->key_size + hm->value_size));
}

static void *get_element_key(Hash_Map *hm, unsigned int index)
{
    Hash_Map_Element_Information *hmei = get_element_information(hm, index);
    return (unsigned char *)hmei + sizeof(Hash_Map_Element_Information);
}

static void *get_element_value(Hash_Map *hm, unsigned int index)
{
    Hash_Map_Element_Information *hmei = get_element_information(hm, index);
    return (unsigned char *)hmei + sizeof(Hash_Map_Element_Information) + hm->key_size;
}

static void put_element_key(Hash_Map *hm, unsigned int index, const void *key)
{
    void *target = get_element_key(hm, index);
    memcpy(target, key, hm->key_size);
}

static void put_element_value(Hash_Map *hm, unsigned int index, const void *value)
{
    void *target = get_element_value(hm, index);
    memcpy(target, value, hm->value_size);
}

int hash_map_create(Hash_Map *hm, unsigned int initial_capacity, size_t key_size, size_t value_size,
                    KeyCompareFunc key_compare_func, KeyHashFunc key_hash_func)
{
    hm->key_compare_func = key_compare_func;
    hm->key_hash_func = key_hash_func;
    hm->key_size = key_size;
    hm->value_size = value_size;
    hm->capacity = initial_capacity > 0 ? initial_capacity : 1;
    hm->num_elements = 0;
    hm->data = calloc(hm->capacity, sizeof(Hash_Map_Element_Information) + key_size + value_size);
    if (!hm->data)
    {
        printf("Error: not enough memory to allocate hash map.");
        return -1;
    }
    return 0;
}

void hash_map_destroy(Hash_Map *hm)
{
    free(hm->data);
}

static int hash_map_grow(Hash_Map *hm)
{
    Hash_Map old_hm = *hm;
    if (hash_map_create(hm, old_hm.capacity << 1, old_hm.key_size, old_hm.value_size, old_hm.key_compare_func, old_hm.key_hash_func))
    {
        printf("Error when growing hash map: could not create new hash map");
        return -1;
    }

    for (unsigned int pos = 0; pos < old_hm.capacity; ++pos)
    {
        Hash_Map_Element_Information *hmei = get_element_information(&old_hm, pos);
        if (hmei->valid)
        {
            void *key = get_element_key(&old_hm, pos);
            void *value = get_element_value(&old_hm, pos);
            if (hash_map_put(hm, key, value))
            {
                printf("Error when growing hash map: could not put element");
                return -1;
            }
        }
    }

    hash_map_destroy(&old_hm);
    return 0;
}

int hash_map_put(Hash_Map *hm, const void *key, const void *value)
{
    unsigned int pos = hm->key_hash_func(key) % hm->capacity;

    for (;;)
    {
        Hash_Map_Element_Information *hmei = get_element_information(hm, pos);
        if (!hmei->valid)
        {
            hmei->valid = 1;
            put_element_key(hm, pos, key);
            put_element_value(hm, pos, value);
            ++hm->num_elements;
            break;
        }
        else
        {
            void *element_key = get_element_key(hm, pos);
            if (hm->key_compare_func(element_key, key))
            {
                put_element_key(hm, pos, key);
                put_element_value(hm, pos, value);
                break;
            }
        }

        pos = (pos + 1) % hm->capacity;
    }

    if ((hm->num_elements << 2) > hm->capacity)
    {
        if (hash_map_grow(hm))
        {
            return -1;
        }
    }

    return 0;
}

int hash_map_get(Hash_Map *hm, const void *key, void *value)
{
    unsigned int pos = hm->key_hash_func(key) % hm->capacity;
    for (;;)
    {
        Hash_Map_Element_Information *hmei = get_element_information(hm, pos);
        if (hmei->valid)
        {
            void *possible_key = get_element_key(hm, pos);
            if (hm->key_compare_func(possible_key, key))
            {
                void *entry_value = get_element_value(hm, pos);
                memcpy(value, entry_value, hm->value_size);
                return 0;
            }
        }
        else
        {
            return -1;
        }

        pos = (pos + 1) % hm->capacity;
    }
}

static void adjust_gap(Hash_Map *hm, unsigned int gap_index)
{
    unsigned int pos = (gap_index + 1) % hm->capacity;
    for (;;)
    {
        Hash_Map_Element_Information *current_hmei = get_element_information(hm, pos);
        if (!current_hmei->valid)
        {
            break;
        }

        void *current_key = get_element_key(hm, pos);
        unsigned int hash_position = hm->key_hash_func(current_key) % hm->capacity;
        unsigned int normalized_gap_index = (gap_index < hash_position) ? gap_index + hm->capacity : gap_index;
        unsigned int normalized_pos = (pos < hash_position) ? pos + hm->capacity : pos;
        if (normalized_gap_index >= hash_position && normalized_gap_index <= normalized_pos)
        {
            void *current_value = get_element_value(hm, pos);
            current_hmei->valid = 0;

            Hash_Map_Element_Information *gap_hmei = get_element_information(hm, gap_index);
            put_element_key(hm, gap_index, current_key);
            put_element_value(hm, gap_index, current_value);
            gap_hmei->valid = 1;
            gap_index = pos;
        }
        pos = (pos + 1) % hm->capacity;
    }
}

int hash_map_delete(Hash_Map *hm, const void *key)
{
    unsigned int pos = hm->key_hash_func(key) % hm->capacity;
    for (;;)
    {
        Hash_Map_Element_Information *hmei = get_element_information(hm, pos);
        if (hmei->valid)
        {
            void *possible_key = get_element_key(hm, pos);
            if (hm->key_compare_func(possible_key, key))
            {
                hmei->valid = 0;
                adjust_gap(hm, pos);
                --hm->num_elements;
                return 0;
            }
        }
        else
        {
            return -1;
        }

        pos = (pos + 1) % hm->capacity;
    }
}

char buffer[1 * 1024 * 1024];
#include <assert.h>
void hash_map_log(Hash_Map *hm)
{
    static int flag = 0;
    assert(flag == 0);
    flag = 1;
    int written = 0;
    written += sprintf(buffer, "\n\nhashmap size: %d\nhashmap ptr: %p\n", hm->capacity, hm->data);
    for (unsigned int pos = 0; pos < hm->capacity; ++pos)
    {
        Hash_Map_Element_Information *hmei = get_element_information(hm, pos);
        if (hmei->valid)
        {
            void *key = get_element_key(hm, pos);
            void *value = get_element_value(hm, pos);
            written += sprintf(buffer + written, "%u. %d -> %p\n", pos, *(int *)key, *(void **)value);
        }
    }

    written += sprintf(buffer + written, "the end of this hash map");
    buffer[written] = 0;
    printf("%s", buffer);
    flag = 0;

    #if 0
    printf("HashMap Size: %d", hm->capacity);
    for (unsigned int pos = 0; pos < hm->capacity; ++pos)
    {
        Hash_Map_Element_Information *hmei = get_element_information(hm, pos);
        if (hmei->valid)
        {
            void *key = get_element_key(hm, pos);
            void *value = get_element_value(hm, pos);
            printf("%u. %d -> %p", pos, *(int *)key, *(void **)value);
        }
    }
    #endif
}

void hash_map_for_each_entry(Hash_Map *hm, ForEachFunc for_each_func, void *custom_data)
{
    for (unsigned int pos = 0; pos < hm->capacity; ++pos)
    {
        Hash_Map_Element_Information *hmei = get_element_information(hm, pos);
        if (hmei->valid)
        {
            void *key = get_element_key(hm, pos);
            void *value = get_element_value(hm, pos);
            for_each_func(key, value, custom_data);
        }
    }
}