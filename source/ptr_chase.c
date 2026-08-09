/* 
   Copyright (c) 2016, 2018 Andreas F. Borchert
   All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
   KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include "helper_io.h"
#include "helper_rng.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

/* this variable must not be declared static */
volatile void* chase_pointers_global; // to defeat optimizations

void chase_pointers(void** memory, size_t count) {
    void** p = (void**) memory;
    while (count-- > 0) {
        p = (void**) *p;//;
    }
    chase_pointers_global = *p;
}

void swap(size_t* v1, size_t* v2) {
    size_t tmp = *v1;
    *v1 = *v2;
    *v2 = tmp;
}

/* create a cyclic pointer chain that covers all words
   in a memory section of the given size in a randomized order */
int create_random_chain(void*** memory, size_t size) {
    int err = -1;
    if (!memory) return err;

    if (rng_open() != RNG_OK) goto cleanup;

    void** tmp_memory = NULL;
    tmp_memory = malloc(size);
    if (!tmp_memory) goto cleanup;

    size_t len = size / sizeof(void*);

    // shuffle indices
    size_t* indices = NULL;
    indices = malloc(sizeof(size_t)*len);
    if (!indices) goto cleanup;

    for (size_t i = 0; i < len; ++i) {
        indices[i] = i;
    }
    for (size_t i = 0; i < len-1; ++i) {
        unsigned int rand_uint = 0;
        if (rng_uint(&rand_uint) != RNG_OK) goto cleanup;
        size_t j = i + (rand_uint % (len - i));
        if (i != j) {
            swap(&indices[i], &indices[j]);
        }
    }

    // fill memory with indices of the next element
    for (size_t i = 1; i < len; ++i) {
        tmp_memory[indices[i-1]] = (void*)&tmp_memory[indices[i]];
    }
    tmp_memory[indices[len-1]] = (void*)&tmp_memory[indices[0]];

    err = 0;

cleanup:
    if (err) {
        if (tmp_memory) free(tmp_memory); 
    }
    else {
        *memory = tmp_memory;
    }
    if (indices) free(indices);
    rng_close();
    return err;
}

#define SIZE (1024 * 1024 * 16)

int main(int argc, char** argv) {

    int ret = EXIT_FAILURE;

    void** memory = NULL;
    size_t memsize = SIZE;
    size_t len = memsize / sizeof(void*);

    /* Create pointer chain */
    if (create_random_chain(&memory, memsize)) {
        fprintf(stderr, PROGRAM_NAME ": " "Pointer chain creation failed\n");
        goto cleanup;
    }

    /* Get time duration */
    time_t duration = 0;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK) {
        fprintf(stderr, PROGRAM_NAME ": " "Invalid time argument\n");
        goto cleanup;
    }

    /* Open rng */
    rng_open();

    time_t start = time(NULL);
    do {
        unsigned int rand_offs = 0;
        if (rng_uint(&rand_offs) != RNG_OK) {
            fprintf(stderr, PROGRAM_NAME ": " "rng_uint() failed\n");
            goto cleanup;
        }
        // chase_pointers(memory + (rand_offs % len), len);
    } while(time(NULL)-start < duration);

    ret = EXIT_SUCCESS;

cleanup:
    rng_close();
    if (memory) free(memory);

    return ret;
}