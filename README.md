# NES Label Slider
View the entire collection of Nintendo Entertainment System game covers on a tft screen.

![alt text](NesLabelSlider.png?raw=true)

### Required Components: 
- ESP32 WROOM 
- TFT LCD 4.0'' / 3.5'' (ST7796 / ILI9488)
- NES Game Shell 
- SD/MicroSD Card Reader
- 3 Microswitch button

### Buttons 
These buttons are for the function of:
- Change the image manually.
- Modify the image change delay time (Stopped "Fix a single cover"/5 sec/15 sec/30 sec/60 sec)
- Change screen brightness (Minium 0 / Maximun 250)

Function button:
- Mode/Option: Image / Brightness / Delay
- Plus: Next Image / Up Brightness/Delay 
- Minus: Previous Image / Down Brightness/Delay

### Define the screen with which it will be programmed. 
```C
#define TFT_ST7796
//#define TFT_ILI9488
```

### GPIO Pinout TFT Screen/Buttons/MicroSD
```C
#if defined(TFT_ST7796)
  #define GFX_BL        22
  #define DC            2             
  #define CS            15            
  #define SCK           14            
  #define MOSI          13
  #define MISO          12
  #define RST           4
#endif

#if defined(TFT_ILI9488)
  #define DC            15
  #define CS            33
  #define WR            4
  #define RD            2
  #define D0            12
  #define D1            13
  #define D2            26
  #define D3            25
  #define D4            17
  #define D5            16
  #define D6            27
  #define D7            14
  #define RST           32
#endif
```
```C
#define BTN_GPIO_MODE       21
#define BTN_GPIO_MINUS      3
#define BTN_GPIO_PLUS       1
```
```C
#define SD_GPIO_CS          5
#define SD_GPIO_MOSI        23
#define SD_GPIO_MISO        19
#define SD_GPIO_SCK         18
```

Implementation performed by [@David](https://twitter.com/XGAMES_VJ) ¡ Camarada a seguirle dando !
- https://youtu.be/3pPOBtM_-Gw

### Notes:
- The MicroSD Memory must be formatted in FAT32.
- The images must be with png extension and in 320x480 resolution
- The battery level indicator is in development (Fixed value of 99% is indicated)

## Possible improvements in the future
- Add Lipo Battery Rechargeable (View state on screen)
- Sleep mode (Screen Off) / Alarm clock (Speaker)

Resources used:
- Arduino GFX: https://github.com/moononournation/Arduino_GFX
- PNGdec: https://github.com/bitbank2/PNGdec
- Core ESP32 Arduino: https://github.com/espressif/arduino-esp32

Similar projects:
- https://www.thingiverse.com/thing:2159241
- https://www.instructables.com/NES-Label-Slider-With-35-TFT-and-Raspberry-Pi-Zero/