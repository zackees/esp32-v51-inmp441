#include "led.h"
#include <math.h>
#include <Arduino.h>

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"

#include "driver/ledc.h"
#include "defs.h"

#define MAX_8BIT 255
#define MAX_16BIT 65535
#define MAX_LED_VALUE ((1 << LED_PWM_RESOLUTION) - 1)

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_14_BIT // Set duty resolution to 13 bits
//#define LEDC_DUTY (4095)                // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FULL_DUTY (16383)            // Set duty to 100%. ((2 ** 14) - 1) = 16383
#define LEDC_FREQUENCY (1024*4)           // Frequency in Hertz. Set frequency at 5 kHz

// #define ENABLE_SLEEP_COMPATIBLE_LEDC

namespace
{


static const uint16_t PROGMEM _gamma_2_8[256] = {
    0,     0,     0,     1,     1,     2,     4,     6,     8,     11,    14,
    18,    23,    29,    35,    41,    49,    57,    67,    77,    88,    99,
    112,   126,   141,   156,   173,   191,   210,   230,   251,   274,   297,
    322,   348,   375,   404,   433,   464,   497,   531,   566,   602,   640,
    680,   721,   763,   807,   853,   899,   948,   998,   1050,  1103,  1158,
    1215,  1273,  1333,  1394,  1458,  1523,  1590,  1658,  1729,  1801,  1875,
    1951,  2029,  2109,  2190,  2274,  2359,  2446,  2536,  2627,  2720,  2816,
    2913,  3012,  3114,  3217,  3323,  3431,  3541,  3653,  3767,  3883,  4001,
    4122,  4245,  4370,  4498,  4627,  4759,  4893,  5030,  5169,  5310,  5453,
    5599,  5747,  5898,  6051,  6206,  6364,  6525,  6688,  6853,  7021,  7191,
    7364,  7539,  7717,  7897,  8080,  8266,  8454,  8645,  8838,  9034,  9233,
    9434,  9638,  9845,  10055, 10267, 10482, 10699, 10920, 11143, 11369, 11598,
    11829, 12064, 12301, 12541, 12784, 13030, 13279, 13530, 13785, 14042, 14303,
    14566, 14832, 15102, 15374, 15649, 15928, 16209, 16493, 16781, 17071, 17365,
    17661, 17961, 18264, 18570, 18879, 19191, 19507, 19825, 20147, 20472, 20800,
    21131, 21466, 21804, 22145, 22489, 22837, 23188, 23542, 23899, 24260, 24625,
    24992, 25363, 25737, 26115, 26496, 26880, 27268, 27659, 28054, 28452, 28854,
    29259, 29667, 30079, 30495, 30914, 31337, 31763, 32192, 32626, 33062, 33503,
    33947, 34394, 34846, 35300, 35759, 36221, 36687, 37156, 37629, 38106, 38586,
    39071, 39558, 40050, 40545, 41045, 41547, 42054, 42565, 43079, 43597, 44119,
    44644, 45174, 45707, 46245, 46786, 47331, 47880, 48432, 48989, 49550, 50114,
    50683, 51255, 51832, 52412, 52996, 53585, 54177, 54773, 55374, 55978, 56587,
    57199, 57816, 58436, 59061, 59690, 60323, 60960, 61601, 62246, 62896, 63549,
    64207, 64869, 65535};

  uint16_t gamma_correct(uint8_t val)
  {
    // return uint16_t(val) * uint16_t(val);
    uint16_t val16 = _gamma_2_8[val];
    return val16;
  }

  uint32_t map_to_led_duty_cycle(uint16_t val16)
  {
    #if !LED_PIN_IS_SINK
    uint32_t val32 = map(val16, 0, MAX_16BIT, 0, LEDC_FULL_DUTY);
    #else
    uint32_t val32 = map(val16, 0, MAX_16BIT, LEDC_FULL_DUTY, 0);
    #endif
    return val32;
  }

  ledc_timer_config_t ledc_timer = {
      //.duty_resolution  = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .speed_mode = LEDC_MODE,
      .duty_resolution = LEDC_DUTY_RES,
      .timer_num = LEDC_TIMER,
      .freq_hz = LEDC_FREQUENCY, // Set output frequency
      .clk_cfg = LEDC_AUTO_CLK};

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {
      .gpio_num = LED_PIN,
      .speed_mode = LEDC_MODE,
      .channel = LEDC_CHANNEL,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER,
      .duty = 0, // Set duty to 0%
      .hpoint = 0};
} // namespace

void led_setup()
{
  // Prepare and then apply the LEDC PWM timer configuration
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
  led_write(0);
}

void led_write(uint8_t val)
{
  uint16_t val16 = gamma_correct(val);
  uint32_t val32 = map_to_led_duty_cycle(val16);
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, val32));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

void test_led()
{
  const int delay_ms = 4;
  for (int i = 0; i <= 255; i += 1) {
    led_write(i);
    delay(delay_ms);
  }
  for (int i = 255; i >= 0; i -= 1) {
    led_write(i);
    delay(delay_ms);
  }
}
