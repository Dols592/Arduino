/*
 */

//#include <stdint.h>
#include <c_types.h>
#include <DString.h>
#include <DLog.h>

#define DISPLAY_LOG_QUEU_SIZE 10

class DcfDisplay
{
public:
  DcfDisplay();
  void Init();
  void Loop();
  void AddLog(const DString LogMsg);
  static void AddLogString(const char* LogMsg);
  
  void DrawDisplay_All();
  void DrawDisplay_Status();
  void DrawDisplay_Log();
  void MoveUp1Pix(int32_t TX, int32_t TY, int32_t BX, int32_t BY);

protected:
  void DrawAntenna(int x, int y, int RX);

protected:
  static DcfDisplay* mDisplayPointer;
  int32_t mTxtPos;
  int32_t mTimerLastScroll;
  int32_t mTimerLastStatus;
  int32_t mTimerLastMsg;
  DString mLogQueu[DISPLAY_LOG_QUEU_SIZE];
  int32_t mLogQueuSize;
  int32_t mLogCounter;

 };

