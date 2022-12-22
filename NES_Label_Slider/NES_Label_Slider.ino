/*******************************************************************************
 * NES Label Slider by eolvera85
 * Dependent Libraries:
 * 1 - Arduino GFX: https://github.com/moononournation/Arduino_GFX
 * 2 - PNGdec: https://github.com/bitbank2/PNGdec
 * 3 - Core ESP32 Arduino: https://github.com/espressif/arduino-esp32
 ******************************************************************************/

#include <Arduino_GFX_Library.h>
#include <PNGdec.h>
#include <SD.h>
#include <Preferences.h>
#include "ListDynamic.h"

enum BTN_ACTION_E {MINUS, PLUS};

#define APP_NAME_VALUE      "NES_LABEL"
#define KEY_BACKLIGHT_VALUE "keyBkLightLast"
#define KEY_PNG_VALUE       "keyPngLast"
#define KEY_DELAY_VALUE     "keyDelayLast"

//#define TFT_ST7796
#define TFT_ILI9488

#if defined(TFT_ST7796)
  #define GFX_BL        DF_GFX_BL     // GPIO22 default backlight pin, you may replace DF_GFX_BL to actual backlight pin
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

#define PNG_FILENAME        "/intro.png"
#define BACKLIGHT_FACTOR    10            //
#define BACKLIGHT_MINIUM    0             //Maximun 0
#define BACKLIGHT_MAXIMUM   250           //Maximun 250
#define BTN_GPIO_MODE       21
#define BTN_GPIO_MINUS      3
#define BTN_GPIO_PLUS       1

#define DELAY_05_SECONDS     5000
#define DELAY_15_SECONDS     15000
#define DELAY_30_SECONDS     30000
#define DELAY_60_SECONDS     60000

// Arduino_GFX setting // More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class
#if defined(TFT_ST7796)
  Arduino_DataBus *bus = new Arduino_ESP32SPI(DC, CS, SCK, MOSI, MISO);
#endif
#if defined(TFT_ILI9488)
  Arduino_DataBus *bus = new Arduino_ESP32PAR8(DC, CS, WR, RD, D0, D1, D2 , D3, D4, D5, D6, D7);
#endif

// More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class
#if defined(TFT_ST7796)
  Arduino_GFX *gfx = new Arduino_ST7796(bus, RST, 0 /* rotation */);
#endif
#if defined(TFT_ILI9488)
  Arduino_GFX *gfx = new Arduino_ILI9488(bus, RST, 0 /* rotation */, false /* IPS */);
#endif

PNG png;
File pngFile;
Preferences preferences;

int16_t w, h, xOffset, yOffset;

int currentBackLight;
int currentDelay;
int currentIndexPng;
int currentAction;
int countPng;

String currentImage;

List<String> listImages;
List<String> listMode;
List<String> listDelay;

bool visibleBar;
bool canDrawBar;

unsigned long barVisibleTime;

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

  File root;
  String strname;

  int xCursor;
  int yCursor;
  int indexImage;
    
  // Init Display
  gfx->begin();
  gfx->fillScreen(BLACK);

  //Read BackLight last
  preferences.begin(APP_NAME_VALUE, false);
  currentBackLight = preferences.getInt(KEY_BACKLIGHT_VALUE, BACKLIGHT_FACTOR);
  currentIndexPng = preferences.getInt(KEY_PNG_VALUE, 0);
  currentDelay = preferences.getInt(KEY_DELAY_VALUE, 1);
  preferences.end();
  
  pinMode(BTN_GPIO_MODE, INPUT_PULLUP);
  pinMode(BTN_GPIO_MINUS, INPUT_PULLUP);
  pinMode(BTN_GPIO_PLUS, INPUT_PULLUP);

  #if defined(TFT_ST7796)
    pinMode(GFX_BL, OUTPUT);
    analogWrite(GFX_BL, BACKLIGHT_MAXIMUM);    
  #endif

  if (!SD.begin(SS))
  {
    Serial.println(F("ERROR: File System Mount Failed!"));
    gfx->println(F("ERROR: File System Mount Failed!"));
  }
  else
  {
    xCursor = 5;
    yCursor = 5;

    gfx->setTextColor(RED);
    gfx->setCursor(xCursor, yCursor);
    gfx->print("Reading images....");
    gfx->setTextColor(WHITE);
    
    root = SD.open("/", "r");
    while (File file = root.openNextFile()) 
    {
      strname = file.name();
    
      if (!file.isDirectory() && strname.endsWith(".png") && ("/" + strname) != PNG_FILENAME) 
      {
        listImages.Add(strname);

        yCursor += 10;

        gfx->setCursor(xCursor, yCursor);
        gfx->print(strname);

        countPng = listImages.Count();
        
        if (countPng >= 46 && (countPng % 46) == 0)
        {
          xCursor = 160;
          yCursor = -5;
        }

        if (countPng >= 92 && (countPng % 92) == 0)
        {
          delay(250);
          gfx->fillScreen(BLACK);
          xCursor = 5;
          yCursor = -5;
        }
      }
    }
    root.close();
    
    yCursor += 20;
    gfx->setCursor(xCursor, yCursor);
    gfx->print("Total Images: ");
    gfx->setCursor(xCursor + 80, yCursor);
    gfx->print(countPng);

    yCursor += 20;
    gfx->setCursor(xCursor, yCursor);
    gfx->print("Booting............... ");

    delay(2000);

    xOffset = 1;
    yOffset = 1; //18;

    DrawImagePng(PNG_FILENAME);
  }
                                // currentAction
  listMode.Add("Image: ");      //      0
  listMode.Add("Brightness: "); //      1
  listMode.Add("Delay: ");      //      2

                                //currentDelay
  listDelay.Add("Sttoped");     //    0
  listDelay.Add("5 seconds");   //    1      
  listDelay.Add("15 seconds");  //    2
  listDelay.Add("30 seconds");  //    3
  listDelay.Add("60 seconds");  //    4
  
  currentAction = -1;
  showBar();
  delay(2000);  

  Serial.println("Current Index Image: ");
  Serial.println(currentIndexPng);
  Serial.println(listImages.Value(currentIndexPng));
    
  currentBackLight += BACKLIGHT_FACTOR;
  setBrightness(MINUS);

  currentIndexPng--;
  if (currentIndexPng == -1)
    currentIndexPng = countPng - 1;

  Serial.println("Current Delay: ");
  Serial.println(listDelay.Value(currentDelay));

  visibleBar = false;

  if (currentDelay == 0)
  {
    currentIndexPng++;

    if ((currentIndexPng + 1) == countPng)
      currentIndexPng = 0;

    currentImage = listImages.Value(currentIndexPng);
    DrawImagePng("/" + currentImage);
  }
    
    
  xTaskCreatePinnedToCore(taskButtonsAction, "taskButtonsAction", 4096, NULL, 1, NULL, 1);
}

void loop()
{
  String strname;
  int rc;   

  if (!visibleBar && currentDelay != 0)
  {
    currentIndexPng++;

    if ((currentIndexPng + 1) >= countPng)
      currentIndexPng = 0;  
      
    currentImage = listImages.Value(currentIndexPng);
    strname = "/" + currentImage;
    
    DrawImagePng(strname);
  }

  if(currentDelay == 2)
    delay(DELAY_15_SECONDS);
  else if (currentDelay == 3)
    delay(DELAY_30_SECONDS);
  else if (currentDelay == 4)
    delay(DELAY_60_SECONDS);
  else
    delay(DELAY_05_SECONDS);
}

void taskButtonsAction(void * parameters)
{
  while(true)
  {
    if (digitalRead(BTN_GPIO_MODE) == LOW && canDrawBar)
    {
      Serial.println("Press Button Mode");
      
      currentAction++;

      if (currentAction > 2)
        currentAction = 0;

      showBar();
      
      delay(500);
      barVisibleTime = millis();
    }
  
    if (digitalRead(BTN_GPIO_PLUS) == LOW && visibleBar)
    {
      Serial.println("Press Button Plus");
      Serial.println("Action " + listMode.Value(currentAction));

      if (currentAction == 0)
        setImage(PLUS);
          
      if (currentAction == 1)
        setBrightness(PLUS);
        
      if (currentAction == 2)
        setDelay(PLUS);
        
      delay(500);
      barVisibleTime = millis();
    }

    if (digitalRead(BTN_GPIO_MINUS) == LOW && visibleBar)
    {
      Serial.println("Press Button Minus");
      Serial.println("Action " + listMode.Value(currentAction));

      if (currentAction == 0)
        setImage(MINUS);
        
      if (currentAction == 1)
        setBrightness(MINUS);

      if (currentAction == 2)
        setDelay(MINUS);
        
      delay(500);
      barVisibleTime = millis();      
    }

    //10 seconds have passed after the last action
    if (visibleBar && millis() > (barVisibleTime + DELAY_05_SECONDS))
    {
      yOffset = 1;
      visibleBar = false;
      barVisibleTime = 0;

      if (currentDelay == 0)
        DrawImagePng("/" + currentImage);          
    }
  }
}

void setDelay(BTN_ACTION_E action)
{
  if (action == MINUS)
  {
    currentDelay--;

    if (currentDelay == -1)
      currentDelay = 0;
  }
  else
  {
    currentDelay++;

    if (currentDelay > 4)
      currentDelay = 4;
  }

  preferences.begin(APP_NAME_VALUE, false);
  preferences.putInt(KEY_DELAY_VALUE, currentDelay);
  preferences.end();

  Serial.println("Current Value Delay");
  Serial.println(listDelay.Value(currentDelay));

  showBar();
}

void setImage(BTN_ACTION_E action)
{
  if (action == MINUS)
  {
    currentIndexPng--;

    if (currentIndexPng == -1)
      currentIndexPng = countPng - 1;
  }
  else
  {
    currentIndexPng++;

    if ((currentIndexPng + 1) >= countPng)
      currentIndexPng = 0;   
  }

  currentImage = listImages.Value(currentIndexPng);
  yOffset = 18;

  showBar();
  DrawImagePng("/" + currentImage);
}

void setBrightness(BTN_ACTION_E action)
{
  if (action == MINUS)
  {
    currentBackLight -= BACKLIGHT_FACTOR;

    if (currentBackLight < BACKLIGHT_MINIUM)
      currentBackLight = BACKLIGHT_MINIUM;      
  }
  else
  {
    currentBackLight += BACKLIGHT_FACTOR;
    
    if (currentBackLight > BACKLIGHT_MAXIMUM)
      currentBackLight = BACKLIGHT_MAXIMUM;    
  }
    
  #if defined(TFT_ST7796)
    analogWrite(GFX_BL, currentBackLight);
  #endif
  #if defined(TFT_ILI9488)
    //gfx->setBrightness(0x51, currentBackLight);
  #endif

  preferences.begin(APP_NAME_VALUE, false);
  preferences.putInt(KEY_BACKLIGHT_VALUE, currentBackLight);
  preferences.end();
      
  Serial.println("Current Value BackLight");
  Serial.println(currentBackLight);

  showBar();
}

void DrawImagePng(String strname)
{
  int rc;

  Serial.println(strname);
  
  canDrawBar = false;
  rc = png.open(strname.c_str(), myOpen, myClose, myRead, mySeek, PNGDraw);

  if (rc == PNG_SUCCESS)
  {
    rc = png.decode(NULL, 0);

    Serial.printf("Draw offset: (%d, %d)", xOffset, yOffset);
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    
    png.close();
    
    preferences.begin(APP_NAME_VALUE, false);
    preferences.putInt(KEY_PNG_VALUE, currentIndexPng);
    preferences.end();
  }
  else
  {
    Serial.println("png.open() failed!");
  }  

  canDrawBar = true;
}

void showBar()
{
  //Draw bar
  for (int x = 1; x < 321; x++)
    for (int y = 1; y < 19; y++)
      gfx->drawPixel(x, y, BLACK);

  //Draw Battery
  BatteryDraw(99);

  //Draw Option
  if (currentAction != -1)
  {
    gfx->setTextColor(WHITE);
    gfx->setCursor(5, 5);
    gfx->print(listMode.Value(currentAction));

    if (currentAction == 0)
    {
      gfx->setCursor(50, 5);
      gfx->print(currentImage);
    }

    if (currentAction == 1)
    {
      gfx->setCursor(80, 5);
      gfx->print(currentBackLight);
    }

    if (currentAction == 2)
    {
      gfx->setCursor(45, 5);
      gfx->print(listDelay.Value(currentDelay));
    }    
  }

  visibleBar = true;
}

void BatteryDraw(int levelBattery)
{
  int x;
  int y;

  gfx->setTextColor(WHITE);

  if (levelBattery == 100)
    gfx->setCursor(277, 5);
  else if (levelBattery > 9)
    gfx->setCursor(283, 5);
  else
    gfx->setCursor(288, 5);
  
  gfx->print(levelBattery);

  gfx->setCursor(295, 5);
  gfx->print("%");

  for (x = 304; x < 314; x++)
    for (y = 5; y < 12; y++)
      gfx->drawPixel(x, y, WHITE);

  for (x = 314; x < 316; x++)
    for (y = 7; y < 10; y++)
      gfx->drawPixel(x, y, WHITE);
}