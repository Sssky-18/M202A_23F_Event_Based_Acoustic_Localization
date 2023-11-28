#include "config.h"
#include "driver/i2s.h"
#include <math.h>
#include <dsps_conv.h>
#include <freertos/FreeRTOS.h>
#include "init.h"

TTGOClass *ttgo;
int16_t buffer_speaker[bufferSize_speaker];
uint8_t buffer_mic[BUFFER_SIZE_MIC] = {0};
xTaskHandle micSerialTaskHandle;



void MicrophoneTask(void *pvParameters)
{
  int16_t *data_mic = new int16_t[DATA_SIZE_MIC];
  static int64_t data_sum = 0;
  static uint16_t data_mic_idx = 0;
  int64_t current_time, before_read_time, time_offset;
  uint32_t total_read_length = 0, read_length_offset = 0;

  size_t read_len = 0;
  int16_t val16;
  esp_timer_init();
  for (;;)
  {
    before_read_time = esp_timer_get_time();
    i2s_read(I2S_NUM_0, (char *)byte_buffer_mic, read_chunk_size_byte * 2, &read_len, portMAX_DELAY);
    for (int i = 0; i < read_len / 2; i++)
    {
      val16 = buffer_mic[i * 2] + buffer_mic[i * 2 + 1] * 256;
      val16 = ABS(val16);
      data_sum += val16;
      data_sum -= data_mic[data_mic_idx];
      data_mic[data_mic_idx] = val16;
      data_mic_idx++;
      if (data_mic_idx >= DATA_SIZE_MIC)
      {
        data_mic_idx = 0;
      }
    }
    if (total_read_length > 10000 && read_length_offset == 0)
    {
      read_length_offset = total_read_length;
      time_offset = esp_timer_get_time();
    }
    total_read_length += read_len / 2;
    current_time = esp_timer_get_time();
    // Serial.printf("Total read length %d: at time: %f, newly read %d,average_rate %f, last read used %f, average %f\n",
    //       total_read_length-read_length_offset,
    //       (current_time-time_offset) / 1e6,
    //       read_len,
    //       (float)(total_read_length-read_length_offset) * 1e6 / (current_time-time_offset),
    //       (current_time - before_read_time)/1e6,
    //       (float)data_sum/DATA_SIZE_MIC);
    xTaskNotify(micSerialTaskHandle, 0, eNoAction);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void SerialTransmissionTask(void *pvParameters)
{
  micSerialTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // Serial.write(byte_buffer_mic, read_chunk_size_byte * 2);
  }
}



void setup()
{
  Serial.begin(921600);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->enableLDO3();
  setup_gui();
  setup_speaker();
  setup_mic();
    xTaskCreatePinnedToCore(
      SerialTransmissionTask,   // Task function
      "SerialTransmissionTask", // Name of the task
      10000,                    // Stack size in words
      NULL,                     // Task input parameter
      1,                        // Priority of the task
      NULL,                     // Task handle
      0                         // Core where the task should run
  );
  xTaskCreatePinnedToCore(MicrophoneTask, "MicrophoneTask", 10000, NULL, 1, NULL, 1);
}

void loop()
{
  size_t bytesWritten = 0;
#ifdef DEBUG_OUTPUT
  i2s_write(i2sPort_speaker, (const char *)buffer_speaker, bufferSize_speaker * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
#endif

  vTaskDelay(pdMS_TO_TICKS(2000));
}