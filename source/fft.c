#include "helper_io.h"

#include <fftw3.h>

#include <math.h>
#include <stdio.h>
#include <time.h>

#define pow 16
#define N (1<<pow)
#define PI 3.14159265358979323846

#define MAX_WIDTH 60     // max horizontal width in characters
#define MAX_HEIGHT 20    // max vertical height in stars

// Print FFT magnitudes as asterisks
void print_fft_magnitude(fftw_complex *fft_out, int n, int bin_factor) {
    // n = original real input length
    // fft_out = output of fftw_plan_dft_r2c_1d (size N/2 + 1)
    // bin_factor = how many FFT bins to combine per output column

    int out_bins = (n/2 + bin_factor - 1) / bin_factor; // ceil division
    double mags[out_bins];
    double max_mag = 0.0;

    // Compute magnitudes and bin them
    for (int i = 0; i < out_bins; i++) {
        double sum = 0.0;
        int start = i * bin_factor;
        int end = start + bin_factor;
        if (end > n/2 + 1) end = n/2 + 1;
        for (int j = start; j < end; j++) {
            double re = fft_out[j][0];
            double im = fft_out[j][1];
            sum += sqrt(re*re + im*im); // magnitude
        }
        mags[i] = sum / bin_factor; // average magnitude in bin
        // mags[i] = 20*log10(mags[i] + 1e-12);
        if (mags[i] > max_mag) max_mag = mags[i];
    }

    // Scale magnitudes to MAX_HEIGHT
    for (int i = 0; i < out_bins; i++) {
        int height = (int)(mags[i] / max_mag * MAX_HEIGHT + 0.5);
        if (height == 0) continue; // skip insignificant bins
        printf("%6d |", i*bin_factor);
        for (int j = 0; j < height; j++) printf("*");
        printf("\n");
    }
}

int main(int argc, char** argv) {
    /* Buffers */
    double* in = NULL;
    fftw_complex* out = NULL;
    fftw_plan plan = NULL;

    /* Allocate input/output buffers */
    in = fftw_alloc_real(N);
    if (!in) {
        fprintf(stderr, "fftw_malloc_real() failed\n");
        goto cleanup;
    }
    out = fftw_alloc_complex(N);
    if (!out) {
        fprintf(stderr, "fftw_malloc_complex() failed\n");
        goto cleanup;
    }

    /* Create plan */
    plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    if (!plan) {
        fprintf(stderr, "Plan creation failed\n");
        goto cleanup;
    }

    /* Load input data */
    double freq1 = 3.0;
    double freq2 = 5.0;
    double freq3 = 6.0;
    for (size_t i = 0; i < N; ++i) {
        // in[i] = sin(2 * PI * freq1 * i / N) + 5*sin(2 * PI * freq2 * i / N) + 2*sin(2 * PI * freq3 * i / N);
        in[i] = (i >= 1 && i <= 10) ? 1 : 0;
    }

    /* Input time duration */
    time_t duration = 0;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK) {
        fprintf(stderr, "Invalid time argument\n");
        goto cleanup;
    }

    /* Do the thing */
    time_t start = time(NULL);
    do {
        fftw_execute(plan);
    } while(time(NULL) - start < duration);

    /* Print magnitudes */
    print_fft_magnitude(out, N, 1<<10); // bin_factor = 2 for horizontal scaling

cleanup:
    if (plan) fftw_destroy_plan(plan);
    if (out) fftw_free(out);
    if (in) fftw_free(in);
    return 0;
}
