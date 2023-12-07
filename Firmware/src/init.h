#include "config.h"
#include <driver/i2s.h>
#include "dsp.h"
#include "math.h"
extern fir_f32_t fir_filter;
extern int16_t buffer_speaker[bufferSize_speaker];
extern TTGOClass *ttgo;
void setup_mic();
void setup_speaker();
double sinc(double x);
void setup_gui();