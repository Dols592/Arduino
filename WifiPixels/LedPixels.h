/*
 
 */
#include <Adafruit_NeoPixel.h>

class CLedPixels
{
public: //Interface
  void Init(int16_t Pin, neoPixelType Type, uint16_t NumPixels, int32_t BytesPerPixel);
  void LedPixelsSetVariable(String Name, String Value);
  void LedPixelsLoop();

protected: //Help functions
  void NewLed();
  void NewHueLed();
  void DimmLeds();
  uint32_t Hue2RGB(byte WheelPos);

public: //Settings
  int32_t mValueRed = 0x20;
  int32_t mValueGreen = 0x00;
  int32_t mValueBlue = 0x00;
  int32_t mValueIntensity = 0x30;
  int32_t mValueRotationSpeed = 40;
  int32_t mValueDimSpeed = 9;
  int32_t mValueMode = 2;

protected: //setup Variables
  int32_t mBytesPerPixel;
  int32_t mNrOfPixels;

protected: //Variables
  Adafruit_NeoPixel mStrip;
  uint8_t* mPixels = NULL;
  unsigned long mLastDim = 0;
  unsigned long mLastRotation = 0;
  int32_t mLastLed = 0;
  int32_t mLastColor = 0;
};

