#include "config.h"
#include <driver/i2s.h>
#include <HTTPClient.h> // Required for ESP8266Audio
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define BUFFER_SIZE (2*256)

// TWATCH 2020 V3 PDM microphone pin
#define MIC_DATA            2
#define MIC_CLOCK           0

uint8_t buffer[BUFFER_SIZE] = {0};
TTGOClass *ttgo = nullptr;
lv_obj_t *chart = nullptr;
lv_chart_series_t *ser1 = nullptr;

void MicrophoneTask(void *pvParameters) {
    size_t read_len = 0;
    uint32_t j = 0;
    int16_t val1, val2, val16, val_max = 0, val_max_1 = 0, true_max;
    float val_avg = 0, val_avg_1 = 0;

    for (;;) {
        j++;
        i2s_read(I2S_NUM_0, (char *) buffer, BUFFER_SIZE, &read_len, portMAX_DELAY);
        for (int i = 0; i < BUFFER_SIZE / 2 ; i++) {
            val1 = buffer[i * 2];
            val2 = buffer[i * 2 + 1];
            val16 = val1 + val2 *  256;
            if (val16 > 0) {
                val_avg += val16;
                val_max = max(val_max, val16);
            } else {
                val_avg_1 += val16;
                val_max_1 = min(val_max_1, val16);
            }
        }
        Serial.write(buffer, read_len);

        if (j % 2 == 0) {
            val_avg = val_avg / (BUFFER_SIZE / 2);
            val_avg_1 = val_avg_1 / (BUFFER_SIZE / 2);
            true_max = max(val_max, (int16_t)-val_max_1);
            lv_chart_set_next(chart, ser1, true_max);
            val_avg = 0;
            val_max = 0;
            val_avg_1 = 0;
            val_max_1 = 0;
        }
        // lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void setup() {
    Serial.begin(921600);

    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
    ttgo->lvgl_begin();

    lv_obj_t *text = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(text, "PDM Microphone Test");
    lv_obj_align(text, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    chart = lv_chart_create(lv_scr_act(), NULL);
    lv_obj_set_size(chart, 200, 150);
    lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart, 0, 8192);
    ser1 = lv_chart_add_series(chart, LV_COLOR_RED);

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate =  44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    i2s_pin_config_t i2s_cfg;
    i2s_cfg.bck_io_num   = I2S_PIN_NO_CHANGE;
    i2s_cfg.ws_io_num    = MIC_CLOCK;
    i2s_cfg.data_out_num = I2S_PIN_NO_CHANGE;
    i2s_cfg.data_in_num  = MIC_DATA;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &i2s_cfg);
    i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

    xTaskCreatePinnedToCore(
        MicrophoneTask,   // Task function
        "MicrophoneTask", // Name of the task
        10000,            // Stack size in words
        NULL,             // Task input parameter
        1,                // Priority of the task
        NULL,             // Task handle
        0                 // Core where the task should run
    );
}

void loop() {
    // Keep the loop empty or add other non-microphone related tasks
    delay(10); // Small delay to prevent the loop from running too fast
}
