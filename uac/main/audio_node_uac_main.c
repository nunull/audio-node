#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "usb_device_uac.h"
// esp-idf/examples/peripherals/i2s/i2s_basic/i2s_std/main/i2s_std_example_main.c
#include "driver/i2s_std.h"

#include "sdkconfig.h"

static const char *TAG = "audio_node";

static i2s_chan_handle_t mic_chan;
static i2s_chan_handle_t amp_chan;

// https://docs.espressif.com/projects/esp-iot-solution/en/latest/usb/usb_device/usb_device_uac.html
// https://github.com/espressif/esp-iot-solution/blob/cf4f5a654ad37a5e7a5fa9e5d8dc17ae5b88748d/examples/usb/device/usb_uac/main/usb_uac_main.c

// ICS-43434 datasheet: https://invensense.tdk.com/wp-content/uploads/2016/02/DS-000069-ICS-43434-v1.2.pdf
// MAX98357A datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/max98357a-max98357b.pdf

static esp_err_t uac_device_output_cb(uint8_t *buf, size_t len, void *arg)
{
    // swap MSB and LSB
    // for (size_t i = 0; i < len; i += 2) {
    //     uint8_t temp = buf[i];
    //     buf[i] = buf[i + 1];
    //     buf[i + 1] = temp;
    // }

    for (size_t i = 0; i < len; i++) {
        // buf[i] = buf[i] >> 2;
    }

    size_t bytes_written = 0;
    // bsp_extra_i2s_write(buf, len, &bytes_written, 0);
    i2s_channel_write(amp_chan, buf, len, &bytes_written, 40);
    // i2s_channel_write(amp_chan, buf, len, &bytes_written, 0);
    return ESP_OK;
}

static esp_err_t uac_device_input_cb(uint8_t *buf, size_t len, size_t *bytes_read, void *arg)
{
    // if (bsp_extra_i2s_read(buf, len, bytes_read, 0) != ESP_OK) {

    // uint24_t *buf_read = (uint24_t *)calloc(len, sizeof(uint24_t));
    uint8_t buf_read[len];

    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2s.html
    // if (i2s_channel_read(mic_chan, buf, len, bytes_read, 1000) != ESP_OK) {
    // if (i2s_channel_read(mic_chan, buf_read, len*sizeof(uint24_t), bytes_read, 1000) != ESP_OK) {
    if (i2s_channel_read(mic_chan, buf_read, len, bytes_read, 1000) != ESP_OK) {
    // if (i2s_channel_read(mic_chan, buf, len, bytes_read, 1000) != ESP_OK) {
        ESP_LOGE(TAG, "i2s read failed");
    }

    // int samples_read = bytes_read / sizeof(int32_t);

    // // swap MSB and LSB
    // for (int i = 0; i < *bytes_read; i += 2) {
    //     // buf[i] = buf[i] / 10;
    //     uint8_t temp = buf[i];
    //     buf[i] = buf[i + 1];
    //     buf[i + 1] = temp;
    // }

    // for (size_t i = 0; i < *bytes_read; i += 2) {
    //     // buf[i] = buf[i] / 10;
    //     uint16_t temp = buf_read[i];
    //     buf_read[i] = buf_read[i + 1];
    //     buf_read[i + 1] = temp;
    // }

    int32_t *samples_32 = (int32_t *)buf_read;
    int16_t *samples_16 = (int16_t *)buf;
    size_t num_samples = *bytes_read / 4; // divide by 4 to convert from 32 bit, divide by two to convert from stereo format to mono
    for (size_t i = 0; i < num_samples; i++) { // Skip every second sample (convert from stereo format to mono)
        // samples_16[i] = samples_32[i] >> 8;  // Convert 24-bit to 16-bit
        uint16_t sample_16 = samples_32[i] >> 16;  // Convert 24-bit to 16-bit

        // swap MSB and LSB
        // samples_16[i] = (sample_16 >> 8) | (sample_16 << 8);
        samples_16[i] = sample_16;
    }

    *bytes_read = num_samples*4; // * 2

    // // swap MSB and LSB
    // for (int i = 0; i < *bytes_read; i += 2) {
    //     // buf[i] = buf[i] / 10;
    //     uint8_t temp = buf[i];
    //     buf[i] = buf[i + 1];
    //     buf[i + 1] = temp;
    // }

    return ESP_OK;
}

// static void uac_device_set_mute_cb(uint32_t mute, void *arg)
// {
//     ESP_LOGI(TAG, "uac_device_set_mute_cb: %"PRIu32"", mute);
//     // bsp_extra_codec_mute_set(mute);
// }

// static void uac_device_set_volume_cb(uint32_t volume, void *arg)
// {
//     ESP_LOGI(TAG, "uac_device_set_volume_cb: %"PRIu32"", volume);
//     // bsp_extra_codec_volume_set(volume, NULL);
// }

// https://github.com/espressif/esp-idf/blob/v5.4/examples/peripherals/i2s/i2s_basic/i2s_std/main/i2s_std_example_main.c

static void i2s_init(void)
{
    i2s_chan_config_t mic_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&mic_chan_cfg, NULL, &mic_chan));

    i2s_chan_config_t amp_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&amp_chan_cfg, &amp_chan, NULL));

    i2s_std_config_t mic_std_cfg = {
        // .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .clk_cfg = {
            .sample_rate_hz = 44100,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            // .mclk_multiple = I2S_MCLK_MULTIPLE_256,
            .mclk_multiple = I2S_MCLK_MULTIPLE_384,
        },
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        // .slot_cfg = I2S_STD_PCM_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_32BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = 32,
            .ws_pol = false,
            .bit_shift = true,
            // .bit_order_lsb = true,
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = GPIO_NUM_3,
            .ws   = GPIO_NUM_2,
            .dout = I2S_GPIO_UNUSED,
            .din  = GPIO_NUM_4,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    // rx_std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(mic_chan, &mic_std_cfg));

    i2s_std_config_t amp_std_cfg = {
        // .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        // .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .clk_cfg = {
            .sample_rate_hz = 44100,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        // .slot_cfg = I2S_STD_PCM_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_DATA_BIT_WIDTH_32BIT, //I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            // .slot_mask = I2S_STD_SLOT_LEFT,
            .slot_mask = I2S_STD_SLOT_BOTH,
            .ws_width = I2S_DATA_BIT_WIDTH_16BIT,
            .ws_pol = false,
            .bit_shift = false,
            // .msb_right = true,
            .left_align = true,
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = GPIO_NUM_6,
            .ws   = GPIO_NUM_7,
            .dout = GPIO_NUM_5,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(amp_chan, &amp_std_cfg));

    ESP_ERROR_CHECK(i2s_channel_enable(mic_chan));
    ESP_ERROR_CHECK(i2s_channel_enable(amp_chan));

    // xTaskCreate(i2s_throughput_task, "i2s_throughput_task", 4096, NULL, 5, NULL);
    // xTaskCreate(audio_task, "audio_task", 4096, NULL, 5, NULL);
}

void app_main(void)
{
    i2s_init();

    // Issue: Your I2S microphone might be outputting data in a different format than expected (e.g., Left-Justified, PDM, or 24-bit data packed into 16-bit).

    uac_device_config_t config = {
        .output_cb = uac_device_output_cb,
        .input_cb = uac_device_input_cb,
        // .set_mute_cb = uac_device_set_mute_cb,
        // .set_volume_cb = uac_device_set_volume_cb,
        .set_mute_cb = NULL,
        .set_volume_cb = NULL,
        .cb_ctx = NULL,
    };

    uac_device_init(&config);
}