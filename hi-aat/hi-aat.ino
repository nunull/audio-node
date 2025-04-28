// Basic audio throughput sketch for audio node v1.1
// The sketch outputs the audio input from the onboard mic to the speaker.
//
// CAREFUL This might produce loud feedback between speaker and mic.
//
// 1. Install https://github.com/pschatzmann/arduino-audio-tools
// 2. Install "esp32" by Espressif in the boards manager
// 3. Select "ESP32-S3-Box" as the board
//
// If upload does not work, try holding BOOT, pressing RST briefly and release
// BOOT, then try the upload again.
//
// Connect a speaker to the board.

#include "AudioTools.h"

uint8_t channels = 2;
uint16_t sample_rate = 44100;
uint8_t bits_per_sample = 32;

I2SStream mic;
I2SStream amp;
StreamCopy copier(amp, mic);

void setup() {
  Serial.begin(115200);

  auto micConfig = mic.defaultConfig(RX_MODE);

  micConfig.pin_bck = GPIO_NUM_3;
  micConfig.pin_ws = GPIO_NUM_2;
  micConfig.pin_data = GPIO_NUM_4;

  micConfig.sample_rate = sample_rate;
  micConfig.channels = channels;
  micConfig.bits_per_sample = bits_per_sample;

  mic.begin(micConfig);

  auto ampConfig = amp.defaultConfig(TX_MODE);

  ampConfig.pin_bck = GPIO_NUM_6;
  ampConfig.pin_ws = GPIO_NUM_7;
  ampConfig.pin_data = GPIO_NUM_5;
  
  ampConfig.sample_rate = sample_rate;
  ampConfig.channels = channels;
  ampConfig.bits_per_sample = bits_per_sample;
  
  amp.begin(ampConfig);
}

void loop() {
  copier.copy();
}