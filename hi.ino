// Basic blink + serial + audio throughput sketch for audio node v1.1
// The sketch blinks the led and outputs the audio input from the onboard mic
// to the speaker.
//
// CAREFUL This might produce loud feedback between speaker and mic.
//
// 1. Install "esp32" by Espressif in the boards manager
// 2. Select "ESP32-S3-Box" as the board
//
// If upload does not work, try holding BOOT, pressing RST briefly and release
// BOOT, then try the upload again

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2s.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 32

int32_t samples[BUFFER_SIZE];

i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
  .sample_rate = uint32_t(SAMPLE_RATE),
  .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL3,
  .dma_buf_count = 3,
  .dma_buf_len = BUFFER_SIZE,
  .use_apll = true,
  .tx_desc_auto_clear = false,
  .fixed_mclk = 0
};

i2s_pin_config_t i2s_mic_pins = {
  .bck_io_num = GPIO_NUM_3,
  .ws_io_num = GPIO_NUM_2,
  .data_out_num = I2S_PIN_NO_CHANGE,
  .data_in_num = GPIO_NUM_4
};

i2s_pin_config_t i2s_amp_pins = {
  .bck_io_num = GPIO_NUM_6, // sck
  .ws_io_num = GPIO_NUM_7, // ws
  .data_out_num = GPIO_NUM_5, // sd
  .data_in_num = I2S_PIN_NO_CHANGE
};

void setup() {
  Serial.begin(115200);
  pinMode(GPIO_NUM_21, OUTPUT);

  delay(500);
  Serial.println("hi");

  // audio setup
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);
  i2s_set_pin(I2S_NUM_0, &i2s_amp_pins);

  xTaskCreate(audioTask, "audioTask", 4096, NULL, 5, NULL);
}

void loop() {
  digitalWrite(GPIO_NUM_21, HIGH);
  delay(500);
  digitalWrite(GPIO_NUM_21, LOW);
  delay(500);
}

static void audioTask(void*) {
  while(1) {
    size_t bytes_read = 0;
    size_t bytes_written = 0;

    i2s_read(I2S_NUM_0, samples, BUFFER_SIZE, &bytes_read, portMAX_DELAY);
    i2s_write(I2S_NUM_0, samples, BUFFER_SIZE, &bytes_written, portMAX_DELAY);
  }
}
