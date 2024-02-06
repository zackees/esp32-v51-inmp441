
/*
Uses the new idf 5.1 i2s driver..
*/

#include <iostream>
#include <Arduino.h>
#include "audio.h"
#include "defs.h"
// #include <driver/i2s.h>
#include <driver/i2s_std.h>
#include "i2s_device.h"
#include "driver/gpio.h"

#define SD_PULLDOWN_ENABLED 0

namespace
{
  static_assert(AUDIO_BIT_RESOLUTION == 16, "Only 16 bit resolution is supported");
  static_assert(AUDIO_CHANNELS == 1, "Only 1 channel is supported");
  static_assert(sizeof(audio_sample_t) == 2, "audio_sample_t must be 16 bit");
  i2s_chan_handle_t rx_chan = NULL;
  i2s_chan_handle_t tx_chan = NULL;
  i2s_chan_config_t i2s_chan_cfg_rx = {
      .id = I2S_NUM_0,
      .role = I2S_ROLE_MASTER,
      .dma_desc_num = 6,
      .dma_frame_num = 240,
      .auto_clear = false,
  };
  i2s_std_config_t i2s_std_cfg_rx = {
      .clk_cfg = {
          .sample_rate_hz = AUDIO_SAMPLE_RATE,
          .clk_src = i2s_clock_src_t(I2S_CLK_SRC_PLL_160M),
          .mclk_multiple = I2S_MCLK_MULTIPLE_384,
      },

      //.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_MONO),
      /*
  #define I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bits_per_sample, mono_or_stereo) { \
      .data_bit_width = bits_per_sample, \
      .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO, \
      .slot_mode = mono_or_stereo, \
      .slot_mask = I2S_STD_SLOT_BOTH, \
      .ws_width = bits_per_sample, \
      .ws_pol = false, \
      .bit_shift = true, \
      .left_align = true, \
      .big_endian = false, \
      .bit_order_lsb = false \
  }
      */
      .slot_cfg = {.data_bit_width = i2s_data_bit_width_t(AUDIO_BIT_RESOLUTION),
                   .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT, // Check this
                   .slot_mode = I2S_SLOT_MODE_MONO,
                   .slot_mask = I2S_STD_SLOT_RIGHT,
                   .ws_width = I2S_SLOT_BIT_WIDTH_32BIT,
                   .ws_pol = false,
                   .bit_shift = true,  // philips mode
                   .left_align = true, // philips mode.
                   .big_endian = false,
                   .bit_order_lsb = false},

      .gpio_cfg = {
          .mclk = GPIO_NUM_1,
          .bclk = PIN_I2S_SCK,
          .ws = PIN_I2S_WS,
          .dout = I2S_GPIO_UNUSED,
          .din = PIN_IS2_SD,
          .invert_flags = {
              .mclk_inv = false,
              .bclk_inv = false,
              .ws_inv = false,
          },
      },
  }; //

  void init_pulldown_datapin() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // Disable GPIO interrupts
    io_conf.mode = GPIO_MODE_INPUT;        // Set as Input mode
    io_conf.pin_bit_mask = (1ULL << PIN_IS2_SD); // Bit mask of the pin
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; // Enable pull-down resistor
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;    // Disable pull-up resistor
    gpio_config(&io_conf);
  }

  void init_i2s_pins()
  {
    esp_err_t err = i2s_new_channel(&i2s_chan_cfg_rx, NULL, &rx_chan);
    ESP_ERROR_CHECK(err);
    err = i2s_channel_init_std_mode(rx_chan, &i2s_std_cfg_rx);
    ESP_ERROR_CHECK(err);
    i2s_chan_info_t info;
    err = i2s_channel_get_info(rx_chan, &info);
    ESP_ERROR_CHECK(err);
    delay(1000);
    Serial.printf("I2S channel info: %d, %d, %d, %d, %d\n", info.id, info.role, info.dir, info.mode, info.pair_chan);
    delay(500);

    ESP_ERROR_CHECK(err);
    err = i2s_channel_enable(rx_chan);
    ESP_ERROR_CHECK(err);

    gpio_num_t gpio_pin = PIN_IS2_SD; // Replace with your GPIO pin

    // Data sheet recommends a pulldown on the input pin when connecting stero microphones.
    // However this may also be necessary in mono mode because of the tri state nature of the
    // I2S bus.
    // Configure the pin as an input
    #if SD_PULLDOWN_ENABLED
    init_pulldown_datapin();
    #endif
  }
}  // namespace

void i2s_audio_init()
{
  pinMode(PIN_AUDIO_PWR, OUTPUT);
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.
  init_i2s_pins();
}

// esp_err_t i2s_channel_read(i2s_chan_handle_t handle, void *dest, size_t size, size_t *bytes_read, uint32_t timeout_ms)

void i2s_audio_shutdown()
{
  // i2s_stop(I2S_NUM_0);
  // i2s_driver_uninstall(I2S_NUM_0);
  i2s_del_channel(rx_chan);
}

size_t i2s_read_raw_samples(audio_sample_t (&buffer)[IS2_AUDIO_BUFFER_LEN])
{

  size_t bytes_read = 0;
  esp_err_t err = i2s_channel_read(rx_chan, buffer, sizeof(buffer), &bytes_read, 0);
  if (err == ESP_OK)
  {
    if (bytes_read > 0)
    {
      // cout << "Bytes read: " << bytes_read << endl;
      const size_t count = bytes_read / sizeof(audio_sample_t);
      return count;
    }
  }
  if (err != ESP_ERR_TIMEOUT)
  {
    ESP_ERROR_CHECK(err);
  }
  return 0;
}