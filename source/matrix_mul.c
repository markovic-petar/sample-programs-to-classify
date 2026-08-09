#include "helper_io.h"
#include "helper_rng.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// Matrices' dimensions
#define M 100
#define N 100
#define P 100

void matrix_print(int* mx, size_t mx_rows, size_t mx_cols)
{
    // NULL ptr check
    if (!mx) return;

    printf("matrix:\n");
    for (size_t i = 0; i < mx_rows; ++i)
    {
        for (size_t j = 0; j < mx_cols; ++j)
        {
            printf("%12d", mx[i * mx_cols + j]);
        }
        printf("\n");
    }
}

typedef enum {
    MX_MUL_OK = 0,
    MX_MUL_ERR = -1,
} mx_mul_error_t;

mx_mul_error_t matrix_mul (
    int* dst, size_t dst_rows, size_t dst_cols,
    const int* src1, size_t src1_rows, size_t src1_cols,
    const int* src2, size_t src2_rows, size_t src2_cols
)
{
    // Matrices' NULL ptr. check
    if (!dst || !src1 || !src2)
        return MX_MUL_ERR;

    // Matrices' dimensions check
    if (
        src1_cols != src2_rows ||
        dst_rows != src1_rows  ||
        dst_cols != src2_cols
    )
        return MX_MUL_ERR;

    // Do the thing
    for (size_t i = 0; i < dst_rows; ++i)
    {
        for (size_t j = 0; j < dst_cols; ++j)
        {
            int sum = 0;
            for (size_t k = 0; k < src1_cols; ++k)
            {
                sum += src1[i*src1_cols + k] * src2[k*src2_cols + j]; 
            }
            dst[i*dst_cols + j] = sum;
        }
    }

    return MX_MUL_OK;
}

int main(int argc, char* argv[])
{
    int err = 0;

    // Init. the hw. rng. source
    if (rng_open() != RNG_OK)
        { err = -1; goto cleanup; }

    // Allocate matrices
    int *a = NULL, *b = NULL, *c = NULL;

    a = (int*)malloc(M * N * sizeof(int));
    if (!a) { err = -1; goto cleanup; }

    b = (int*)malloc(N * P * sizeof(int));
    if (!b) { err = -1; goto cleanup; }

    c = (int*)malloc(M * P * sizeof(int));
    if (!c) { err = -1; goto cleanup; }

    // Init. vectors with random data
    if (rng_int_vector(a, M * N) != RNG_OK)
        { err = -1; goto cleanup; }

    if (rng_int_vector(b, N * P) != RNG_OK)
        { err = -1; goto cleanup; }

    // Get program repeat duration
    time_t duration;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK)
    {
        // bad input
        printf("Input error. Correct usage: %s [seconds]", argv[0]);
        err = -1;
        goto cleanup;
    }

    time_t start_time = time(NULL);

    // multiply matrices: C = A * B
    while(time(NULL) - start_time < duration)
    {
        matrix_mul(
            c, M, P,
            a, M, N,
            b, N, P
        );
    }

    // Print result
    // matrix_print(c, M, P);

cleanup:
    // Close rng
    rng_close();

    // Dealloc vectors
    if (c) free(c);
    if (b) free(b);
    if (a) free(a);
    
    return err;
}
