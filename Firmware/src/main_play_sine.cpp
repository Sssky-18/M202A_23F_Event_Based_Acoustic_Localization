#include "config.h"
#include "driver/i2s.h"
#include <math.h>
#include <dsps_conv.h>
#include <freertos/FreeRTOS.h>
#include "init.h"

TTGOClass *ttgo;
int16_t buffer_speaker[bufferSize_speaker];

xTaskHandle micSerialSendTaskHandle,micTimestampTaskHandle,speakerPlaySyncTaskHandle,micTasksHandle;
int16_t *data_mic_cyclic;
uint16_t data_mic_idx = 0;
uint64_t total_read_length = 0; 

auto sem_mic=xSemaphoreCreateMutex();


void MicrophoneTask(void *pvParameters)
{
  data_mic_cyclic = new int16_t[DATA_SIZE_MIC];
  uint8_t *byte_buffer_mic=new uint8_t[BUFFER_SIZE_MIC];
  static int64_t data_abs_sum = 0;
  int64_t current_time, before_read_time, time_offset;
  uint32_t read_length_offset = 0;
  size_t read_len = 0;
  int16_t val16;
  for (;;)
  {
    before_read_time = esp_timer_get_time();
    i2s_read(I2S_NUM_0, (char *)byte_buffer_mic, read_chunk_size_byte * 2, &read_len, portMAX_DELAY);
    total_read_length += read_len / 2;
    xSemaphoreTake(sem_mic, portMAX_DELAY);
    for (int i = 0; i < read_len / 2; i++)
    {
      val16 = byte_buffer_mic[i * 2] + byte_buffer_mic[i * 2 + 1] * 256;
      val16 = ABS(val16);
      data_abs_sum += val16;
      data_abs_sum -= data_mic_cyclic[data_mic_idx];
      data_mic_cyclic[data_mic_idx] = val16;
      data_mic_idx++;
      if (data_mic_idx == DATA_SIZE_MIC)
      {
        data_mic_idx = 0;
      }
    }
    xTaskNotify(micTasksHandle, 0, eNoAction);
    if (read_length_offset == 0 && total_read_length > 10000 )
    {
      read_length_offset = total_read_length;
      time_offset = esp_timer_get_time();
    }
    current_time = esp_timer_get_time();
    // Serial.printf("Total read length %d: at time: %f, newly read %d,average_rate %f, last read used %f, average %f\n",
    //       total_read_length-read_length_offset,
    //       (current_time-time_offset) / 1e6,
    //       read_len,
    //       (float)(total_read_length-read_length_offset) * 1e6 / (current_time-time_offset),
    //       (current_time - before_read_time)/1e6,
    //       (float)data_abs_sum/DATA_SIZE_MIC);
    // vTaskDelay(pdMS_TO_TICKS(40));
  }
}

void micTasksHub(void *pvParameters)
{
  micTasksHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xTaskNotify(micSerialSendTaskHandle, 0, eNoAction);
    xTaskNotify(micTimestampTaskHandle, 0, eNoAction);
  }

}

void SerialTransmissionTask(void *pvParameters)
{
  micSerialSendTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(sem_mic, portMAX_DELAY);
    // Send byte_buffer_mic through serial
    // Serial.write(byte_buffer_mic, read_chunk_size_byte * 2);
    xSemaphoreGive(sem_mic);
  }
}

void micTimestampTask(void *pvParameters)
{
  micTimestampTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(sem_mic, portMAX_DELAY);
    xSemaphoreGive(sem_mic);
  }
}

void create_semaphores(void)
{
  xSemaphoreGive(sem_mic);
}

void setup()
{
  esp_timer_init();
  create_semaphores();
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
  xTaskCreatePinnedToCore(micTasksHub, "micTasksHub", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(micTimestampTask, "micTimestampTask", 10000, NULL, 1, NULL, 1);
  xTaskCreate(SerialTransmissionTask, "SerialTransmissionTask", 10000, NULL, 1, NULL);
}

void loop()
{
  size_t bytesWritten = 0;
// #ifdef DEBUG_OUTPUT
//   i2s_write(i2sPort_speaker, (const char *)buffer_speaker, bufferSize_speaker * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
// #endif

  vTaskDelay(pdMS_TO_TICKS(2000));
}