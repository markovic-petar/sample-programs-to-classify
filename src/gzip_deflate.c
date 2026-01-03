#if defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include "helper_io.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "zlib.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

int main(int argc, char **argv) {
    int ret = EXIT_FAILURE;
    
    /* Buffers */
    z_stream strm;
    unsigned char* in = NULL;
    size_t in_len;
    unsigned char* out = NULL;
    size_t out_len;

    /* Allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "DeflateInit failed.\n");
        goto cleanup;
    }

    /* Read input file and allocate input buffer */
    if (io_fread_to_char_buf("plaintext.txt", &in, &in_len)) {
        fprintf(stderr, "Fread failed.\n");
        goto cleanup;
    }

    /* Allocate output buffer */
    out_len = deflateBound(&strm, in_len);
    out = malloc(out_len);
    if (!out) {
        fprintf(stderr, "Malloc failed.\n");
        goto cleanup;
    }

    /* Extract time duration */
    time_t duration; size_t duration_argv_ix = 1;
    ret = io_time_from_args(argc, argv, &duration, duration_argv_ix);
    if (ret != IO_OK) {
        fprintf(stderr, "Invalid time argument.\n");
        goto cleanup;
    }

    /* Repeat the algorithm */
    time_t start_time = time(NULL);
    do {
        strm.avail_in = in_len;
        strm.next_in = in;

        strm.avail_out = out_len;
        strm.next_out = out;

        int zret = deflate(&strm, Z_FINISH);  /* no bad return value */
        if (zret != Z_STREAM_END) {
            fprintf(stderr, "Deflate failed.\n");
            goto cleanup;
        }

        /* Reset the stream for repetition */
        if (deflateReset(&strm) != Z_OK) {
            fprintf(stderr, "DeflateReset failed.\n");
            goto cleanup;
        }
    } while (time(NULL) - start_time < duration);

    size_t used_out = out_len - strm.avail_out;

    if(io_fwrite_from_char_buf("deflate.gz", out, used_out)) {
        fprintf(stderr, "Fwrite failed.\n");
        goto cleanup;
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (out) free(out);
    if (in) free(in);
    (void)deflateEnd(&strm);
    return ret;
}
