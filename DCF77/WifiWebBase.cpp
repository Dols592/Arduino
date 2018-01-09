/*
 */

#include "WifiWebBase.h"

extern CWifiWebBase gWifiWebBase;

#ifdef WIFI_USE_WEB_SERVER
  ESP8266WebServer CWifiWebBase::mWebServer(80);
#endif

File CWifiWebBase::mUploadFile;

CWifiWebBase::CWifiWebBase()
  //: mWebServer(80)
  : mLastStatus(0xff)
  , mCurrentConfigPos(0)
  , mTotalClientRetries(0)
{  
}

void CWifiWebBase::Setup() 
{
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) 
    {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %d\n", fileName.c_str(), fileSize);
    }
 
    Dir dir2 = SPIFFS.openDir("/wwwroot");
    while (dir2.next()) 
    {    
      String fileName = dir2.fileName();
      size_t fileSize = dir2.fileSize();
      Serial.printf("FS File: %s, size: %d\n", fileName.c_str(), fileSize);
    }

  WiFi.enableSTA(true);
  WiFi.enableAP(false);
  WiFi.setAutoConnect(false); //this setting is persistant and not immediat.
  WiFi.disconnect(); //disconnect if there was a autoconnect started

#ifdef WIFI_USE_WEB_SERVER
  //SERVER INIT
  mWebServer.on("/list", HTTP_GET, CWifiWebBase::handleFileList);
  mWebServer.on("/edit", HTTP_GET, []()
  {
    if(!gWifiWebBase.handleFileRead("/FSBrowser/edit.htm")) 
      gWifiWebBase.mWebServer.send(404, "text/plain", "FileNotFound");
  });
  mWebServer.on("/edit", HTTP_PUT, CWifiWebBase::handleFileCreate);
  mWebServer.on("/edit", HTTP_DELETE, CWifiWebBase::handleFileDelete);
  mWebServer.on("/edit", HTTP_POST, [](){ gWifiWebBase.mWebServer.send(200, "text/plain", ""); }, CWifiWebBase::handleFileUpload);

  mWebServer.onNotFound([](){if(!gWifiWebBase.handleFileRead(gWifiWebBase.mWebServer.uri()))gWifiWebBase.mWebServer.send(404, "text/plain", "FileNotFound");});

  mWebServer.begin();
#endif
}

void CWifiWebBase::Loop()
{
  WiFiMode_t WifiMode = WiFi.getMode();
  if (WifiMode == WIFI_STA || WifiMode == WIFI_AP_STA)
    LoopClient();
  if (WifiMode == WIFI_AP || WifiMode == WIFI_AP_STA)
    LoopAP();
    
#ifdef WIFI_USE_WEB_SERVER
  mWebServer.handleClient();
#endif
}

void CWifiWebBase::LoopClient()
{
  uint8_t CurrentStatus = WiFi.status();

  if (CurrentStatus != mLastStatus)
  {
    switch (CurrentStatus)
    {
      case WL_IDLE_STATUS: //nothing is done
        Serial.println("Wifi idle.");
        break;
      case WL_NO_SSID_AVAIL: //wifi network not found
        Serial.println("Wifi network not available.");
        break;
      case WL_CONNECTED:
        Serial.print("Wifi connected, IP address: ");
        Serial.println(WiFi.localIP());  
        break;
      case WL_CONNECT_FAILED: //connection failed, login incorrect, etc
        Serial.println("Wifi connection failed.");
        break;
//      case WL_DISCONNECTED: //connecting or connection lost
//        Serial.println("Wifi connecting.");
//        wait for connection or fail
//        break;
    }
    mLastStatus = CurrentStatus;
  }
  
  if (CurrentStatus == WL_IDLE_STATUS || CurrentStatus == WL_NO_SSID_AVAIL || CurrentStatus == WL_CONNECT_FAILED )
    ConnectToNext();
}

void CWifiWebBase::LoopAP()
{
}

void CWifiWebBase::ConnectToNext()
{
  do
  {
    mCurrentConfigPos++;
    if (mCurrentConfigPos >= MAX_WIFI_CLIENT_INFO)
    {
      mCurrentConfigPos = 0;
      mTotalClientRetries++;
      if (mTotalClientRetries >= MAX_WIFI_CLIENT_RETRIES)
      {
        EnableAP();
      }
      return;
    }
  } while (!mWifiClientInfo[mCurrentConfigPos].enable);

  Connect();
}

void CWifiWebBase::Connect()
{
  WiFi.disconnect();
  if (mWifiClientInfo[mCurrentConfigPos].enable)
  {
    WiFi.config(mWifiClientInfo[mCurrentConfigPos].ip, mWifiClientInfo[mCurrentConfigPos].gateway, mWifiClientInfo[mCurrentConfigPos].subnet);
    WiFi.begin(mWifiClientInfo[mCurrentConfigPos].ssid, mWifiClientInfo[mCurrentConfigPos].password);    

    Serial.print("Wifi connecting to ");
    Serial.println(mWifiClientInfo[mCurrentConfigPos].ssid);
  }
}

void CWifiWebBase::EnableAP()
{
  WiFi.disconnect();
  WiFi.enableSTA(false);
  WiFi.enableAP(true);

  WiFi.softAPConfig(mWifiAPInfo.ip, mWifiAPInfo.gateway, mWifiAPInfo.subnet);
  WiFi.softAP(mWifiAPInfo.ssid, mWifiAPInfo.password);

  Serial.print("Wifi Starting AP \"");
  Serial.print(mWifiAPInfo.ssid);  
  Serial.print("\" IP address : ");
  Serial.println(WiFi.softAPIP());
}

void CWifiWebBase::DisableAP()
{
  WiFi.softAPdisconnect();

  WiFi.enableSTA(true);
  WiFi.enableAP(false);
}

//============================================================================================

//#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <FS.h>

//holds the current upload
//File fsUploadFile;

#ifdef WIFI_USE_WEB_SERVER

String CWifiWebBase::getContentType(String filename)
{
  if(mWebServer.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool CWifiWebBase::handleFileRead(String path)
{
  if (path.endsWith("/"))
  {
    if (handleFileRead(path + "index.html"))
      return true;
    else
      return handleFileRead(path + "index.htm");
  }

  String contentType = getContentType(path);

  if(SPIFFS.exists(path))
    path = path;
  else if(SPIFFS.exists("/wwwroot"+path))
    path = "/wwwroot"+path;
  else if(SPIFFS.exists(path+".gz"))
    path = path+".gz";
  else if(SPIFFS.exists("/wwwroot"+path+".gz"))
    path = "/wwwroot"+path+".gz";
  else
    return false;

  Serial.printf("Try to stream file %s\n", path.c_str());

  File file = SPIFFS.open(path, "r");
  size_t sent = mWebServer.streamFile(file, contentType);
  file.close();
  return true;
}

void CWifiWebBase::handleFileUpload()
{
  if(mWebServer.uri() != "/edit") return;
  HTTPUpload& upload = mWebServer.upload();
  if(upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    mUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  }
  else if(upload.status == UPLOAD_FILE_WRITE)
  {
    if(mUploadFile)
      mUploadFile.write(upload.buf, upload.currentSize);
  }
  else if(upload.status == UPLOAD_FILE_END)
  {
    if(mUploadFile)
      mUploadFile.close();
  }
}

void CWifiWebBase::handleFileDelete()
{
  if(mWebServer.args() == 0) return mWebServer.send(500, "text/plain", "BAD ARGS");
  String path = mWebServer.arg(0);

  if(path == "/")
    return mWebServer.send(500, "text/plain", "BAD PATH");

  if(!SPIFFS.exists(path))
    return mWebServer.send(404, "text/plain", "FileNotFound");
  
  SPIFFS.remove(path);
  mWebServer.send(200, "text/plain", "");
  path = String();
}

void CWifiWebBase::handleFileCreate()
{
  if(mWebServer.args() == 0)
    return mWebServer.send(500, "text/plain", "BAD ARGS");
    
  String path = mWebServer.arg(0);

  if(path == "/")
    return mWebServer.send(500, "text/plain", "BAD PATH");

  if(SPIFFS.exists(path))
    return mWebServer.send(500, "text/plain", "FILE EXISTS");
  
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return mWebServer.send(500, "text/plain", "CREATE FAILED");
  
  mWebServer.send(200, "text/plain", "");
  path = String();
}

void CWifiWebBase::handleFileList() 
{
  if(!mWebServer.hasArg("dir")) {mWebServer.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = mWebServer.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next())
  {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  mWebServer.send(200, "text/json", output);
}

#endif
/*
void setup(void)
{

//  SPIFFS.begin();
//  {
//    Dir dir = SPIFFS.openDir("/");
//    while (dir.next()) {    
//      String fileName = dir.fileName();
//      size_t fileSize = dir.fileSize();
//      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
//    }
//    DBG_OUTPUT_PORT.printf("\n");
//  }
  
//  MDNS.begin(host);
//  DBG_OUTPUT_PORT.print("Open http://");
//  DBG_OUTPUT_PORT.print(host);
//  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");
  
  
  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);

  //load editor
  server.on("/edit", HTTP_GET, []()
  {
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]()
  {
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
}
 */
 
