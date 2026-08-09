#include "helper_io.h"

#include "config.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static inline io_error_t parse_llong(const char str[], long long* val)
{
    // Test for NULL ptrs
    if (!str || !val)
        return IO_INPUT_ERR;

    char *endptr;
    errno = 0;

    *val = strtoll(str, &endptr, 10);
    
    if (
        endptr == str   ||  // empty string
        *endptr != '\0' ||  // trailing characters
        ((*val == LLONG_MIN || *val == LLONG_MAX) && errno == ERANGE)  // number is out of range
    )
        return IO_INPUT_ERR;

    return IO_OK;
}

static inline io_error_t parse_time_t(const char str[], time_t* t)
{
    long long tmp;
    io_error_t ret = parse_llong(str, &tmp);
    if (ret != IO_OK) return ret;

    // Check if cast is correct
    if ((time_t)tmp != tmp)
        return IO_INPUT_ERR;

    *t = (time_t)tmp;
    return IO_OK;
}

io_error_t io_time_from_args(int argc, char* argv[], time_t* t, size_t argv_ix)
{
    // Check for bad function args
    if (
        (argc <= argv_ix) ||
        !argv           ||
        !t
    )
        return IO_INPUT_ERR;

    // No run time provided
    // if (argc == 1)
    // {
    //     *t = DEFAULT_RUN_TIME_S;
    //     return IO_OK;
    // }

    // Check if argv[argv_ix] is a valid char ptr
    if (!argv[argv_ix])
        return IO_INPUT_ERR;

    io_error_t ret = parse_time_t(argv[argv_ix], t);
    if (ret != IO_OK) return ret;

    return IO_OK;
}

io_error_t io_fread_to_char_buf(const char* file_name, unsigned char** buf, size_t* buf_len)
{
    io_error_t ret = IO_INPUT_ERR;

    unsigned char* tmp_buf = NULL;
    FILE* input = NULL;

    /* Check input */
    if (!file_name || !buf_len)
    {
        goto cleanup;
    }
    /* Open decrypted data */
    input = fopen(file_name, "rb");
    if (!input)
    {
        goto cleanup;
    }
    /* Get the file contents' size */
    if (fseek(input, 0, SEEK_END))
    {
        goto cleanup;
    }
    long tmp_len = ftell(input);
    if (tmp_len < 0)
    {
        goto cleanup;
    }
    *buf_len = (size_t)tmp_len;
    /* if 'buf' is not NULL - malloc and read the contents into 'buf' */
    if (buf)
    {
        tmp_buf = malloc(sizeof(char)*(*buf_len));
        if (!tmp_buf)
        {
            goto cleanup;
        }
        /* Go back to file start */
        if (fseek(input, 0, SEEK_SET))
        {
            goto cleanup;
        }
        /* Read the entire file into memory. */
        size_t read_len = fread(tmp_buf, sizeof(char), *buf_len, input);
        if (read_len != *buf_len || ferror(input))
        {
            goto cleanup;
        }
        /* Assign the ptr */
        *buf = tmp_buf;
    }

    ret = IO_OK;

cleanup:
    if (input) fclose(input);
    if (ret && tmp_buf) free(tmp_buf);
    return ret;
}

io_error_t io_fwrite_from_char_buf(const char* file_name, const unsigned char* buf, const size_t buf_len)
{
    io_error_t ret = IO_OUTPUT_ERR;

    if (!file_name || !buf)
    {
        goto cleanup;
    }

    FILE* output = fopen(file_name, "wb");
    if (!output)
    {
        goto cleanup;
    }

    size_t fwrite_count = 0;
    fwrite_count = fwrite(buf, sizeof(char), buf_len, output);
    if (fwrite_count != buf_len)
    {
        goto cleanup;
    }

    ret = IO_OK;

cleanup:
    if (output) fclose(output);
    return ret;
}