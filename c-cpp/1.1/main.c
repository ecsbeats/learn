#include "kv_store.h"
#include <stdio.h>

#define NUM_THREADS 4
#define ITEMS_PER_THREAD 100

typedef struct {
    KeyValueStore** store_ptr;
    mtx_t* main_mutex; // Mutex to protect the store pointer itself
    int thread_id;
} ThreadData;

int worker_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    printf("Thread %d starting.\n", data->thread_id);
    
    for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
        char key_buffer[64];
        char value_buffer[64];
        
        snprintf(key_buffer, sizeof(key_buffer), "key_t%d_i%d", data->thread_id, i);
        snprintf(value_buffer, sizeof(value_buffer), "value_from_thread_%d", data->thread_id);
        
        // ** THE FIX **
        // Lock the main mutex BEFORE calling the function that might realloc
        mtx_lock(data->main_mutex);
        if (!store_add_or_update_item(data->store_ptr, key_buffer, value_buffer)) {
            fprintf(stderr, "Thread %d: Failed to add item %s\n", data->thread_id, key_buffer);
        }
        // Unlock AFTER the call is complete
        mtx_unlock(data->main_mutex);
    }
    
    char update_key[64];
    snprintf(update_key, sizeof(update_key), "key_t%d_i%d", data->thread_id, 0);
    
    mtx_lock(data->main_mutex);
    store_add_or_update_item(data->store_ptr, update_key, "UPDATED_VALUE");
    mtx_unlock(data->main_mutex);

    printf("Thread %d finished.\n", data->thread_id);
    return 0;
}

int main(void) {
    KeyValueStore* store = store_create(10);
    if (!store) {
        fprintf(stderr, "Failed to create key-value store.\n");
        return 1;
    }

    thrd_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    // Create the new mutex to protect the store pointer
    mtx_t main_mutex;
    if (mtx_init(&main_mutex, mtx_plain) != thrd_success) {
        fprintf(stderr, "Failed to create main mutex.\n");
        store_destroy(store);
        return 1;
    }

    printf("Starting threads to add/update items concurrently...\n");
    KeyValueStore** store_ptr = &store;

    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_data[i] = (ThreadData){ 
            .store_ptr = store_ptr, 
            .main_mutex = &main_mutex, 
            .thread_id = i 
        };
        if (thrd_create(&threads[i], worker_thread, &thread_data[i]) != thrd_success) {
            fprintf(stderr, "Failed to create thread %d\n", i);
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        thrd_join(threads[i], NULL);
    }
    
    printf("\nAll threads have completed.\n");

    int expected_items = NUM_THREADS * ITEMS_PER_THREAD;
    int expected_adds = expected_items + NUM_THREADS;
    printf("Expected final item count (no duplicates): %d\n", expected_items);
    printf("Expected total additions/updates: %d\n", expected_adds);

    KeyValueStore* final_store = *store_ptr; 
    printf("Actual items in store: %zu\n", final_store->item_count);
    printf("Total additions/updates (from atomic counter): %u\n", final_store->add_count);
    
    const char* original_val = store_get_value(final_store, "key_t1_i50");
    const char* updated_val = store_get_value(final_store, "key_t1_i0");
    
    printf("\nVerifying a few keys:\n");
    printf("  Value for 'key_t1_i50': %s\n", original_val ? original_val : "NOT FOUND");
    printf("  Value for 'key_t1_i0' should be updated: %s\n", updated_val ? updated_val : "NOT FOUND");

    // Clean up all resources
    mtx_destroy(&main_mutex);
    store_destroy(final_store);
    printf("\nStore destroyed. Program finished.\n");

    return 0;
}
