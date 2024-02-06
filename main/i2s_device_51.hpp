
#include <iostream>
#include <Arduino.h>
#include "audio.h"
#include "defs.h"
// #include <driver/i2s.h>
#include <driver/i2s_std.h>
#include "i2s_device.h"
#include "driver/gpio.h"
namespace
{
  bool g_is_initialized = false;
  static_assert(AUDIO_BIT_RESOLUTION == 16, "Only 16 bit resolution is supported");
  static_assert(AUDIO_CHANNELS == 1, "Only 1 channel is supported");
  static_assert(sizeof(audio_sample_t) == 2, "audio_sample_t must be 16 bit");
  i2s_chan_handle_t     rx_chan = NULL;
  i2s_chan_handle_t tx_chan = NULL;
  //i2s_chan_config_t     i2s_chan_cfg_rx = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER); // stores I2S channel values
  i2s_chan_config_t  i2s_chan_cfg_rx = {
    .id = I2S_NUM_0,
    .role = I2S_ROLE_SLAVE,
    .dma_desc_num = 6,
    .dma_frame_num = 240,
    .auto_clear = false,
  };
  i2s_std_config_t      i2s_std_cfg_rx = {
    //.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE),
    .clk_cfg = {
        .sample_rate_hz = 44100ul,
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
    .slot_cfg = {
      .data_bit_width = I2S_DATA_BIT_WIDTH_24BIT,
      .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
      .slot_mode = I2S_SLOT_MODE_MONO,
      .slot_mask = I2S_STD_SLOT_RIGHT,
      .ws_width = I2S_SLOT_BIT_WIDTH_32BIT,
      .ws_pol = false,
      .bit_shift = true,
      .left_align = true,
      .big_endian = false,
      .bit_order_lsb = false
    },
    
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
};


  void init_i2s_pins() {

    //i2s_chan_cfg_rx.id            = I2S_NUM_AUTO;  // I2S_NUM_AUTO, I2S_NUM_0, I2S_NUM_1
    //i2s_chan_cfg_rx.role          = I2S_ROLE_SLAVE;        // I2S controller master role, bclk and lrc signal will be set to output
    //i2s_chan_cfg_rx.dma_desc_num  = 8;                     // number of DMA buffer
    //i2s_chan_cfg_rx.dma_frame_num = 512;                    // I2S frame number in one DMA buffer.
    //i2s_chan_cfg_rx.auto_clear    = true;                   // i2s will always send zero automatically if no data to send
    esp_err_t err = i2s_new_channel(&i2s_chan_cfg_rx, NULL, &rx_chan);
    //rx_chan->dir = I2S_DIR_RX;
    ESP_ERROR_CHECK(err);

    //i2s_std_cfg_rx.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    //i2s_std_cfg_rx.slot_cfg.data_bit_width = I2S_DATA_BIT_WIDTH_16BIT;  // Bits per sample
    //i2s_std_cfg_rx.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT;   // I2S channel slot bit-width equals to data bit-width
    //i2s_std_cfg_rx.slot_cfg.slot_mode      = I2S_SLOT_MODE_STEREO;      // I2S_SLOT_MODE_MONO, I2S_SLOT_MODE_STEREO,
    //i2s_std_cfg_rx.slot_cfg.slot_mask      = I2S_STD_SLOT_RIGHT;         // I2S_STD_SLOT_LEFT, I2S_STD_SLOT_RIGHT
   //i2s_std_cfg_rx.slot_cfg.ws_width       = I2S_DATA_BIT_WIDTH_16BIT;  // WS signal width (i.e. the number of bclk ticks that ws signal is high)
   // i2s_std_cfg_rx.slot_cfg.ws_pol         = false;                     // WS signal polarity, set true to enable high lever first
    //i2s_std_cfg_rx.slot_cfg.bit_shift      = true;                      // Set to enable bit shift in Philips mode
    /*
    i2s_std_cfg_rx.gpio_cfg.bclk           = I2S_GPIO_UNUSED;           // BCLK, Assignment in setPinout()
    i2s_std_cfg_rx.gpio_cfg.din            = I2S_GPIO_UNUSED;           // not used
    i2s_std_cfg_rx.gpio_cfg.dout           = I2S_GPIO_UNUSED;           // DOUT, Assignment in setPinout()
    i2s_std_cfg_rx.gpio_cfg.mclk           = I2S_GPIO_UNUSED;           // MCLK, Assignment in setPinout()
    i2s_std_cfg_rx.gpio_cfg.ws             = I2S_GPIO_UNUSED;           // LRC,  Assignment in setPinout()

    i2s_std_cfg_rx.gpio_cfg.bclk           = PIN_I2S_SCK;           // BCLK, Assignment in setPinout()
    i2s_std_cfg_rx.gpio_cfg.din            = PIN_IS2_SD;           // not used
    i2s_std_cfg_rx.gpio_cfg.dout           = I2S_GPIO_UNUSED;           // DOUT, Assignment in setPinout()
    i2s_std_cfg_rx.gpio_cfg.mclk           = I2S_GPIO_UNUSED;           // MCLK, Assignment in setPinout()
    i2s_std_cfg_rx.gpio_cfg.ws             = PIN_I2S_WS;           // LRC,  Assignment in setPinout()
    i2s_std_cfg_rx.gpio_cfg.invert_flags.mclk_inv = false;
    i2s_std_cfg_rx.gpio_cfg.invert_flags.bclk_inv = false;
    i2s_std_cfg_rx.gpio_cfg.invert_flags.ws_inv   = false;
    i2s_std_cfg_rx.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100);
        */
    //i2s_std_cfg_rx.clk_cfg.sample_rate_hz = 44100;
    //i2s_std_cfg_rx.clk_cfg.clk_src        = I2S_CLK_SRC_DEFAULT;        // Select PLL_F160M as the default source clock
    //i2s_std_cfg_rx.clk_cfg.mclk_multiple  = I2S_MCLK_MULTIPLE_128;      // mclk = sample_rate * 128
    //i2s_std_cfg_rx.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    //i2s_std_cfg_rx.slot_cfg.bit_shift = 1;
    //i2s_std_cfg_rx.slot_cfg.ws_width = 20;
    
    //i2s_std_cfg_rx.slot_cfg.big_endian = 1;
    // bit_order_lsbbit_order_lsb
    //i2s_std_cfg_rx.slot_cfg.bit_order_lsb = 1;
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

/*
    // Configure the pin as an input
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // Disable GPIO interrupts
    io_conf.mode = GPIO_MODE_INPUT;        // Set as Input mode
    io_conf.pin_bit_mask = (1ULL << gpio_pin); // Bit mask of the pin
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; // Enable pull-down resistor
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;    // Disable pull-up resistor
    */

    // Apply the GPIO configuration
    //gpio_config(&io_conf);
    err = gpio_pulldown_en(gpio_pin);
    ESP_ERROR_CHECK(err);
  }

#if 0
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = AUDIO_SAMPLE_RATE,
      .bits_per_sample = i2s_bits_per_sample_t(AUDIO_BIT_RESOLUTION),
      .channel_format = i2s_channel_fmt_t(I2S_CHANNEL_FMT_ONLY_RIGHT),
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0,
      .dma_buf_count = AUDIO_DMA_BUFFER_COUNT,
      .dma_buf_len = IS2_AUDIO_BUFFER_LEN,
      //.use_apll = true
      // .tx_desc_auto_clear ?
  };

  const i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_I2S_SCK,
      .ws_io_num = PIN_I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = PIN_IS2_SD};

  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = 16000,                              // 16KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,      // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,      // use right channel
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
      .dma_buf_count = 4,                       // number of buffers
      .dma_buf_len = 8                          // 8 samples per buffer (minimum)
  };
  #endif

}

void i2s_audio_init()
{
#if 0
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
  // i2s_start(I2S_NUM_0);
#else
  //init_i2s_pins();
#endif
}

//esp_err_t i2s_channel_read(i2s_chan_handle_t handle, void *dest, size_t size, size_t *bytes_read, uint32_t timeout_ms)

void i2s_audio_shutdown()
{
  // i2s_stop(I2S_NUM_0);
  // i2s_driver_uninstall(I2S_NUM_0);
  i2s_del_channel(rx_chan);
}

size_t i2s_read_raw_samples(audio_sample_t (&buffer)[IS2_AUDIO_BUFFER_LEN])
{
  if (!g_is_initialized)
  {
    init_i2s_pins();
    g_is_initialized = true;
  }
  #if 0
  size_t bytes_read = 0;
  i2s_event_t event;

  uint32_t current_time = millis();
  esp_err_t result = i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytes_read, 0);
  if (result == ESP_OK)
  {
    if (bytes_read > 0)
    {
      // cout << "Bytes read: " << bytes_read << endl;
      const size_t count = bytes_read / sizeof(audio_sample_t);
      return count;
    }
  }
  return 0;
  #else
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
  if (err != ESP_ERR_TIMEOUT) {
    ESP_ERROR_CHECK(err);
  }
  return 0;
  #endif
}