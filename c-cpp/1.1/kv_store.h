#ifndef KV_STORE_H
#define KV_STORE_H

#include <stdatomic.h>
#include <threads.h>
#include <stddef.h>
#include <stdbool.h> // For bool type

_Static_assert(sizeof(void*) * 8 >= 32, "This system requires at least a 32-bit architecture.");

typedef struct {
    char* key;
    char* value;
} KeyValuePair;

typedef struct {
    mtx_t mutex;
    atomic_uint add_count; // Total successful additions/updates
    size_t item_count;
    size_t capacity;
    KeyValuePair pairs[]; // Flexible Array Member
} KeyValueStore;

KeyValueStore* store_create(size_t initial_capacity);
void store_destroy(KeyValueStore* store);

/**
 * @brief Adds a key-value pair to the store or updates it if the key exists.
 * This function is thread-safe.
 * @param store_ptr A pointer to the pointer of the store. This is required to handle reallocations safely.
 * @param key The key string.
 * @param value The value string.
 * @return True on success, false on failure.
 */
bool store_add_or_update_item(KeyValueStore** store_ptr, const char* key, const char* value);

const char* store_get_value(KeyValueStore* store, const char* key);

#endif // KV_STORE_H
