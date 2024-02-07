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

Simply power the board and read the stdout through the connection monitor.



# Schematic

![image](https://github.com/zackees/esp32-v51-inmp441/assets/6856673/95f2223f-6127-4bc0-a346-03e80c125f60)


# XIAO Pins

  * Datasheet
    * https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf

# Pinouts

  * XIAO:
    * ![image](https://github.com/zackees/noodz-soundreactive/assets/6856673/b1114268-d4b9-4eeb-9ecf-c81d819812d9)
  * Generic ESP32-C3
    * ![image](https://github.com/zackees/noodz-soundreactive/assets/6856673/4beef3b1-20db-4457-be57-3be4b7ca0fc7)
