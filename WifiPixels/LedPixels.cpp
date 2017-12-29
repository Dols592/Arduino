/*
 
 */

#include "LedPixels.h"
//#include <Adafruit_NeoPixel.h>

void CLedPixels::Init(int16_t Pin, neoPixelType Type, uint16_t NumPixels, int32_t BytesPerPixel)
{
  mValueRed = 0x20;
  mValueGreen = 0x00;
  mValueBlue = 0x00;
  mValueIntensity = 0x30;
  mValueRotationSpeed = 40;
  mValueDimSpeed = 9;
  mValueMode = 2;

  mNrOfPixels = NumPixels;
  mBytesPerPixel = BytesPerPixel;

  mStrip.updateType(Type); 
  mStrip.updateLength(NumPixels); 
  mStrip.setPin(Pin); 
  mStrip.begin(); // This initializes the NeoPixel library.
  mPixels = mStrip.getPixels();
  mStrip.show(); // Initialize all pixels to 'off'
}

void CLedPixels::LedPixelsSetVariable(String Name, String Value)
{
  Name.toUpperCase();
  int32_t ValueInt = Value.toInt();
  uint8_t ValueChar = ValueInt;
  if (Name.compareTo("RED") == 0) 
    mValueRed = ValueChar;

  if (Name.compareTo("GREEN") == 0) 
    mValueGreen = ValueChar;

  if (Name.compareTo("BLUE") == 0) 
    mValueBlue = ValueChar;

  if (Name.compareTo("INTENSITY") == 0) 
    mValueIntensity = ValueChar;

  if (Name.compareTo("ROTATIONSPEED") == 0) 
    mValueRotationSpeed = ValueChar>0?ValueChar:mValueRotationSpeed;

  if (Name.compareTo("DIMSPEED") == 0) 
    mValueDimSpeed = ValueChar>0?ValueChar:mValueDimSpeed;

  if (Name.compareTo("MODE") == 0) 
    mValueMode = ValueChar;

  if (mValueMode == 1)
  {
    int32_t Value32Red = mValueRed;
    Value32Red *= mValueIntensity;
    Value32Red /= 255;
  
    int32_t Value32Green = mValueGreen;
    Value32Green *= mValueIntensity;
    Value32Green /= 255;
  
    int32_t Value32Blue = mValueBlue;
    Value32Blue *= mValueIntensity;
    Value32Blue /= 255;
  
    for(int i=0; i<mNrOfPixels; i++)
    {
      mPixels[(i*mBytesPerPixel)+1] = (uint8_t) Value32Red;
      mPixels[(i*mBytesPerPixel)+0] = (uint8_t) Value32Green;
      mPixels[(i*mBytesPerPixel)+2] = (uint8_t) Value32Blue;
    }
    
    mStrip.show(); // Initialize all pixels to 'off'    
  }
}

void CLedPixels::LedPixelsLoop()
{
  unsigned long currentMillis = millis();
  bool LedsChanged = false;

  if (mValueMode == 2)
  {
    if (currentMillis - mLastDim >= mValueDimSpeed)
    {
      DimmLeds();
      mLastDim = currentMillis;
      LedsChanged = true;
    }  
  
    if (currentMillis - mLastRotation >= (5000/mValueRotationSpeed))
    {
      NewLed();
      mLastRotation = currentMillis;
      LedsChanged = true;
    }  
  }

  if (mValueMode == 3)
  {
    if (currentMillis - mLastRotation >= (1000/mValueRotationSpeed))
    {
      NewHueLed();
      mLastRotation = currentMillis;
      LedsChanged = true;
    }      
  }

  if (LedsChanged)
    mStrip.show();
}

void CLedPixels::NewLed()
{
  mLastLed++;
  if (mLastLed >= mNrOfPixels)
  {
    mLastLed = 0;
    mLastColor++;
  }

  switch(mLastColor)
  {
    default:
    case 0:
      mPixels[(mLastLed*mBytesPerPixel)+1] = mValueIntensity; //red   
      mLastColor = 0;
      break;
    case 1:
      mPixels[(mLastLed*mBytesPerPixel)] = mValueIntensity;  //green  
      break;
    case 2:
      mPixels[(mLastLed*mBytesPerPixel)+2] = mValueIntensity; //blue
      break;
    case 3:
      mPixels[(mLastLed*mBytesPerPixel)+1] = mValueIntensity; //red
      mPixels[(mLastLed*mBytesPerPixel)] = mValueIntensity; //green
      break;
    case 4:
      mPixels[(mLastLed*mBytesPerPixel)+1] = mValueIntensity; //red
      mPixels[(mLastLed*mBytesPerPixel)+2] = mValueIntensity; //blue
      break;
    case 5:
      mPixels[(mLastLed*mBytesPerPixel)] = mValueIntensity; //green
      mPixels[(mLastLed*mBytesPerPixel)+2] = mValueIntensity; //blue
      break;
  }
}

void CLedPixels::NewHueLed()
{
  mLastColor = (mLastColor+1)%360;
  
  for(int i=0; i<mNrOfPixels; i++)
  {
    uint32_t Hue = (mLastColor + (i*360/mNrOfPixels)) % 360;
    uint32_t Value32Red(0);
    uint32_t Value32Green(0);
    uint32_t Value32Blue(0);
    if (Hue < 120)
    {
      Value32Red = ((359-(Hue*3))*mValueIntensity*mValueRed)/91545;
      Value32Green = ((Hue*3)*mValueIntensity*mValueGreen)/91545;
    }
    else if (Hue < 240)
    {
      Hue -= 120;
      Value32Green = ((359-(Hue*3))*mValueIntensity*mValueGreen)/91545;
      Value32Blue = ((Hue*3)*mValueIntensity*mValueBlue)/91545;
    }
    else
    {
      Hue -= 240;
      Value32Blue = ((359-(Hue*3))*mValueIntensity*mValueBlue)/91545;
      Value32Red = ((Hue*3)*mValueIntensity*mValueRed)/91545;
    }

    mPixels[(i*mBytesPerPixel)+1] = (uint8_t) Value32Red;
    mPixels[(i*mBytesPerPixel)+0] = (uint8_t) Value32Green;
    mPixels[(i*mBytesPerPixel)+2] = (uint8_t) Value32Blue;
  }
}

void CLedPixels::DimmLeds()
{
  for(int i=0; i<(mNrOfPixels*mBytesPerPixel); i++)
  {
    if (mPixels[i] > 0)
      mPixels[i]--;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t CLedPixels::Hue2RGB(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return mStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return mStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return mStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
#if(0)

//Hardware Setup
#if(1)
  #define NEO1_PIN    0  // NeoPixel DATA
  #define NEO1_PTYPE NEO_GRB   // most NeoPixel products
  #define NEO1_NUMPIXELS  12
  #define NEO1_BYTESPERPIXEL 3
#endif
#if(1)
  #define NEO2_PIN   4  // NeoPixel DATA
  #define NEO2_PTYPE  NEO_GRBW  // f.e. SK6812
  #define NEO2_NUMPIXELS  139
  #define NEO2_BYTESPERPIXEL 4
#endif
#if(1)
  #define NEO3_PIN   5  // NeoPixel DATA
  #define NEO3_PTYPE  NEO_GRB  // f.e. SK6812
  #define NEO3_NUMPIXELS  50
  #define NEO3_BYTESPERPIXEL 3
#endif

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NEO1_NUMPIXELS, NEO1_PIN, NEO1_PTYPE + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NEO2_NUMPIXELS, NEO2_PIN, NEO2_PTYPE + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(NEO3_NUMPIXELS, NEO3_PIN, NEO3_PTYPE + NEO_KHZ800);

//Variables
static int32_t ValueRed = 0x20;
static int32_t ValueGreen = 0x00;
static int32_t ValueBlue = 0x00;
static int32_t ValueIntensity = 0x30;
static int32_t ValueRotationSpeed = 40;
static int32_t ValueDimSpeed = 9;
static int32_t ValueMode = 2;

static uint8_t* Pixels = NULL;
static unsigned long LastDim = 0;
static unsigned long LastRotation = 0;
int32_t LastLed = 0;
int32_t LastColor = 0;

void LedPixelsSetup()
{
  strip1.begin(); // This initializes the NeoPixel library.
  Pixels = strip1.getPixels();
  strip1.show(); // Initialize all pixels to 'off'
}

void LedPixelsSetVariable(String Name, String Value)
{
  Name.toUpperCase();
  int32_t ValueInt = Value.toInt();
  uint8_t ValueChar = ValueInt;
  if (Name.compareTo("RED") == 0) 
    ValueRed = ValueChar;

  if (Name.compareTo("GREEN") == 0) 
    ValueGreen = ValueChar;

  if (Name.compareTo("BLUE") == 0) 
    ValueBlue = ValueChar;

  if (Name.compareTo("INTENSITY") == 0) 
    ValueIntensity = ValueChar;

  if (Name.compareTo("ROTATIONSPEED") == 0) 
    ValueRotationSpeed = ValueChar>0?ValueChar:ValueRotationSpeed;

  if (Name.compareTo("DIMSPEED") == 0) 
    ValueDimSpeed = ValueChar>0?ValueChar:ValueDimSpeed;

  if (Name.compareTo("MODE") == 0) 
    ValueMode = ValueChar;

  if (ValueMode == 1)
  {
    int32_t Value32Red = ValueRed;
    Value32Red *= ValueIntensity;
    Value32Red /= 255;
  
    int32_t Value32Green = ValueGreen;
    Value32Green *= ValueIntensity;
    Value32Green /= 255;
  
    int32_t Value32Blue = ValueBlue;
    Value32Blue *= ValueIntensity;
    Value32Blue /= 255;
  
    for(int i=0; i<NEO1_NUMPIXELS; i++)
    {
      Pixels[(i*NEO1_BYTESPERPIXEL)+1] = (uint8_t) Value32Red;
      Pixels[(i*NEO1_BYTESPERPIXEL)+0] = (uint8_t) Value32Green;
      Pixels[(i*NEO1_BYTESPERPIXEL)+2] = (uint8_t) Value32Blue;
    }
    strip1.show(); // Initialize all pixels to 'off'    
  }
}

void LedPixelsLoop()
{
  unsigned long currentMillis = millis();
  bool LedsChanged = false;

  if (ValueMode == 2)
  {
    if (currentMillis - LastDim >= ValueDimSpeed)
    {
      DimmLeds();
      LastDim = currentMillis;
      LedsChanged = true;
    }  
  
    if (currentMillis - LastRotation >= (5000/ValueRotationSpeed))
    {
      NewLed();
      LastRotation = currentMillis;
      LedsChanged = true;
    }  
  }

  if (ValueMode == 3)
  {
    if (currentMillis - LastRotation >= (1000/ValueRotationSpeed))
    {
      NewHueLed();
      LastRotation = currentMillis;
      LedsChanged = true;
    }      
  }

  if (LedsChanged)
    strip1.show();
}

void NewLed()
{
  LastLed++;
  if (LastLed >= NEO1_NUMPIXELS)
  {
    LastLed = 0;
    LastColor++;
  }

  switch(LastColor)
  {
    default:
    case 0:
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)+1] = ValueIntensity; //red   
      LastColor = 0;
      break;
    case 1:
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)] = ValueIntensity;  //green  
      break;
    case 2:
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)+2] = ValueIntensity; //blue
      break;
    case 3:
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)+1] = ValueIntensity; //red
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)] = ValueIntensity; //green
      break;
    case 4:
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)+1] = ValueIntensity; //red
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)+2] = ValueIntensity; //blue
      break;
    case 5:
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)] = ValueIntensity; //green
      Pixels[(LastLed*NEO1_BYTESPERPIXEL)+2] = ValueIntensity; //blue
      break;
  }
}

void NewHueLed()
{
  LastColor = (LastColor+1)%360;
  
  for(int i=0; i<NEO1_NUMPIXELS; i++)
  {
    uint32_t Hue = (LastColor + (i*360/NEO1_NUMPIXELS)) % 360;
    uint32_t Value32Red(0);
    uint32_t Value32Green(0);
    uint32_t Value32Blue(0);
    if (Hue < 120)
    {
      Value32Red = ((359-(Hue*3))*ValueIntensity*ValueRed)/91545;
      Value32Green = ((Hue*3)*ValueIntensity*ValueGreen)/91545;
    }
    else if (Hue < 240)
    {
      Hue -= 120;
      Value32Green = ((359-(Hue*3))*ValueIntensity*ValueGreen)/91545;
      Value32Blue = ((Hue*3)*ValueIntensity*ValueBlue)/91545;
    }
    else
    {
      Hue -= 240;
      Value32Blue = ((359-(Hue*3))*ValueIntensity*ValueBlue)/91545;
      Value32Red = ((Hue*3)*ValueIntensity*ValueRed)/91545;
    }

    Pixels[(i*NEO1_BYTESPERPIXEL)+1] = (uint8_t) Value32Red;
    Pixels[(i*NEO1_BYTESPERPIXEL)+0] = (uint8_t) Value32Green;
    Pixels[(i*NEO1_BYTESPERPIXEL)+2] = (uint8_t) Value32Blue;
  }
}

void DimmLeds()
{
  for(int i=0; i<(NEO1_NUMPIXELS*NEO1_BYTESPERPIXEL); i++)
  {
    if (Pixels[i] > 0)
      Pixels[i]--;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Hue2RGB(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
#endif

