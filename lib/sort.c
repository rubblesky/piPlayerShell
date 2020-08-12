#include "sort.h"

#include <stdlib.h>

static int partition(void* array, int p, int r,
                     int (*compare)(void*, int, int),
                     void (*exchange)(void*, int, int));
void quick_sort(void* array, int p, int r, void (*compare)(void*, int, int),
                void (*exchange)(void*, int, int)) {
    if (p < r) {
        int q = partition(array, p, r, compare, exchange);
        quick_sort(array, p, q - 1, compare, exchange);
        quick_sort(array, q + 1, r, compare, exchange);
    }
}
static int partition(void* array, int p, int r,
                     int (*compare)(void*, int, int),
                     void (*exchange)(void*, int, int)) {
    int i = p - 1;
    for (int j = p; j <= r; j++) {
        if ((*compare)(array, j, r) <= 0) {
            i += 1;
            (*exchange)(array, i, j);
        }
    }
    return i;
}
