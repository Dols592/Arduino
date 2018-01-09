/*
*/

#include "SoftConfig.h"
#include "WifiWebBase.h"
#include <DFC_ESP8266FtpServer.h>
#include <DFC_ESP8266EasyWebServer.h>

CWifiWebBase gWifiWebBase;
//FtpServer ftpSrv;
DFC_ESP8266FtpServer FtpServer;
DFC_ESP8266EasyWebServer WebServer;

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

  FSInfo fs_info;
  SPIFFS.info(fs_info);
  Serial.printf("   TotalBytes: %d\r\n", fs_info.totalBytes);
  Serial.printf("    usedBytes: %d\r\n", fs_info.usedBytes);
  Serial.printf("    blockSize: %d\r\n", fs_info.blockSize);
  Serial.printf("     pageSize: %d\r\n", fs_info.pageSize);
  Serial.printf(" maxOpenFiles: %d\r\n", fs_info.maxOpenFiles);
  Serial.printf("maxPathLength: %d\r\n", fs_info.maxPathLength);


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
  FtpServer.Init();

  Serial.println("Initializing web");
  WebServer.Init();

  Serial.println("Initialisation Finished");
  Serial.println("=============================================");
}

void loop()
{
  gWifiWebBase.Loop();
  //ftpSrv.HandleFTP();
  FtpServer.Loop();
  WebServer.Loop();
}


