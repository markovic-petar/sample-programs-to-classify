#ifndef PROG_EM_TEST_HELPER_RNG
#define PROG_EM_TEST_HELPER_RNG

#include <time.h>
#include <stddef.h>

typedef enum
{
    RNG_OK = 0,
    RNG_ERR_OPEN = -1,
    RNG_ERR_READ = -2,
}
rng_error_t;

// Open stream
rng_error_t rng_open();

// Close stream
void rng_close();

// Generate int values
rng_error_t rng_int(int* num);
rng_error_t rng_int_vector(int* vec, size_t dim);

// Generate unsigned int values
rng_error_t rng_uint(unsigned int* num);
rng_error_t rng_uint_vector(unsigned int* vec, size_t dim);

// Generate double values
#define RNG_DOUBLE_MAX 1000.0
#define RNG_DOUBLE_MIN -1000.0
rng_error_t rng_double(double* num);
rng_error_t rng_double_vector(double* vec, size_t dim);

// Generate deterministic int values - Good for testing
void rng_fixed_int_vector(int* vec, size_t dim, unsigned int seed);

#endif
