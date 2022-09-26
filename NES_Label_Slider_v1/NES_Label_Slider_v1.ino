/*******************************************************************************
 * NES Label Slider v1.0 by eolvera85
 * Dependent Libraries:
 * 1 - Arduino GFX: https://github.com/moononournation/Arduino_GFX
 * 2 - PNGdec: https://github.com/bitbank2/PNGdec
 * 3 - Core ESP32 Arduino: https://github.com/espressif/arduino-esp32
 ******************************************************************************/

#include <Arduino_GFX_Library.h>
#include <SD.h>
#include <PNGdec.h>

#define GFX_BL        DF_GFX_BL     // default backlight pin, you may replace DF_GFX_BL to actual backlight pin
#define PNG_FILENAME  "/intro.png"
#define BACKLIGHT_FACTOR  10        //
#define BACKLIGHT_MAXIMUM 250       //Maximun 0-250
#define BTN_GPIO_BACKLIGHT  25
      
// Arduino_GFX setting
// More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class
Arduino_DataBus *bus = new Arduino_ESP32SPI(2 /* DC */, 15 /* CS */, 14 /* SCK */, 13 /* MOSI */, 12 /* MISO */);

// More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class
Arduino_GFX *gfx = new Arduino_ST7796(bus, 4 /* RST */, 0 /* rotation */);


PNG png;
int16_t w, h, xOffset, yOffset;
File pngFile; // Functions to access a file on the SD card
int currentBackLight;

void *myOpen(const char *filename, int32_t *size)
{
  pngFile = SD.open(filename, "r");

  if (!pngFile || pngFile.isDirectory())
  {
    Serial.printf("ERROR: Failed to open '%s' file for reading", filename);
    gfx->println(F("ERROR: Failed to open file for reading"));
  }
  else
  {
    *size = pngFile.size();
    Serial.printf("Opened '%s', size: %d\n", filename, *size);
  }

  return &pngFile;
}

void myClose(void *handle)
{
  if (pngFile)
    pngFile.close();
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!pngFile)
    return 0;
    
  return pngFile.read(buffer, length);
}

int32_t mySeek(PNGFILE *handle, int32_t position)
{
  if (!pngFile)
    return 0;
    
  return pngFile.seek(position);
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw)
{
  uint16_t usPixels[320];
  uint8_t usMask[320];

  Serial.printf("Draw pos = 0,%d. size = %d x 1\n", pDraw->y, pDraw->iWidth);
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);
  png.getAlphaMask(pDraw, usMask, 1);
  gfx->draw16bitRGBBitmap(xOffset, yOffset + pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
}

void setup()
{
  Serial.begin(115200);

  // Init Display
  gfx->begin();
  gfx->fillScreen(BLACK);

  currentBackLight = BACKLIGHT_FACTOR;
  
  pinMode(BTN_GPIO_BACKLIGHT, INPUT_PULLUP);
  
  pinMode(GFX_BL, OUTPUT);
  analogWrite(GFX_BL, currentBackLight);
  
  Serial.println("Current Value BackLight");
  Serial.println(currentBackLight);
      
  if (!SD.begin(SS))
  {
    Serial.println(F("ERROR: File System Mount Failed!"));
    gfx->println(F("ERROR: File System Mount Failed!"));
  }
  else
  {
    unsigned long start = millis();
    int rc;
    
    rc = png.open(PNG_FILENAME, myOpen, myClose, myRead, mySeek, PNGDraw);
    
    if (rc == PNG_SUCCESS)
    {
      int16_t pw = png.getWidth();
      int16_t ph = png.getHeight();

      xOffset = (w - pw) / 300;
      yOffset = (h - ph) / 200;

      rc = png.decode(NULL, 0);

      Serial.printf("Draw offset: (%d, %d), time used: %lu\n", xOffset, yOffset, millis() - start);
      Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
      png.close();
    }
    else
    {
      Serial.println("png.open() failed!");
    }
  }

  //xTaskCreate(taskBackLight, "taskBackLight", 4096, NULL, 1, NULL);
  xTaskCreatePinnedToCore(taskBackLight, "taskBackLight", 1024, NULL, 1, NULL, 1);
  
  delay(5000); // 5 seconds
}

void loop()
{
  unsigned long start = millis();
  int rc;  
  File root = SD.open("/", "r");
  String strname;
  
  while (File file = root.openNextFile()) 
  {
    strname = file.name();
    strname = "/" + strname;
    
    if (!file.isDirectory() && strname.endsWith(".png") && strname != PNG_FILENAME) 
    {
      Serial.println(strname);
    
      rc = png.open(strname.c_str(), myOpen, myClose, myRead, mySeek, PNGDraw);

      if (rc == PNG_SUCCESS)
      {
        // random draw position
        int16_t pw = png.getWidth();
        int16_t ph = png.getHeight();

        //xOffset = (w - pw) / 2;
        //yOffset = (h - ph) / 2;

        rc = png.decode(NULL, 0);

        Serial.printf("Draw offset: (%d, %d), time used: %lu\n", xOffset, yOffset, millis() - start);
        Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
        png.close();
      }
      else
      {
        Serial.println("png.open() failed!");
      }
      
      delay(5000); // 1 second      
    }
  }

  root.close();
}

void taskBackLight(void * parameter)
{
  while(true)
  {
    if (digitalRead(BTN_GPIO_BACKLIGHT) == LOW)
    {
      Serial.println("Press Button BackLight");
      
      currentBackLight += BACKLIGHT_FACTOR;

      if (currentBackLight > BACKLIGHT_MAXIMUM)
        currentBackLight = BACKLIGHT_FACTOR;

      analogWrite(GFX_BL, currentBackLight);
      
      Serial.println("Current Value BackLight");
      Serial.println(currentBackLight);
    }
  }
}
