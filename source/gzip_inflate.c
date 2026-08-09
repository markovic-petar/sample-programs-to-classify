/* zpipe.c: example of proper use of zlib's inflate() and deflate()
   Not copyrighted -- provided to the public domain
   Version 1.4  11 December 2005  Mark Adler */

/* Version history:
   1.0  30 Oct 2004  First version
   1.1   8 Nov 2004  Add void casting for unused return values
                     Use switch statement for inflate() return values
   1.2   9 Nov 2004  Add assertions to document zlib guarantees
   1.3   6 Apr 2005  Remove incorrect assertion in inf()
   1.4  11 Dec 2005  Add hack to avoid MSDOS end-of-line conversions
                     Avoid some compiler warnings for input and output buffers
 */

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
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        goto cleanup;

    /* Read input file */
    if (io_fread_to_char_buf("deflate.gz", &in, &in_len)) {
        fprintf(stderr, PROGRAM_NAME ": " "Fread failed.\n");
        goto cleanup;
    }

    /* Read the uncompressed file size into 'out_len' */
    if (io_fread_to_char_buf("plaintext.txt", NULL, &out_len)) {
        fprintf(stderr, PROGRAM_NAME ": " "Fread len. failed.\n");
        goto cleanup;
    }

    /* Allocate output buffer */;
    out = malloc(out_len);
    if (!out) {
        fprintf(stderr, PROGRAM_NAME ": " "Malloc failed.\n");
        goto cleanup;
    }

    /* extract time duration */
    time_t duration; size_t duration_argv_ix = 1;
    ret = io_time_from_args(argc, argv, &duration, duration_argv_ix);
    if (ret != IO_OK) {
        fprintf(stderr, PROGRAM_NAME ": " "Invalid time argument.\n");
        goto cleanup;
    }

    /* Repeat the algorithm */
    time_t start_time = time(NULL);
    do {
        strm.avail_in = in_len;
        strm.next_in = in;

        strm.avail_out = out_len;
        strm.next_out = out;

        int zret = inflate(&strm, Z_FINISH);
        if (zret != Z_STREAM_END) {
            fprintf(stderr, PROGRAM_NAME ": " "Inflate failed.\n");
            goto cleanup;
        }

        /* Reset the stream for repetition */
        if (inflateReset(&strm) != Z_OK)  {
            fprintf(stderr, PROGRAM_NAME ": " "InflateReset failed.\n");
            goto cleanup;
        }
    } while (time(NULL) - start_time < duration);

    size_t used_out = out_len - strm.avail_out;
    if(io_fwrite_from_char_buf("inflate.txt", out, used_out)) {
        fprintf(stderr, PROGRAM_NAME ": " "Fwrite failed.\n");
        goto cleanup;
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (out) free(out);
    if (in) free(in);
    (void)inflateEnd(&strm);
    return ret;
}
