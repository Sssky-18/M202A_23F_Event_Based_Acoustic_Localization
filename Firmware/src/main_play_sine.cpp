#include "config.h"
#include "driver/i2s.h"
#include <math.h>
#include <dsps_conv.h>
#include <freertos/FreeRTOS.h>

TTGOClass *ttgo;
int16_t buffer_speaker[bufferSize_speaker];
uint8_t buffer_mic[BUFFER_SIZE_MIC] = {0};
xTaskHandle micSerialTaskHandle;

double sinc(double x) {
    if (x == 0.0) {
        return 1.0;
    } else {
        return sin(M_PI * x) / (M_PI * x);
    }
}

void setup_speaker()
{
  i2s_config_t i2sConfig_speaker = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = sampleRate_speaker,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 10,
      .dma_buf_len = 512,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0};
  i2s_pin_config_t pinConfig_speaker = {
      .bck_io_num = i2sBckPin,
      .ws_io_num = i2sWsPin,
      .data_out_num = i2sDoutPin,
      // .data_in_num = I2S_PIN_NO_CHANGE // Not using input
  };
  i2s_driver_install(i2sPort_speaker, &i2sConfig_speaker, 0, NULL);
  i2s_set_pin(i2sPort_speaker, &pinConfig_speaker);
  i2s_set_clk(i2sPort_speaker, sampleRate_speaker, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

  float Cutoff=8000.0;
  double sincValue;
    for (int i = 0; i < bufferSize_speaker; i++) {
        double x = (i - bufferSize_speaker / 2.0)/ sampleRate_speaker;
        sincValue = sinc(x * Cutoff);
        buffer_speaker[i] = (int16_t)(sincValue * 32767.0);
        Serial.printf("%d ", buffer_speaker[i]);
    }
}

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
    i2s_read(I2S_NUM_0, (char *)buffer_mic, read_chunk_size * 2, &read_len, portMAX_DELAY);

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
    // Send buffer_mic through serial
    // Serial.write(buffer_mic, read_chunk_size * 2);
  }
}

void setup_mic()
{
  i2s_config_t i2s_config_MIC = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
      .sample_rate = 44100,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 6,
      .dma_buf_len = 882,
  };
  i2s_pin_config_t i2s_cfg;
  i2s_cfg.mck_io_num = I2S_PIN_NO_CHANGE;
  i2s_cfg.bck_io_num = I2S_PIN_NO_CHANGE;
  i2s_cfg.ws_io_num = MIC_CLOCK;
  i2s_cfg.data_out_num = I2S_PIN_NO_CHANGE;
  i2s_cfg.data_in_num = MIC_DATA;

  i2s_driver_install(i2sPort_mic, &i2s_config_MIC, 0, NULL);
  i2s_set_pin(i2sPort_mic, &i2s_cfg);
  i2s_set_clk(i2sPort_mic, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

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

void setup_gui()
{
  ttgo->openBL();
  ttgo->setBrightness(25);
}

void setup()
{
  // esp_timer_early_init();
  // esp_timer_init();
  Serial.begin(921600);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->enableLDO3();
  setup_gui();
  setup_speaker();
  setup_mic();
}

void loop()
{
  size_t bytesWritten = 0;
#ifdef DEBUG_OUTPUT
  i2s_write(i2sPort_speaker, (const char *)buffer_speaker, bufferSize_speaker * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
#endif

  vTaskDelay(pdMS_TO_TICKS(2000));
}