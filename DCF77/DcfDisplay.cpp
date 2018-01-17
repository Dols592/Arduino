/*
 */
 
#include "DcfDisplay.h"
#include <DString.h>

#include <U8g2lib.h>
#include <U8x8lib.h>
#include <ESP8266WebServer.h>

#include "XbmFiles.h"

//U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0); // hardware
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0); // hardware

DcfDisplay* DcfDisplay::mDisplayPointer = NULL;

DcfDisplay::DcfDisplay()
{
  mDisplayPointer = this;
}
void DcfDisplay::Init()
{
  u8g2.begin();
  u8g2.clear();
  u8g2.setContrast(255);

  DrawDisplay_All();

  mTxtPos = 64;
  mTimerLastScroll = 0;
  mTimerLastStatus = 0;
  mTimerLastMsg = millis();
  mLogQueuSize = 0;
  mLogCounter = 0;
}

void DcfDisplay::Loop()
{
  bool Upload(false);
  int32 CurTime = millis();

//  int32_t StopwatchMsg = CurTime - mTimerLastMsg;
//  if (StopwatchMsg > 1000)
//  {
//    AddLogString(DString::FormatNew("New Log Message %d", mLogCounter).c_str());
//    mTimerLastMsg += 1000;
//  }
  
  int32_t StopwatchScroll = CurTime - mTimerLastScroll;
  if (StopwatchScroll > 40)
  {

    DrawDisplay_Log();
    mTimerLastScroll = CurTime;
    Upload = true;
  }
  
  int32 StopwatchStatus = CurTime - mTimerLastStatus;
  if (StopwatchStatus > 1000)
  {
    DrawDisplay_Status();    
    mTimerLastStatus = CurTime;
    Upload = true;
  }

  if (Upload)
  {
    u8g2.sendBuffer();    
  }

}

void DcfDisplay::AddLog(const DString LogMsg)
{
  mLogCounter++;

  if (mLogQueuSize >= DISPLAY_LOG_QUEU_SIZE)
    return;

  DString NewLogMsg = LogMsg;
  if (LogMsg.length() > 20)
  {
    NewLogMsg = NewLogMsg.Mid(0, 23);
//    NewLogMsg += "_";
  }
    
  mLogQueu[mLogQueuSize++] = NewLogMsg;
}

void DcfDisplay::AddLogString(const char* LogMsg)
{
  DcfDisplay::mDisplayPointer->AddLog(LogMsg);
}

void DcfDisplay::DrawDisplay_All()
{
  u8g2.clearBuffer();
  u8g2.drawXBM( 9, 17, 112, 37, Dols_xbm);  
  u8g2.drawRFrame(0, 8, 128, 56, 7);
  DrawDisplay_Status();
  
  u8g2.sendBuffer();  
}

void DcfDisplay::DrawDisplay_Status()
{
  //to do: erase area
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(30, 6,WiFi.localIP().toString().c_str());
  DrawAntenna(5, 0, (WiFi.RSSI() + 105 )/5);
}

void DcfDisplay::DrawDisplay_Log()
{
  if (mLogQueuSize <= 0)
    return;

  if (mTxtPos > 61)
  {
    MoveUp1Pix(5, 10, 123, 63);
    mTxtPos--;
  }

  if (mTxtPos > 61)
  {
    //copy into temp buffer
    uint32_t Height = u8g2.getBufferTileHeight(); //number of lines
    uint32_t Width = u8g2.getBufferTileWidth()*8; //number of pixel per line
    uint8_t* Buf = u8g2.getBufferPtr() + ((Height-1) * Width);

    //copy to temporary buffer, en ease bottom
    uint8_t TmpBuf[Width];
    memcpy(TmpBuf, Buf, Width);
    memset(Buf, 0, Width);

    //write text on visible part
    u8g2.drawStr(5, 62, mLogQueu[0].c_str());

    //move down the visible part and arase botton 2 rows
    int32_t BitShift = mTxtPos - 62;
    for (int32_t x=0; x<128; x++)
    {
       uint8_t NewByte = *(Buf+x);
       NewByte = NewByte << BitShift;
       NewByte = NewByte & 0x3F;
       NewByte = NewByte | TmpBuf[x];
       *(Buf+x) = NewByte;
    }
  }
  else
  {
    u8g2.drawStr(5, mTxtPos, mLogQueu[0].c_str());
    for (int32_t i=1;i<mLogQueuSize; i++)
      mLogQueu[i-1] = mLogQueu[i];

    mLogQueuSize--;
    mTxtPos += 7;
  }
}

void DcfDisplay::MoveUp1Pix(int32_t TX, int32_t TY, int32_t BX, int32_t BY)
{
  uint32_t Height = u8g2.getBufferTileHeight()*8; //number of lines
  uint32_t Width = u8g2.getBufferTileWidth()*8; //number of pixel per line
  uint8_t* Buf = u8g2.getBufferPtr();

  uint32_t TopRowByte = TY / 8;
  uint32_t TopRowBit = TY % 8;
  uint32_t BottomRowByte = BY / 8;
  uint32_t BottomRowBit = BY % 8;
  if (BottomRowBit == 0)
  {
    BottomRowByte--;
    BottomRowBit = 8;
  }

  //Top row of bytes
  if (TopRowBit > 0)
  {
    uint8_t FixMask = 0xFF << TopRowBit;
    for (int32_t x=TX; x<BX; x++)
    {
      uint8_t* BufW = Buf + (TopRowByte * Width) + x;
      uint8_t OrgByte = *BufW;
      uint8_t FixBits = OrgByte & ~FixMask;
      uint8_t NewBit = (*(BufW+Width)) << 7;
      
      uint8_t NewByte = (OrgByte >> 1) & FixMask;
      NewByte = NewByte | FixBits | NewBit;
      *BufW = NewByte;
    }

    TopRowByte++;
  }

  //middle bytes
  for (int32_t y=TopRowByte; y<BottomRowByte; y++)
  {
    for (int32_t x=TX; x<BX; x++)
    {
      uint8_t* BufW = Buf + (y * Width) + x;
      uint8_t OrgByte = *BufW;
      uint8_t NewBit = (*(BufW+Width)) << 7;      
      uint8_t NewByte = (OrgByte >> 1);
      NewByte = NewByte | NewBit;
      *BufW = NewByte;
    }
  }

  
  //end row of bytes
  if (BottomRowBit > 1)
  {
    uint8_t FixMask = 0xFF << BottomRowBit;
    uint8_t BlankMask = 0xFF << (BottomRowBit-1);
    for (int32_t x=TX; x<BX; x++)
    {
      
      uint8_t* BufW = Buf + (BottomRowByte * Width) + x;
      uint8_t OrgByte = *BufW;
      uint8_t FixBits = OrgByte & FixMask;
      uint8_t NewByte = (OrgByte >> 1) & ~BlankMask;
      NewByte = NewByte | FixBits;
      *BufW = NewByte;
    }
  }
}

void DcfDisplay::DrawAntenna(int x, int y, int RX)
{
  u8g2.drawXBM( x, y, 8, 7, AntennaSymbol_xbm);

  if (RX <0)
  {
    
  }
  else if (RX < 4)
  {
    u8g2.drawXBM( x+4, y, RX*2, 7, AntennaBars1_xbm);
  }
  else if (RX < 8)
  {
    u8g2.drawXBM( x+4, y, 8, 7, AntennaBars1_xbm);
    u8g2.drawXBM( x+12, y, (RX-4)*2, 7, AntennaBars2_xbm);
  }
}

