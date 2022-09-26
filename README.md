# NES Label Slider
View the entire collection of Nintendo Entertainment System game covers on a tft screen.

![alt text](NesLabelSlider.png?raw=true)

Required Components: 
- ESP32 WROOM 
- TFT LCD 4.0'' / 3.5''
- NES Game Shell 
- SD/MicroSD Card Reader

### Notes:
- Format is FAT32.
- Added button for backlight control (default is 50) for each press increment 50 (Minimum 0 / Maximum 250).
- You need to configure the driver of the TFT screen.

Implementation performed by [@David](https://twitter.com/XGAMES_VJ) ¡ Camarada a seguirle dando !
- https://youtu.be/3pPOBtM_-Gw

## Possible improvements in the future
- Add Lipo Battery Rechargeable (View state on screen)
- Previous/Stop/Next image button. 
- Change the display delay of images.
- Sleep mode (Screen Off) / Alarm clock (Speaker)

Resources used:
- Arduino GFX: https://github.com/moononournation/Arduino_GFX
- PNGdec: https://github.com/bitbank2/PNGdec
- Core ESP32 Arduino: https://github.com/espressif/arduino-esp32

Similar projects:
- https://www.thingiverse.com/thing:2159241
- https://www.instructables.com/NES-Label-Slider-With-35-TFT-and-Raspberry-Pi-Zero/
