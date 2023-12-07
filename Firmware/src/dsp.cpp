#include "dsp.h"
void init_fir_filter(int16_t *buffer_speaker, size_t size) {
    // Allocate memory for FIR coefficients in float
    float *fir_coeffs = (float *)malloc(sizeof(float) * size);

    // Convert coefficients from int16_t to float
    for (int i = 0; i < size; ++i) {
        fir_coeffs[i] = (float)buffer_speaker[i];
    }

    // Allocate memory for delay line
    float *delay_line = (float *)calloc(size, sizeof(float));

    // Initialize the FIR filter
    dsps_fird_init_f32(&fir_filter, fir_coeffs, delay_line, size, 1);

    // Free the coefficients array as it's now copied to the FIR structure
    free(fir_coeffs);
}

void apply_fir_on_int16_input(int16_t *data_mic_cyclic, size_t data_length, float *out) {
    // Allocate memory for the temporary float input
    float *input_float = (float *)malloc(sizeof(float) * data_length);

    // Convert the input data from int16_t to float
    for (int i = 0; i < data_length; ++i) {
        input_float[i] = (float)data_mic_cyclic[i];
    }

    // Apply the FIR filter
    dsps_fird_f32(&fir_filter, input_float, out, data_length);

    // Free the temporary float array
    free(input_float);
}
