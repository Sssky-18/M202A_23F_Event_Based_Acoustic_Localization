#include "config.h"
#include "driver/i2s.h"
#include <math.h>
#include <dsps_conv.h>
#include <freertos/FreeRTOS.h>
#include "init.h"

TTGOClass *ttgo;
int16_t buffer_speaker[bufferSize_speaker];

xTaskHandle micSerialSendTaskHandle, micTimestampTaskHandle, speakerPlaySyncTaskHandle, micTasksHandle;
int16_t *data_mic_cyclic;
uint16_t data_mic_idx = 0;
uint64_t total_read_length = 0;

const float noise_reject_ratio = 1.5;
uint64_t data_energy_sum = 0, data_energy_avg = 0;

auto sem_mic = xSemaphoreCreateMutex();

bool interesting_task_cd = true, interesting_task_detected = false;
TimerHandle_t reset_interesting_task_cooldown_timer;

bool eventTimeStampAvailable = false;
uint64_t eventTimeStamp = 0;

void MicrophoneTask(void *pvParameters)
{
  int16_t val16;
  data_mic_cyclic = new int16_t[DATA_SIZE_MIC];
  uint8_t *byte_buffer_mic = new uint8_t[BUFFER_SIZE_MIC];
  int64_t current_time, before_read_time, time_offset;
  uint32_t read_length_offset = 0;
  size_t read_len = 0;
  uint16_t new_read_max_abs = 0;
  bool bootstrap_complete = false;
  for (;;)
  {
    before_read_time = esp_timer_get_time();
    new_read_max_abs = 0;
    i2s_read(I2S_NUM_0, (char *)byte_buffer_mic, read_chunk_size_byte * 2, &read_len, portMAX_DELAY);
    total_read_length += read_len / 2;
    xSemaphoreTake(sem_mic, portMAX_DELAY);
    for (int i = 0; i < read_len / 2; i++)
    {
      val16 = byte_buffer_mic[i * 2] + byte_buffer_mic[i * 2 + 1] * 256;
      if (ABS(val16) > new_read_max_abs)
      {
        new_read_max_abs = ABS(val16);
      }
      data_energy_sum += val16 * val16;
      data_energy_sum -= data_mic_cyclic[data_mic_idx] * data_mic_cyclic[data_mic_idx];
      data_mic_cyclic[data_mic_idx] = val16;
      data_mic_idx++;
      if (data_mic_idx == DATA_SIZE_MIC)
      {
        data_mic_idx = 0;
      }
    }
    xTaskNotify(micTasksHandle, 0, eNoAction);
    if (!bootstrap_complete) // not enough data accumulated
    {
      read_length_offset = total_read_length;
      time_offset = esp_timer_get_time();
      bootstrap_complete = true;
    }
    else // start processing
    {
      if (!interesting_task_cd)
        if ((uint64_t)new_read_max_abs * new_read_max_abs * read_len / 2 >= data_energy_sum * noise_reject_ratio) // interesting event detected
        {
          interesting_task_detected = true;
          interesting_task_cd = true;
          xTimerStart(reset_interesting_task_cooldown_timer, 0);
#ifdef DEBUG_OUTPUT
          Serial.printf("Interesting event detected at time: %f, max_abs %d, average %f\n",
                        (current_time - time_offset) / 1e6,
                        new_read_max_abs,
                        (float)data_energy_sum / DATA_SIZE_MIC);
#endif
        }
    }
    current_time = esp_timer_get_time();
    // Serial.printf("Total read length %d: at time: %f, newly read %d,average_rate %f, last read used %f, average %f\n",
    //       total_read_length-read_length_offset,
    //       (current_time-time_offset) / 1e6,
    //       read_len,
    //       (float)(total_read_length-read_length_offset) * 1e6 / (current_time-time_offset),
    //       (current_time - before_read_time)/1e6,
    //       (float)data_energy_sum/DATA_SIZE_MIC);
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

void micTimestampTaskNaive(void *pvParameters)
{
  micTimestampTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (interesting_task_detected)  // not necessary under this context, but this flag is important for micTimestampTaskComplete
    {
      interesting_task_detected=false;
    }
  }

}

void micTimestampTaskComplete(void *pvParameters)
{
  bool flag = false;
  const static size_t internal_buffer_size = read_chunk_size_byte / 2 * 3;
  int16_t internal_buffer[internal_buffer_size]; // exactly 3 periods of reading, roughly 60ms
  micTimestampTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (!(interesting_task_detected || flag))
    {
      if (interesting_task_detected) // start delay
      {
        flag = true;
        interesting_task_detected = false;
      }
      else // start processing
      {
        flag = false;
        xSemaphoreTake(sem_mic, portMAX_DELAY);
        if (data_mic_idx >= internal_buffer_size)
        {
          // Wrap back to the beginning of the internal buffer
          size_t remaining_elements = internal_buffer_size - data_mic_idx;
          memcpy(internal_buffer, &data_mic_cyclic[data_mic_idx - internal_buffer_size], remaining_elements * sizeof(int16_t));
          memcpy(&internal_buffer[remaining_elements], data_mic_cyclic, (internal_buffer_size - remaining_elements) * sizeof(int16_t));
        }
        else
        {
          size_t elements_before_idx = internal_buffer_size - data_mic_idx;
          memcpy(internal_buffer, &data_mic_cyclic[0], data_mic_idx * sizeof(int16_t));
          memcpy(&internal_buffer[data_mic_idx], &data_mic_cyclic[DATA_SIZE_MIC - elements_before_idx], elements_before_idx * sizeof(int16_t));
        }
        xSemaphoreGive(sem_mic);
      }
      continue;
    }
  }
}

void create_event_handles(void)
{
  xSemaphoreGive(sem_mic);
  reset_interesting_task_cooldown_timer = xTimerCreate("reset_interesting_task_cooldown_timer", pdMS_TO_TICKS(100), pdFALSE, (void *)0, [](TimerHandle_t xTimer)
                                                       { interesting_task_cd = false; });
}

void setup()
{
  esp_timer_init();
  create_event_handles();
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
  xTaskCreatePinnedToCore(micTimestampTaskComplete, "micTimestampTask", 10000, NULL, 1, NULL, 1);
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