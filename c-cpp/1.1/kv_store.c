#include "kv_store.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* duplicate_string(const char* src) {
    if (!src) return NULL;
    size_t len = strlen(src) + 1;
    char* dst = malloc(len);
    if (dst) {
        memcpy(dst, src, len);
    }
    return dst;
}

KeyValueStore* store_create(size_t initial_capacity) {
    initial_capacity = (initial_capacity == 0) ? 8 : initial_capacity;
    
    KeyValueStore* store = malloc(sizeof(KeyValueStore) + initial_capacity * sizeof(KeyValuePair));
    if (!store) return NULL;

    if (mtx_init(&store->mutex, mtx_plain) != thrd_success) {
        free(store);
        return NULL;
    }
    
    atomic_init(&store->add_count, 0);
    store->item_count = 0;
    store->capacity = initial_capacity;

    return store;
}

void store_destroy(KeyValueStore* store) {
    if (!store) return;
    for (size_t i = 0; i < store->item_count; ++i) {
        free(store->pairs[i].key);
        free(store->pairs[i].value);
    }
    mtx_destroy(&store->mutex);
    free(store);
}

bool store_add_or_update_item(KeyValueStore** store_ptr, const char* key, const char* value) {
    if (!store_ptr || !(*store_ptr) || !key || !value) return false;

    // The internal mutex protects the contents (the data)
    mtx_lock(&(*store_ptr)->mutex);

    for (size_t i = 0; i < (*store_ptr)->item_count; ++i) {
        if (strcmp((*store_ptr)->pairs[i].key, key) == 0) {
            char* new_value = duplicate_string(value);
            if (!new_value) {
                mtx_unlock(&(*store_ptr)->mutex);
                return false;
            }
            free((*store_ptr)->pairs[i].value);
            (*store_ptr)->pairs[i].value = new_value;
            atomic_fetch_add(&(*store_ptr)->add_count, 1);
            mtx_unlock(&(*store_ptr)->mutex);
            return true;
        }
    }

    if ((*store_ptr)->item_count >= (*store_ptr)->capacity) {
        size_t new_capacity = (*store_ptr)->capacity * 2;
        KeyValueStore* new_store = realloc(*store_ptr, sizeof(KeyValueStore) + new_capacity * sizeof(KeyValuePair));
        
        if (!new_store) {
            mtx_unlock(&(*store_ptr)->mutex);
            return false;
        }
        *store_ptr = new_store;
        (*store_ptr)->capacity = new_capacity;
    }

    char* new_key = duplicate_string(key);
    char* new_value = duplicate_string(value);
    if (!new_key || !new_value) {
        free(new_key);
        free(new_value);
        mtx_unlock(&(*store_ptr)->mutex);
        return false;
    }

    (*store_ptr)->pairs[(*store_ptr)->item_count].key = new_key;
    (*store_ptr)->pairs[(*store_ptr)->item_count].value = new_value;
    (*store_ptr)->item_count++;
    
    atomic_fetch_add(&(*store_ptr)->add_count, 1);
    mtx_unlock(&(*store_ptr)->mutex);
    return true;
}

const char* store_get_value(KeyValueStore* store, const char* key) {
    if (!store || !key) return NULL;
    
    mtx_lock(&store->mutex);
    const char* found_value = NULL;
    for (size_t i = 0; i < store->item_count; ++i) {
        if (strcmp(store->pairs[i].key, key) == 0) {
            found_value = store->pairs[i].value;
            break;
        }
    }
    mtx_unlock(&store->mutex);
    return found_value;
}
