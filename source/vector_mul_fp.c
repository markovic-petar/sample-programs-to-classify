#include "helper_io.h"
#include "helper_rng.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


// vector dimension
#define N 10000

void print_vector(double* vec, size_t dim)
{
    printf("vector:\n");
    for (int i = 0; i < dim; ++i)
        printf("\t%.6e\n", vec[i]);
    printf("\n");
}

void mul_vector(double* dst, double* src1, double* src2, size_t dim)
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
    double *a = NULL, *b = NULL, *c = NULL;

    a = (double*)malloc(N * sizeof(double));
    if (!a) { fprintf(stderr, PROGRAM_NAME ": Malloc failed\n"); err = -1; goto cleanup; }

    b = (double*)malloc(N * sizeof(double));
    if (!b) { fprintf(stderr, PROGRAM_NAME ": Malloc failed\n"); err = -1; goto cleanup; }

    c = (double*)malloc(N * sizeof(double));
    if (!c) { fprintf(stderr, PROGRAM_NAME ": Malloc failed\n"); err = -1; goto cleanup; }

    // Init. vectors with random data
    if (rng_double_vector(a, N) != RNG_OK)
        { fprintf(stderr, PROGRAM_NAME ": rng_double_vector() failed\n"); err = -1; goto cleanup; }

    if (rng_double_vector(b, N) != RNG_OK)
        { fprintf(stderr, PROGRAM_NAME ": rng_double_vector() failed\n"); err = -1; goto cleanup; }

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
        mul_vector(c, a, b, N);
        // double* temp = c; c = a; a = b; b = temp;
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
