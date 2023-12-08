
#ifndef CONFIG_H // include guard
#define CONFIG_H
#include "device.h"
#define MIC_DATA            2
#define MIC_CLOCK           0
#define i2sPort_speaker  I2S_NUM_1
#define i2sPort_mic  I2S_NUM_0
#define i2sBckPin TWATCH_DAC_IIS_BCK   // BCLK pin
#define i2sWsPin  TWATCH_DAC_IIS_WS     // WCLK pin
#define i2sDoutPin TWATCH_DAC_IIS_DOUT // DOUT pin

#define speaker_duration_ms 100
#define sampleRate_speaker  44100
#define bufferSize_speaker  (sampleRate_speaker * speaker_duration_ms / 1000)
#define START_FREQ 2000.0
#define END_FREQ 8000.0
#define chirpRate ((END_FREQ - START_FREQ) / (speaker_duration_ms / 1000.0))

#define sampleRate_mic  44100
#define BUFFER_TIME_MIC_MS  100
#define BUFFER_SIZE_MIC  (2 * (sampleRate_mic * BUFFER_TIME_MIC_MS / 1000))

#define read_chunk_size_byte  2500          // about 20ms of data
#define DATA_SIZE_MIC  (sampleRate_mic / 5) // 200ms of data, must be at least 3x read_chunk_size_byte

#define WIFISSID "Acoustic"
#define WIFIPWD "12345670"
#define COOLDOWN_TIME_MS 1000
#define SYNC_OFFSET_PRE_DEVICE_MS 100
#define SYNC_OFFSET_MS 200

#define MIC_OFFSET (-1550)

#define SERVER_URL "http://192.168.137.249:5000/post_json"

#endif
