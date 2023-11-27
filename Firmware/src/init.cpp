#include "init.h"
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
        // Serial.printf("%d ", buffer_speaker[i]);
    }
}

double sinc(double x) {
    if (x == 0.0) {
        return 1.0;
    } else {
        return sin(M_PI * x) / (M_PI * x);
    }
}

void setup_gui()
{
  ttgo->openBL();
  ttgo->setBrightness(25);
}