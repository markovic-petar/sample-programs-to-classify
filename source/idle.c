#include "config.h"
#include "helper_io.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    // extract run duration
    time_t duration;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK)
    {
        // bad input
        fprintf(stderr, PROGRAM_NAME ": Input error. Correct usage: %s [seconds]\n", argv[0]);
        return -1;
    }

    // perform task for the given duration
    sleep(duration);

    return 0;
}
