#pragma once
#include <dsps_fir.h>
// Define a global FIR filter structure
static fir_f32_t fir_filter;

void init_fir_filter(int16_t *buffer_speaker, size_t size);

void apply_fir_on_int16_input(int16_t *data_mic_cyclic, size_t data_length, float *);
