#include "config.h"
#include "driver/i2s.h"
#include <math.h>
#include <dsps_conv.h>
#include <freertos/FreeRTOS.h>

TTGOClass *ttgo;

const int sampleRate_speaker = 44100;                  
const int sineFreq = 8000;                             
const int bufferSize_speaker = sampleRate_speaker / 10;
const int gainShift = 4;                               
int16_t buffer_speaker[bufferSize_speaker];

const int sampleRate_mic = 44100;
const int BUFFER_TIME_MIC_MS = 100;
const uint32_t BUFFER_SIZE_MIC = 2 * (sampleRate_mic * BUFFER_TIME_MIC_MS / 1000);
uint8_t buffer_mic[BUFFER_SIZE_MIC] = {0};
xTaskHandle micTaskHandle;

// I2S port and pin configuration
const i2s_port_t i2sPort_speaker = I2S_NUM_1;
const i2s_port_t i2sPort_mic = I2S_NUM_0;
const int i2sBckPin = TWATCH_DAC_IIS_BCK;   // BCLK pin
const int i2sWsPin = TWATCH_DAC_IIS_WS;     // WCLK pin
const int i2sDoutPin = TWATCH_DAC_IIS_DOUT; // DOUT pin

void setup_speaker()
{
  i2s_config_t i2sConfig_speaker = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = sampleRate_speaker,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0};
  i2s_pin_config_t pinConfig_speaker = {
      .bck_io_num = i2sBckPin,
      .ws_io_num = i2sWsPin,
      .data_out_num = i2sDoutPin,
      .data_in_num = I2S_PIN_NO_CHANGE // Not using input
  };
  i2s_driver_install(i2sPort_speaker, &i2sConfig_speaker, 0, NULL);
  i2s_set_pin(i2sPort_speaker, &pinConfig_speaker);
  i2s_set_clk(i2sPort_speaker, sampleRate_speaker, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

  for (int i = 0; i < bufferSize_speaker; i++)
  {
    int16_t sample = (int16_t)(32767 * sin(2 * PI * sineFreq * i / sampleRate_speaker));
    buffer_speaker[i] = sample >> gainShift;
  }
}

void MicrophoneTask(void *pvParameters)
{
  int64_t current_time;
  uint32_t total_read_length = 0;
  size_t read_len = 0;
  uint32_t j = 0;
  int16_t val16;
  for (;;)
  {
    j++;
    i2s_read(I2S_NUM_0, (char *)buffer_mic, BUFFER_SIZE_MIC, &read_len, portMAX_DELAY);
    for (int i = 0; i < BUFFER_SIZE_MIC / 2; i++)
    {
      val16 = buffer_mic[i * 2] + buffer_mic[i * 2 + 1] * 256;
    }
    total_read_length += read_len;
    current_time = esp_timer_get_time();
    Serial.printf("Total read length %d: at time: %f, newly read %d,average_rate %d\n", total_read_length, current_time / 1e6, read_len, total_read_length * 1e3 / current_time * 1e3);

    xTaskNotify(micTaskHandle, 0, eNoAction);
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void SerialTransmissionTask(void *pvParameters)
{
  micTaskHandle = xTaskGetCurrentTaskHandle();
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // Send buffer_mic through serial
    // Serial.write(buffer_mic, BUFFER_SIZE_MIC);
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
      .dma_buf_count = 2,
      .dma_buf_len = 128,
  };

  i2s_pin_config_t i2s_cfg;
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
  // ttgo->lvgl_begin();

  // lv_obj_t *text = lv_label_create(lv_scr_act(), NULL);
  // lv_label_set_text(text, "PDM Microphone Test");
  // lv_obj_align(text, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

  // chart = lv_chart_create(lv_scr_act(), NULL);
  // lv_obj_set_size(chart, 200, 150);
  // lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);
  // lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  // lv_chart_set_range(chart, 0, 8192);
  // ser1 = lv_chart_add_series(chart, LV_COLOR_RED);
}

void setup()
{
  esp_timer_early_init();
  esp_timer_init();
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
  // size_t bytesWritten = 0;
  // i2s_write(i2sPort_speaker, (const char *)buffer_speaker, bufferSize_speaker * sizeof(int16_t), &bytesWritten, portMAX_DELAY);

  // Serial.print("Buffer played at: ");
  // Serial.print(millis() / 1000.0);
  // Serial.println(" seconds");

  delay(2000);
}