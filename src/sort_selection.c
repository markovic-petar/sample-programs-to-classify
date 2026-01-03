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

void selection_sort(int* arr, size_t dim) {
    if (!arr || !dim) return;

    for (size_t i = 0; i < dim - 1; ++i) {
        int min = arr[i];
        size_t pos = i;
        /* find min */
        for (size_t j = i + 1; j < dim; ++j) {
            if (arr[j] < min) {
                min = arr[j];
                pos = j;
            }
        }
        /* swap */
        arr[pos] = arr[i];
        arr[i] = min;
    }
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
        selection_sort(arr, dim);
    } while(time(NULL) - start < duration);

    print_vector(arr, 10);
    printf("...\n");

    ret = EXIT_SUCCESS;

cleanup:
    if (arr) free(arr);
    if (arr_initial) free(arr_initial);
    return ret;
}