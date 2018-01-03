 /*
 */
#include "SoftConfig.h"
#include "WifiWebBase.h"
#include <DFC_ESP8266FtpServer.h>

CWifiWebBase gWifiWebBase;
//FtpServer ftpSrv;
DFC_ESP7266FtpServer FtpServer;

void setup() 
{
  delay(1000); //Wait until esp is ready. Without some libraries don't work well. (eg. dhcp in wifi ap)
  Serial.begin(74880); //equals as default esp 
  Serial.println();
  Serial.println();
  Serial.println("=============================================");
  Serial.println("Initialisation");
  Serial.print("Mounting FS...");

  if (!SPIFFS.begin()) 
    Serial.println("Failed !!!");
  else
    Serial.println("Mounted");

  gWifiWebBase.mWifiAPInfo.enable = true;
  gWifiWebBase.mWifiAPInfo.ssid = "DcfClock";
  gWifiWebBase.mWifiAPInfo.ip = IPAddress(192, 168, 1, 1);
  gWifiWebBase.mWifiAPInfo.subnet = IPAddress(255, 255, 255, 0);
  
  gWifiWebBase.mWifiClientInfo[1].enable = true;
  gWifiWebBase.mWifiClientInfo[1].ssid = "Fietswiel";
  gWifiWebBase.mWifiClientInfo[1].password = "power678";
  gWifiWebBase.mWifiClientInfo[1].ip = IPAddress(192, 168, 2, 150);
  gWifiWebBase.mWifiClientInfo[1].subnet = IPAddress(255, 255, 255, 0);
  gWifiWebBase.mWifiClientInfo[1].gateway = IPAddress(192, 168, 2, 254);

  gWifiWebBase.mWifiClientInfo[4].enable = true;
  gWifiWebBase.mWifiClientInfo[4].ssid = "EllipsBV";
  gWifiWebBase.mWifiClientInfo[4].password = "4Ru5ujA2";
  gWifiWebBase.mWifiClientInfo[4].ip = IPAddress(172, 16, 30, 10);
  gWifiWebBase.mWifiClientInfo[4].subnet = IPAddress(255, 255, 0, 0);
  gWifiWebBase.mWifiClientInfo[4].gateway = IPAddress(172, 16, 1, 10);
  
  gWifiWebBase.Setup();

  Serial.println("Initializing ftp");
  //ftpSrv.Begin("dols","dols");
  FtpServer.Start();

  Serial.println("Initialisation Finished");
  Serial.println("=============================================");
}

void loop() 
{
  gWifiWebBase.Loop();
  //ftpSrv.HandleFTP();
  FtpServer.Loop();
  delay(50);
}


