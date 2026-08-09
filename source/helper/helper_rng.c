#include "helper_rng.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


static FILE* rng = NULL;
static const char err_message_fread_fail[] = "RNG: fread failed";
static const char err_message_rng_not_initialized[] = "RNG: rng not initialized";

// #########################################################
// ###   Open/Close /dev/urandom                         ###
// #########################################################

// Open stream
rng_error_t rng_open()
{
    /* Open /dev/urandom/ */
    rng = fopen("/dev/urandom", "rb");
    if (!rng) return RNG_ERR_OPEN;

    return RNG_OK;
}

// Close stream
void rng_close()
{
    if (rng) fclose(rng);
    rng = NULL;
}

// Check if stream is open
static inline int rng_is_open()
{
    if (!rng)
    {
        return 0;
    }
    return 1;
}

// Helper function for handling fread errors
static inline rng_error_t rng_handle_fread(size_t fread_out, size_t dim)
{
    if (fread_out != dim)
    {
        perror(err_message_fread_fail);
        return RNG_ERR_READ;
    }
    return RNG_OK;
}

// #########################################################
// ###   Random Data                                     ###
// #########################################################

/* RNG - Generate int values */
rng_error_t rng_int(int* num)
{
    if (!rng_is_open()) return RNG_ERR_READ;
    return rng_handle_fread(fread(num, sizeof(int), 1, rng), 1);
}

rng_error_t rng_int_vector(int* vec, size_t dim)
{
    if (!rng_is_open()) return RNG_ERR_READ;
    return rng_handle_fread(fread(vec, sizeof(int), dim, rng), dim);
}

/* RNG - Generate unsigned int values */
rng_error_t rng_uint(unsigned int* num)
{
    if (!rng_is_open()) return RNG_ERR_READ;
    return rng_handle_fread(fread(num, sizeof(unsigned int), 1, rng), 1);
}

rng_error_t rng_uint_vector(unsigned int* vec, size_t dim)
{
    if (!rng_is_open()) return RNG_ERR_READ;
    return rng_handle_fread(fread(vec, sizeof(unsigned int), dim, rng), dim);
}

/* RNG - Generate double values */
rng_error_t rng_double(double* num)
{
    // Check if rng stream is open
    if (!rng_is_open()) return RNG_ERR_READ;

    // Generate a random uint64_t
    uint64_t r;
    if (rng_handle_fread(fread(&r, sizeof(r), 1, rng), 1) != RNG_OK)
        return RNG_ERR_READ;

    // Convert and normalize the random uint64_t to a value between RNG_DOUBLE_MAX and RNG_DOUBLE_MIN
    double normalized = (double)r / (double)UINT64_MAX;
    *num = RNG_DOUBLE_MIN + normalized * (RNG_DOUBLE_MAX - RNG_DOUBLE_MIN);

    // Return status
    return RNG_OK;
}

rng_error_t rng_double_vector(double* vec, size_t dim)
{
    if (!rng_is_open()) return RNG_ERR_READ;
    for (size_t i = 0; i < dim; ++i)
    {
        rng_error_t ret = rng_double(&vec[i]);
        if ( ret != RNG_OK) return ret;
    }
    return RNG_OK;
}

// #########################################################
// ###   Predetermined data                              ###
// #########################################################

void rng_fixed_int_vector(int* vec, size_t dim, unsigned int seed) {
    srand(seed);
    int min = -(RAND_MAX / 2);
    int max = (RAND_MAX / 2);
    int range = max - min + 1;
    
    for (size_t i = 0; i < dim; ++i) {
        vec[i] = min + rand() % range;
    }
}

