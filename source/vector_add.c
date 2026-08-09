#include "helper_io.h"
#include "helper_rng.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// vector dimension
#define N 10000

void print_vector(int* vec, int dim)
{
    printf("vector:\n");
    for (int i = 0; i < dim; ++i)
        printf("%16d\n", vec[i]);
    printf("\n");
}

void add_vector(int* dst, int* src1, int* src2, int dim)
{
    for (int i = 0; i < dim; ++i)
        dst[i] = src1[i] + src2[i];
}

int main(int argc, char* argv[])
{
    int err = 0;

    // Init. the hw. rng. source
    if (rng_open() != RNG_OK)
        { fprintf(stderr, PROGRAM_NAME ": rng_open() failed\n"); err = -1; goto cleanup; }

    // Allocate vectors
    int *a = NULL, *b = NULL, *c = NULL;

    a = (int*)malloc(N * sizeof(int));
    if (!a) { fprintf(stderr, PROGRAM_NAME ": Malloc failed\n"); err = -1; goto cleanup; }

    b = (int*)malloc(N * sizeof(int));
    if (!b) { fprintf(stderr, PROGRAM_NAME ": Malloc failed\n"); err = -1; goto cleanup; }

    c = (int*)malloc(N * sizeof(int));
    if (!c) { fprintf(stderr, PROGRAM_NAME ": Malloc failed\n"); err = -1; goto cleanup; }

    // Init. vectors with random data
    if (rng_int_vector(a, N) != RNG_OK)
        { fprintf(stderr, PROGRAM_NAME ": rng_int_vector() failed\n"); err = -1; goto cleanup; }

    if (rng_int_vector(b, N) != RNG_OK)
        { fprintf(stderr, PROGRAM_NAME ": rng_int_vector() failed\n"); err = -1; goto cleanup; }

    // Get program repeat duration
    time_t duration;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK)
    {
        // bad input
        fprintf(stderr, PROGRAM_NAME ": Input error. Correct usage: %s [seconds]\n", argv[0]);
        err = -1;
        goto cleanup;
    }

    time_t start_time = time(NULL);

    // Add vectors
    while(time(NULL) - start_time < duration)
    {
        add_vector(c, a, b, N);
        // int* temp = c; c = a; a = b; b = temp;
    }

    // Print result
    // print_vector(c, N);

cleanup:
    // Close rng
    rng_close();

    // Dealloc vectors
    if (c) free(c);
    if (b) free(b);
    if (a) free(a);
    
    return err;
}
