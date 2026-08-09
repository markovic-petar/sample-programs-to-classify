#include "helper_io.h"
#include "helper_rng.h"

#include <stdio.h>

int main()
{
    rng_open();
    double d = 0.0;
    rng_double(&d);
    printf("RNG double: %f\n", d);
    return 0;
}
