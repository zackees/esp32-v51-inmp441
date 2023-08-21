# Guides

https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started

# Important

The XIAO ESP32C3 *can only be reliably flashed on MacOS*. Windows will work a few times then stop working.

# IS2 Pinout

```cpp
#define PIN_I2S_WS GPIO_NUM_7
#define PIN_IS2_SD GPIO_NUM_8
#define PIN_I2S_SCK GPIO_NUM_4
#define PIN_AUDIO_PWR GPIO_NUM_10
```

L/R -> VDD

# Install

  * Install VSCode
    * Install the platform io plugin
  * `git clone https://github.com/zackees/xiao-inmp441-test`
  * Use VSCode to open `xiao-inmp441-test`

# Programming a board

  * Connect to XIAO board with USBC cable.
    * Make sure the cable is designed for data
    * Click build/upload in the platform io toolpanel
      * ![image](https://github.com/zackees/xiao-inmp441-test/assets/6856673/13bed300-5c01-4837-ba01-008b39cfe71e)

  * The test software should now be installed and the board should be running a test program

# Test routine

  Prior to test please SOLDER ON AN LED to EXTERNAL LED on board. Add a 120 ohm resistor in series with the LED.


https://github.com/zackees/xiao-inmp441-test/assets/6856673/368250d2-1b81-4c72-8af0-213dab47555c



  * Software Test
    * Fade on-off blink test (1 second): Led should fade on/off
      * If LED does not come on then FAIL
    * 5 seconds: ambient sound level check
      * Clap your hands
        * if led does not come on THEN FAIL
    * Click button, lights go on always
      * if no LED on then FAIL
    * SUCCESS

Prior to the battery test please solder on a lipo pouch cell >= 150 mA to the battery connections on the board.

  * Battery Test
    * Plug in power USBC
      * If no RED CHARGE LIGHT then FAIL
    * Unplug power USBC
      * If device turns off then FAIL
    * Desolder battery
    * If not failure then SUCCESS

# Low power test

This is designed for espressif.

This test shows how the IS2 doesn't wake from sleep.

  * Activate the VSCode serial monitor to the ESP32 to check std::cout output.
  * During the test, hold down the user button for a number of seconds.
  * Eventually you hit the low power test, keep holding the button.
  * Low power will enter then the microphone will be checked for 300 ms.
  * Example output (see below).

#### Example output

```
Now doing a microphone check. In a broken state the IS2 will read high values (>70 dB) before settling down to ambient noise levels.
dB: 87.0328
dB: 87.0328
dB: 86.664
...
db: 50.233
```

# Owner's design:

![image](https://github.com/zackees/xiao-inmp441-test/assets/6856673/7017fa5a-ff1d-4d03-8c54-105cfbb52e59)


# XIAO Pins

  * Datasheet
    * https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf

# Pinouts

  * XIAO:
    * ![image](https://github.com/zackees/noodz-soundreactive/assets/6856673/b1114268-d4b9-4eeb-9ecf-c81d819812d9)
  * Generic ESP32-C3
    * ![image](https://github.com/zackees/noodz-soundreactive/assets/6856673/4beef3b1-20db-4457-be57-3be4b7ca0fc7)
