
/*
Uses the new idf 5.1 i2s driver..
*/

#include <iostream>
#include <Arduino.h>
#include "defs.h"
// #include <driver/i2s.h>
#include <driver/i2s_std.h>
#include "i2s_device.h"
#include "driver/gpio.h"

#include "i2s_device.h"

#define SD_PULLDOWN_ENABLED 0

#define I2S_STD_MSB_BIT_SHIFTED_SLOT_DEFAULT_CONFIG(bits_per_sample, mono_or_stereo) \
  {                                                                                  \
    .data_bit_width = bits_per_sample,                                               \
    .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,                                       \
    .slot_mode = mono_or_stereo,                                                     \
    .slot_mask = I2S_STD_SLOT_BOTH,                                                  \
    .ws_width = bits_per_sample,                                                     \
    .ws_pol = false,                                                                 \
    .bit_shift = true,                                                               \
    .left_align = true,                                                              \
    .big_endian = false,                                                             \
    .bit_order_lsb = false                                                           \
  }

namespace
{
  static_assert(AUDIO_BIT_RESOLUTION == 24, "Only 16 bit resolution is supported");
  static_assert(AUDIO_CHANNELS == 1, "Only 1 channel is supported");
  static_assert(sizeof(audio_sample_t) == 4, "audio_sample_t must be 32 bit");

  struct I2SContext
  {
    i2s_chan_handle_t rx_chan;
    i2s_chan_config_t i2s_chan_cfg_rx;
    i2s_std_config_t i2s_std_cfg_rx;
  };

  I2SContext g_i2s_context;

  I2SContext get_i2s_context()
  {
    I2SContext ctx;
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

    ctx = {rx_chan, i2s_chan_cfg_rx, i2s_std_cfg_rx};
    return ctx;
  }

  I2SContext make_philips_i2s()
  {
    // I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG

    I2SContext ctx;
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

        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
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
    }; //

    ctx = {rx_chan, i2s_chan_cfg_rx, i2s_std_cfg_rx};
    return ctx;
  }

  I2SContext make_msb_i2s()
  {
    I2SContext ctx;
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

        .slot_cfg = I2S_STD_MSB_BIT_SHIFTED_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_MONO),
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

    ctx = {rx_chan, i2s_chan_cfg_rx, i2s_std_cfg_rx};
    return ctx;
  }

  I2SContext make_pcm_i2s()
  {
    I2SContext ctx;
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

        .slot_cfg = I2S_STD_PCM_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_MONO),
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

    ctx = {rx_chan, i2s_chan_cfg_rx, i2s_std_cfg_rx};
    return ctx;
  }

  I2SContext make_inmp441_2()
  {
    i2s_chan_handle_t rx_chan = NULL;
    /* RX channel will be registered on our second I2S (for now)*/
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan);
    i2s_std_config_t std_rx_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = PIN_I2S_SCK,
            .ws = PIN_I2S_WS,
            .dout = I2S_GPIO_UNUSED,
            .din = PIN_IS2_SD,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = true,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_rx_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
    I2SContext ctx = {rx_chan, rx_chan_cfg, std_rx_cfg};
    return ctx;
  }

  I2SContext make_inmp441_context() {
    I2SContext ctx;
    i2s_chan_handle_t rx_chan = NULL;
    i2s_chan_handle_t tx_chan = NULL;
    i2s_chan_config_t i2s_chan_cfg_rx = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 6,
        .dma_frame_num = 240,
        .auto_clear = false,
    };
    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(48000),
        .slot_cfg = {
                .data_bit_width = I2S_DATA_BIT_WIDTH_24BIT,
                .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
                .slot_mode = I2S_SLOT_MODE_STEREO,
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
                        .bclk_inv = true,
                        .ws_inv   = false,
                },
        },
    };
    ctx = {rx_chan, i2s_chan_cfg_rx, rx_std_cfg};
    return ctx;
  }

  void init_i2s_pins()
  {
    //g_i2s_context = get_i2s_context();
    g_i2s_context = make_inmp441_2();
    esp_err_t err = i2s_new_channel(&g_i2s_context.i2s_chan_cfg_rx, NULL, &g_i2s_context.rx_chan);
    ESP_ERROR_CHECK(err);
    err = i2s_channel_init_std_mode(g_i2s_context.rx_chan, &g_i2s_context.i2s_std_cfg_rx);
    ESP_ERROR_CHECK(err);
    i2s_chan_info_t info;
    err = i2s_channel_get_info(g_i2s_context.rx_chan, &info);
    ESP_ERROR_CHECK(err);
    delay(1000);
    Serial.printf("I2S channel info: %d, %d, %d, %d, %d\n", info.id, info.role, info.dir, info.mode, info.pair_chan);
    delay(500);

    ESP_ERROR_CHECK(err);
    err = i2s_channel_enable(g_i2s_context.rx_chan);
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

  void init_pulldown_datapin()
  {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;       // Disable GPIO interrupts
    io_conf.mode = GPIO_MODE_INPUT;              // Set as Input mode
    io_conf.pin_bit_mask = (1ULL << PIN_IS2_SD); // Bit mask of the pin
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; // Enable pull-down resistor
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;    // Disable pull-up resistor
    gpio_config(&io_conf);
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
  i2s_del_channel(g_i2s_context.rx_chan);
}

void i2s_audio_enter_light_sleep()
{
  // digitalWrite(PIN_AUDIO_PWR, LOW); // Power off the IS2 microphone.
  // hold pin engaged
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.
  gpio_hold_en(PIN_AUDIO_PWR);
  // i2s_stop(I2S_NUM_0);
  // i2s_driver_uninstall(I2S_NUM_0);
}

void i2s_audio_exit_light_sleep()
{
  // digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.

  // i2s_start(I2S_NUM_0);
  // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
}

size_t i2s_read_raw_samples(audio_sample_t (&buffer)[IS2_AUDIO_BUFFER_LEN])
{

  size_t bytes_read = 0;
  esp_err_t err = i2s_channel_read(g_i2s_context.rx_chan, buffer, sizeof(buffer), &bytes_read, 0);
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