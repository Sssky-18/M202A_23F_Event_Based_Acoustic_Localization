#include "config.h"
#include "driver/i2s.h"
#include <math.h>
#include <freertos/FreeRTOS.h>
#include "init.h"
#include "esp_log.h"
#include "mywifi.h"
#include "dsp.h"
#define DEVICE_ID 0
extern fir_f32_t fir_filter;

TTGOClass *ttgo;
int16_t buffer_speaker[bufferSize_speaker];

xTaskHandle micSerialSendTaskHandle, micTimestampTaskHandle, speakerPlaySyncTaskHandle, micTasksHandle, micPostTimestampTaskHandle, speakerTaskHandle;
int16_t *data_mic_cyclic;
int32_t *data_moving_average_cyclic;
uint8_t *byte_buffer_mic = new uint8_t[BUFFER_SIZE_MIC];
uint16_t data_mic_idx = 0;
int running_read_length = 0;

const float noise_reject_ratio = 50;
// uint64_t data_energy_sum = 0;
const uint64_t data_energy_sum = 32151984;

bool doing_sync = false;
size_t sync_idx = 0;
size_t *sync_ts= new size_t[TOTAL_NODES];

auto sem_mic = xSemaphoreCreateMutex();

bool interesting_task_cd = false, interesting_task_detected = false;
TimerHandle_t reset_interesting_task_cooldown_timer;

bool eventTimeStampAvailable = false;
uint64_t eventTimeStamp = 0;
HTTPClientManager *httpClient = nullptr;

void MicrophoneTask(void *pvParameters)
{
  int16_t val16;
  data_mic_cyclic = new int16_t[DATA_SIZE_MIC]{0};
  data_moving_average_cyclic = new int32_t[DATA_SIZE_MIC];
  int64_t current_time, before_read_time, time_offset;
  uint32_t read_length_offset = 0;
  size_t read_len, argmax = 0, start_copy_idx = 0;
  static bool triggered_last_time=false;
  uint16_t new_read_max_abs = 0;
  bool bootstrap_complete = false;
  for (;;)
  {
    before_read_time = esp_timer_get_time();
    new_read_max_abs = 0;
    i2s_read(i2sPort_mic, (char *)byte_buffer_mic, read_chunk_size_byte * 2, &read_len, portMAX_DELAY);
    xSemaphoreTake(sem_mic, portMAX_DELAY);
    start_copy_idx = data_mic_idx;
    for (int i = 0; i < read_len / 2; i++)
    {
      val16 = byte_buffer_mic[i * 2] + byte_buffer_mic[i * 2 + 1] * 256 - MIC_OFFSET;
      if (ABS(val16) > new_read_max_abs)
      {
        new_read_max_abs = ABS(val16);
        argmax = i;
      }
      // data_energy_sum =data_energy_sum+ (int64_t)val16 *(int64_t) val16 - (int64_t)data_mic_cyclic[data_mic_idx] *(int64_t) data_mic_cyclic[data_mic_idx];
      data_mic_cyclic[data_mic_idx] = val16;
      data_mic_idx++;
      if (data_mic_idx == DATA_SIZE_MIC)
      {
        // to do: call filter
        data_mic_idx = 0;
        start_copy_idx = 0;
      }
    }
    // Serial.printf("Readlen %d val16 %d Data energy sum %lld\n",read_len,val16,data_energy_sum);
    xSemaphoreGive(sem_mic);
    if (!bootstrap_complete) // not enough data accumulated
    {
      read_length_offset = running_read_length;
      time_offset = esp_timer_get_time();
      bootstrap_complete = true;
#ifndef DEBUG_OUTPUT
      Serial.println("Bootstrap Complete");
#endif
    }
    else
    {
      // start processing
      if (doing_sync)
      {
        if ((float)(uint64_t)new_read_max_abs * new_read_max_abs * DATA_SIZE_MIC >= data_energy_sum * 40) // use a different noise reject ratio
        {

        if (!triggered_last_time)  // prevent continuous trigger
        {
    #ifndef DEBUG_OUTPUT
          Serial.printf("Sync event detected at stamp: %ld, max energy %ld, average %f\n",
                        running_read_length + argmax,
                        (uint32_t)new_read_max_abs * new_read_max_abs,
                        (float)data_energy_sum / DATA_SIZE_MIC);
    #endif
          sync_ts[sync_idx] = running_read_length + argmax;
          sync_idx++;
          if (sync_idx == TOTAL_NODES)
          {
            doing_sync = false;
            sync_idx = 0; // finished sync sequence capture
            xTaskNotify(micPostTimestampTaskHandle, 0, eNoAction);
          }
        }
        triggered_last_time=true;
        }
        else
        {
          triggered_last_time=false;
        }
      }
      else if ((float)(uint64_t)new_read_max_abs * new_read_max_abs * DATA_SIZE_MIC >= data_energy_sum * 50) // interesting event detected
        if (!interesting_task_cd)
        {
          interesting_task_detected = true;
          interesting_task_cd = true;
          xTimerStart(reset_interesting_task_cooldown_timer, 0);
          eventTimeStamp = running_read_length + argmax;
            
#ifndef DEBUG_OUTPUT
          Serial.printf("Interesting event detected at stamp: %ld, max energy %ld, average %f\n",
                        running_read_length + argmax,
                        (uint32_t)new_read_max_abs * new_read_max_abs,
                        (float)data_energy_sum / DATA_SIZE_MIC);
#endif
#ifdef USE_NAIVE_TIMESTAMP
          eventTimeStampAvailable = true;
#endif
xTaskNotify(micTasksHandle, 0, eNoAction);
        }
        else
        {
#ifndef DEBUG_OUTPUT
          Serial.printf("Interesting event detected but on cooldown at time: %ld, max energy %ld, average %f\n",
                        running_read_length + argmax,
                        (uint32_t)new_read_max_abs * new_read_max_abs,
                        (float)data_energy_sum / DATA_SIZE_MIC);
#endif
        }
    }
      current_time = esp_timer_get_time();
  running_read_length += read_len / 2;
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
#ifdef DEBUG_OUTPUT
    xSemaphoreTake(sem_mic, portMAX_DELAY);
    Serial.write(byte_buffer_mic, read_chunk_size_byte * 2);
    xSemaphoreGive(sem_mic);
#endif
  }
}

void micTimestampTaskNaive(void *pvParameters)
{
  micTimestampTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (interesting_task_detected) // not necessary under this context, but this flag is important for micTimestampTaskFull
    {
      interesting_task_detected = false;
      xTaskNotify(micPostTimestampTaskHandle, 0, eNoAction);
    }
  }
}

void micPostTimestampTask(void *pvParameters)
{
  uint64_t event_timestamp_processing = 0; // buffer this in case of it being overwritten
  static uint8_t device_id = DEVICE_ID;
  micPostTimestampTaskHandle = xTaskGetCurrentTaskHandle();
  vTaskDelay(pdMS_TO_TICKS(3000));
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    eventTimeStampAvailable = false;
    event_timestamp_processing = eventTimeStamp;
    Serial.println("Syncronization started");
    sync_idx=0;
    doing_sync = true;
    // start syncronization
    vTaskDelay(pdMS_TO_TICKS(SYNC_OFFSET_MS-50));
    vTaskDelay(pdMS_TO_TICKS(SYNC_OFFSET_PRE_DEVICE_MS * DEVICE_ID));

    xTaskNotify(speakerTaskHandle, 0, eNoAction); //start sync
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(COOLDOWN_TIME_MS));
    if (doing_sync)
    {
      doing_sync = false;
      sync_idx = 0;
      continue;
    }
    // start sending timestamp
    if (WiFi.status() != WL_CONNECTED) {
        continue;
    }
    httpClient->postinfo(device_id, event_timestamp_processing, sync_ts);
  }
}

void speakerTask(void *pvParameters)
{
  size_t bytesWritten = 0;
  speakerTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    #ifndef DEBUG_OUTPUT
    ESP_LOGI("speakerTask", "Speaker playing");
    #endif
    i2s_write(i2sPort_speaker, (const char *)buffer_speaker, bufferSize_speaker * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  }
}

// void micTimestampTaskFull(void *pvParameters)
// {
//   bool flag = false;
//   const static size_t internal_buffer_size = read_chunk_size_byte / 2 * 3;
//   int16_t internal_buffer[internal_buffer_size]; // exactly 3 periods of reading, roughly 60ms
//   micTimestampTaskHandle = xTaskGetCurrentTaskHandle();
//   for (;;)
//   {
//     ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//     if (!(interesting_task_detected || flag))
//     {
//       if (interesting_task_detected) // start delay
//       {
//         flag = true;
//         interesting_task_detected = false;
//       }
//       else // start processing
//       {
//         flag = false;
//         xSemaphoreTake(sem_mic, portMAX_DELAY);
//         if (data_mic_idx >= internal_buffer_size)
//         {
//           // Wrap back to the beginning of the internal buffer
//           size_t remaining_elements = internal_buffer_size - data_mic_idx;
//           memcpy(internal_buffer, &data_mic_cyclic[data_mic_idx - internal_buffer_size], remaining_elements * sizeof(int16_t));
//           memcpy(&internal_buffer[remaining_elements], data_mic_cyclic, (internal_buffer_size - remaining_elements) * sizeof(int16_t));
//         }
//         else
//         {
//           size_t elements_before_idx = internal_buffer_size - data_mic_idx;
//           memcpy(internal_buffer, &data_mic_cyclic[0], data_mic_idx * sizeof(int16_t));
//           memcpy(&internal_buffer[data_mic_idx], &data_mic_cyclic[DATA_SIZE_MIC - elements_before_idx], elements_before_idx * sizeof(int16_t));
//         }
//         // To do: find timestamp
//         xTaskNotify(micPostTimestampTaskHandle, 0, eNoAction);
//         xSemaphoreGive(sem_mic);
//       }
//       continue;
//     }
//   }
// }

void create_event_handles(void)
{
  xSemaphoreGive(sem_mic);
  reset_interesting_task_cooldown_timer = xTimerCreate("reset_interesting_task_cooldown_timer", pdMS_TO_TICKS(COOLDOWN_TIME_MS), pdFALSE, (void *)0, [](TimerHandle_t xTimer)
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
  xTaskCreatePinnedToCore(SerialTransmissionTask, "SerialTransmissionTask", 10000, NULL, 1, NULL, 0);
  xTaskCreate(startup_wifi_task, "startup_wifi_task", 10000, NULL, 1, NULL);
  xTaskCreatePinnedToCore(MicrophoneTask, "MicrophoneTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(micTasksHub, "micTasksHub", 10000, NULL, 1, NULL, 0);
#ifndef USE_NAIVE_TIMESTAMP
  xTaskCreatePinnedToCore(micTimestampTaskFull, "micTimestampTask", 10000, NULL, 1, NULL, 1);
#else
  xTaskCreatePinnedToCore(micTimestampTaskNaive, "micTimestampTask", 10000, NULL, 1, NULL, 0);
#endif
  xTaskCreatePinnedToCore(micPostTimestampTask, "micPostTimestampTask", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(speakerTask, "speakerTask", 10000, NULL, 1, NULL, 0);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(2000));
  // i2s_write(i2sPort_speaker, (const char *)buffer_speaker, bufferSize_speaker * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
}