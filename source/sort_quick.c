#include "helper_rng.h"
#include "helper_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

static inline void print_vector(int* arr, size_t dim) {
    printf("\nvector:\n");
    for (size_t i = 0; i < dim; ++i) {
        printf("%13d\n", arr[i]);
    }
    printf("\n");
}

static inline void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline size_t partition(int* arr, size_t down, size_t up) {
    size_t i = down;
    size_t j = up;
    int pivot = arr[down];
    while (i < j) {
        while(arr[i] <= pivot && i < j) {
            i = i + 1;
        }
        while(arr[j] > pivot) {
            j = j - 1;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    arr[down] = arr[j];
    arr[j] = pivot;
    return j;
}

void quicksort(int* arr, size_t low, size_t high) {
    if (low >= high)
        return;
    size_t j = partition(arr, low, high);
    if (j > 0)
        quicksort(arr, low, j-1);
    quicksort(arr, j+1, high);
}

int main(int argc, char** argv) {
    int ret = EXIT_FAILURE;

    /* Allocate vector */
    int* arr_initial = NULL;
    int* arr = NULL;
    size_t dim = 10000;
    arr_initial = malloc(sizeof(int)*dim);
    if (!arr_initial) {
        fprintf(stderr, "Malloc failed\n");
        goto cleanup;
    }
    arr = malloc(sizeof(int)*dim);
    if (!arr) {
        fprintf(stderr, "Malloc failed\n");
        goto cleanup;
    }

    /* Initialize vector */
    rng_fixed_int_vector(arr_initial, dim, 42);

    time_t duration = 0;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK) {
        fprintf(stderr, "Invalid time argument\n");
        goto cleanup;
    }

    time_t start = time(NULL);
    do {
        memcpy(arr, arr_initial, sizeof(int)*dim);
        quicksort(arr, 0, dim-1);
    } while(time(NULL) - start < duration);

    print_vector(arr, 10);
    printf("...\n");

    ret = EXIT_SUCCESS;

cleanup:
    if (arr) free(arr);
    if (arr_initial) free(arr_initial);
    return ret;
}