#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <uthash.h>

// Refer to `https://troydhanson.github.io/uthash/userguide.html` for details.
// It is not a library, so there is no need to install it. All you need is copy
// the header uthash.h in your local directory and include it in your project.

typedef struct {
    int key;
    int value;
    UT_hash_handle hh;
} Int;

int main() {
    Int *hashMap = NULL, *result, *entry, *tmp;
    srand(time(NULL));

    // Populate hash table
    for (int id = 1; id <= 10; id++) {
        entry = malloc(sizeof(Int));
        entry->key = id;
        entry->value = rand() % 100;
        HASH_ADD_INT(hashMap, key, entry);
    }

    // Lookup
    int id = 8;
    HASH_FIND_INT(hashMap, &id, result);
    if (result) {
        printf("Found %d, %d\n", result->key, result->value);
    } else {
        printf("Not found\n");
    }

    // Iterate/print all hash key/value pairs
    HASH_ITER(hh, hashMap, entry, tmp) {
        printf("ID: %2d, Value: %3d\n", entry->key, entry->value);
    }

    // Delete all items in the hash table
    HASH_ITER(hh, hashMap, entry, tmp) {
        HASH_DEL(hashMap, entry);
        free(entry);
    }
}