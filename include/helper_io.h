#ifndef PROG_EM_TEST_HELPER_INPUT
#define PROG_EM_TEST_HELPER_INPUT

#include <time.h>

#define DEFAULT_RUN_TIME_S 10 // s

typedef enum
{
    IO_OK = 0,
    IO_INPUT_ERR = -1,
    IO_OUTPUT_ERR = -2,
}
io_error_t;

io_error_t io_time_from_args(int argc, char* argv[], time_t* t, size_t argno);

io_error_t io_fread_to_char_buf(const char* file_name, unsigned char** buf, size_t* buf_len);

io_error_t io_fwrite_from_char_buf(const char* file_name, const unsigned char* buf, const size_t buf_len);

#endif
