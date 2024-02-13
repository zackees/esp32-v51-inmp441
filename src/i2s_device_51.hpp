
/*
Uses the new idf 5.1 i2s driver..
Not thread safe.
*/

#include <iostream>
#include <Arduino.h>
#include "defs.h"
#include <driver/i2s_std.h>
#include "i2s_device.h"
#include "driver/gpio.h"


namespace
{
  enum {
    INMP441_BIT_RESOLUTION = 24,
    INMP441_CHANNELS = 1,
    INMP441_FULL_FRAME_SIZE = 32
  };


  static_assert(AUDIO_BIT_RESOLUTION == 16, "Only 16 bit resolution is outputted by the microphone");
  static_assert(AUDIO_CHANNELS == 1, "Only 1 channel is supported");
  static_assert(sizeof(audio_sample_t) == 2, "audio_sample_t must be 16 bit");

  struct I2SContext
  {
    i2s_chan_handle_t rx_chan;
    i2s_chan_config_t i2s_chan_cfg_rx;
    i2s_std_config_t i2s_std_cfg_rx;
  };

  I2SContext make_inmp441_context() {
    I2SContext ctx;
    i2s_chan_handle_t rx_chan = NULL;
    i2s_chan_config_t i2s_chan_cfg_rx = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = AUDIO_DMA_BUFFER_COUNT,
        .dma_frame_num = IS2_AUDIO_BUFFER_LEN,
        .auto_clear = false,
    };
    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_CHANNEL_SAMPLE_RATE),
        .slot_cfg = {
                .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
                .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
                .slot_mode = I2S_SLOT_MODE_MONO,
                .slot_mask = I2S_STD_SLOT_RIGHT,
                .ws_width = 32,
                .ws_pol = false,
                .bit_shift = true,
                .left_align = true,
                .big_endian = false,
                .bit_order_lsb = false,
        },
        .gpio_cfg = {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = PIN_I2S_SCK,
                .ws   = PIN_I2S_WS,
                .dout = I2S_GPIO_UNUSED,
                .din  = PIN_IS2_SD,
                .invert_flags = {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv   = false,
                },
        },
    };
    ctx = {rx_chan, i2s_chan_cfg_rx, rx_std_cfg};
    return ctx;
  }
  
  I2SContext s_i2s_context = make_inmp441_context();

  void init_i2s_pins()
  {
    //s_i2s_context = get_i2s_context();
    esp_err_t err = i2s_new_channel(&s_i2s_context.i2s_chan_cfg_rx, NULL, &s_i2s_context.rx_chan);
    ESP_ERROR_CHECK(err);
    err = i2s_channel_init_std_mode(s_i2s_context.rx_chan, &s_i2s_context.i2s_std_cfg_rx);
    ESP_ERROR_CHECK(err);
    i2s_chan_info_t info;
    err = i2s_channel_get_info(s_i2s_context.rx_chan, &info);
    ESP_ERROR_CHECK(err);
    err = i2s_channel_enable(s_i2s_context.rx_chan);
    ESP_ERROR_CHECK(err);

    // Set the pulldown resistor on the SD pin
    gpio_set_pull_mode(PIN_IS2_SD, GPIO_PULLDOWN_ONLY);
  }
} // namespace

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
  i2s_del_channel(s_i2s_context.rx_chan);
}

void i2s_audio_enter_light_sleep()
{
  // digitalWrite(PIN_AUDIO_PWR, LOW); // Power off the IS2 microphone.
  // hold pin engaged
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.
  gpio_hold_en(PIN_AUDIO_PWR);
  i2s_channel_disable(s_i2s_context.rx_chan);
  // i2s_stop(I2S_NUM_0);
  // i2s_driver_uninstall(I2S_NUM_0);
}

void i2s_audio_exit_light_sleep()
{
  // digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.

  // i2s_start(I2S_NUM_0);
  // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_channel_enable(s_i2s_context.rx_chan);
}


size_t i2s_read_samples(audio_sample_t (&buffer)[IS2_AUDIO_BUFFER_LEN])
{
  size_t bytes_read = 0;
  esp_err_t err = i2s_channel_read(s_i2s_context.rx_chan, buffer, sizeof(buffer), &bytes_read, 0);
  const size_t count = bytes_read / sizeof(audio_sample_t);
  ASSERT(bytes_read / sizeof(audio_sample_t) <= IS2_AUDIO_BUFFER_LEN, "Buffer overflow!");
  if (err == ESP_OK)
  {
    if (bytes_read > 0)
    {
      return count;
    }
  }
  if (err != ESP_ERR_TIMEOUT)
  {
    ESP_ERROR_CHECK(err);
  }
  return 0;
}
