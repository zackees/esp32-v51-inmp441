
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

#include "esp_pm.h"
#include "util.h"

namespace
{
  enum
  {
    INMP441_BIT_RESOLUTION = 24,
    INMP441_CHANNELS = 1,
    INMP441_FULL_FRAME_SIZE = 32,
    INMP441_FILTER_SIGNAL = 1, // true means the signal is filtered from wake from sleep glitch
    INMP441_FREEZE_APB_CLOCK = 0,
    INMP441_DISABLE_CHANNEL_ON_LIGHT_SLEEP = 0,
  };

  static_assert(AUDIO_BIT_RESOLUTION == 16, "Only 16 bit resolution is outputted by the microphone");
  static_assert(AUDIO_CHANNELS == 1, "Only 1 channel is supported");
  static_assert(sizeof(audio_sample_t) == 2, "audio_sample_t must be 16 bit");




  QueueHandle_t s_audio_queue;
  dma_buffer_t s_last_dma_buffer;
  bool s_last_dma_buffer_valid = true;

  struct I2SContext
  {
    i2s_chan_handle_t rx_chan;
    i2s_chan_config_t i2s_chan_cfg_rx;
    i2s_std_config_t i2s_std_cfg_rx;
  };

  I2SContext make_inmp441_context()
  {
    I2SContext ctx;
    i2s_chan_handle_t rx_chan = NULL;
    i2s_chan_config_t i2s_chan_cfg_rx = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = AUDIO_DMA_BUFFER_COUNT,
        .dma_frame_num = AUDIO_SAMPLES_PER_DMA_BUFFER, // Maybe make this 2 since we are using a callback.
        .auto_clear = false,
    };
    i2s_std_config_t rx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_CHANNEL_SAMPLE_RATE),
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
            .ws = PIN_I2S_WS,
            .dout = I2S_GPIO_UNUSED,
            .din = PIN_IS2_SD,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ctx = {rx_chan, i2s_chan_cfg_rx, rx_std_cfg};
    return ctx;
  }
  I2SContext s_i2s_context = make_inmp441_context();
  esp_pm_lock_handle_t apb_lock;
  void acquire_apb_power_lock() {
    esp_err_t err = esp_pm_lock_create(
      ESP_PM_APB_FREQ_MAX,
      0,
      "i2s-apb-lock",
      &apb_lock);
  }
} // namespace

void i2s_audio_init()
{
  pinMode(PIN_AUDIO_PWR, OUTPUT);
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.
  if (INMP441_FREEZE_APB_CLOCK) {
    acquire_apb_power_lock();
  }
  esp_err_t err = i2s_new_channel(&s_i2s_context.i2s_chan_cfg_rx, NULL, &s_i2s_context.rx_chan);
  ESP_ERROR_CHECK(err);
  err = i2s_channel_init_std_mode(s_i2s_context.rx_chan, &s_i2s_context.i2s_std_cfg_rx);
  ESP_ERROR_CHECK(err);
  i2s_chan_info_t info;
  err = i2s_channel_get_info(s_i2s_context.rx_chan, &info);
  ESP_ERROR_CHECK(err);
  // ###
  // # Now initialize the interrupt callback to recieve samples.
  err = i2s_channel_enable(s_i2s_context.rx_chan);
  ESP_ERROR_CHECK(err);
  // Set the pulldown resistor on the SD pin
  gpio_set_pull_mode(PIN_IS2_SD, GPIO_PULLDOWN_ONLY);
}

// esp_err_t i2s_channel_read(i2s_chan_handle_t handle, void *dest, size_t size, size_t *bytes_read, uint32_t timeout_ms)

void i2s_audio_shutdown()
{
  // i2s_stop(I2S_NUM_0);
  //  i2s_driver_uninstall(I2S_NUM_0);
  if (INMP441_DISABLE_CHANNEL_ON_LIGHT_SLEEP)
  {
    i2s_channel_disable(s_i2s_context.rx_chan);
  }
  // i2s_del_channel(s_i2s_context.rx_chan);
}

void i2s_audio_enter_light_sleep()
{
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone during light sleep.
  gpio_hold_en(PIN_AUDIO_PWR);
  //i2s_channel_disable(s_i2s_context.rx_chan);
  if (INMP441_DISABLE_CHANNEL_ON_LIGHT_SLEEP)
  {
    i2s_channel_disable(s_i2s_context.rx_chan);
  }
}

void i2s_audio_exit_light_sleep()
{
  //i2s_channel_enable(s_i2s_context.rx_chan);
  if (INMP441_DISABLE_CHANNEL_ON_LIGHT_SLEEP)
  {
    i2s_channel_enable(s_i2s_context.rx_chan);
  }
}


size_t i2s_read_dma_buffer(dma_buffer_t *dma_block, uint32_t timeout)
{
  size_t bytes_read = 0;
  esp_err_t err = i2s_channel_read(
      s_i2s_context.rx_chan,
      dma_block,
      sizeof(dma_buffer_t),
      &bytes_read,
      timeout // timeout - non blocking
  );
  uint32_t start = millis();
  if (err != ESP_OK)
  {
    if (err != ESP_ERR_TIMEOUT)
    {
      ESP_ERROR_CHECK(err);
    }
    else
    {
      // cout << "Timeout occured" << endl;
    }
    return 0;
  }
  if (INMP441_FILTER_SIGNAL)
  {
    audio_sample_t *begin = reinterpret_cast<audio_sample_t *>(dma_block);
    audio_sample_t *end = begin + AUDIO_SAMPLES_PER_DMA_BUFFER;
    int32_t vol = max_volume(begin, end);
    if (vol == 0)
    {
      // Corrupted data, unique to the inmp441 (on esp32-c3?)
      // instead just copy the last buffer.
      memcpy(dma_block, &s_last_dma_buffer, sizeof(dma_buffer_t));
      s_last_dma_buffer_valid = false;
      return AUDIO_SAMPLES_PER_DMA_BUFFER;
    }
    // 2nd state, is this the next buffer after the corrupted buffer?
    // If so, then it is corrupted as well.
    if (!s_last_dma_buffer_valid)
    {
      s_last_dma_buffer_valid = true; // Reset the state.
      memcpy(&s_last_dma_buffer, dma_block, sizeof(dma_buffer_t));
      return AUDIO_SAMPLES_PER_DMA_BUFFER;
    }
    // Store the last buffer.
    s_last_dma_buffer_valid = true;
    memcpy(&s_last_dma_buffer, dma_block, sizeof(dma_buffer_t));
  }
  return bytes_read / sizeof(audio_sample_t);
}

size_t i2s_read_samples(audio_sample_t *begin, audio_sample_t *end, uint32_t timeout)
{
  size_t bytes_read = 0;
  size_t n_samples = 0;
  const bool timeout_enabled = timeout > 0;
  uint32_t expire_time = millis() + timeout;
  while ((begin < end) && (!timeout_enabled || millis() < expire_time))
  {
    size_t samples_left = end - begin;
    if (samples_left < AUDIO_SAMPLES_PER_DMA_BUFFER)
    {
      cout << "break because samples_left: " << samples_left << endl;
      break;
    }
    dma_buffer_t *block = reinterpret_cast<dma_buffer_t *>(begin);
    size_t new_samples = i2s_read_dma_buffer(block, 0);
    n_samples += new_samples;
    if (new_samples != AUDIO_SAMPLES_PER_DMA_BUFFER)
    {
      break;
    }
    begin += n_samples;
  }
  return n_samples;
}
