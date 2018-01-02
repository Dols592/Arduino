/*
 
 */
#include <ESP8266WebServer.h>
#include <FS.h>

#define MAX_WIFI_CLIENT_INFO  10
#define MAX_WIFI_CLIENT_RETRIES 3

struct SWifiClientInfoItem
{
  bool enable;
  const char* ssid; // = "Fietswiel";
  const char* password; // = "power678";
  IPAddress ip; //(192, 168, 2, 150);
  IPAddress subnet; //(255, 255, 255, 0);
  IPAddress gateway; //(172, 168, 2, 254);
  IPAddress dns; //(172, 168, 2, 254);

  SWifiClientInfoItem()
  {
    enable = false;
    ssid = 0;
    password = 0;
    ip = IPAddress(0,0,0,0);
    subnet = IPAddress(0,0,0,0);
    gateway = IPAddress(0,0,0,0);
    dns = IPAddress(0,0,0,0);
  }
};

class CWifiWebBase
{
public: //Constructor
  CWifiWebBase();
  
public: //Interface
  void Setup();
  void Loop();
    
protected: //Help functions
  void LoopClient();
  void LoopAP();
  void ConnectToNext();
  void Connect();
  void EnableAP();
  void DisableAP();

protected: //Borrowed from FSBrowser
  String getContentType(String filename);
  bool handleFileRead(String path);
  static void handleFileUpload();
  static void handleFileDelete();
  static void handleFileCreate();
  static void handleFileList();

public: //Public variables
  static ESP8266WebServer mWebServer;
  SWifiClientInfoItem mWifiClientInfo[MAX_WIFI_CLIENT_INFO];
  SWifiClientInfoItem mWifiAPInfo;
 
protected: //Variables
  uint8_t mLastStatus;
  int32_t mCurrentConfigPos;
  int32_t mTotalClientRetries;
  static File mUploadFile;
};


