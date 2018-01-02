//
#ifndef ESP8266_FTP_SERVER_H
#define ESP8266_FTP_SERVER_H

//Config defines
#define FTP_MAX_CLIENTS   2
#define FTP_PORT_CONTROL  21
#define FTP_PORT_DATA     20

//Includes
#include <ESP8266WiFi.h>

enum nFtpState
{
  NFS_IDLE = 0,
  NFS_WAITFORUSERNAME,
  NFS_WAITFORPASSWORD_USER_REJECTED, //we display error after password
  NFS_WAITFORPASSWORD,
  NFS_WAITFORCOMMAND
};

enum nControlState
{
  NCS_START = 0,
  NFS_COMMAND,
  NFS_ARGUMENTS,
  NFS_END
};


struct SClientInfo
{
  bool InUse;
  nFtpState FtpState;
  String CurrentPath;
  
  nControlState ControlState;
  String Command;
  String Arguments;
  
  WiFiClient ClientConnection;

  void Reset()
  {
    InUse = false;
    FtpState = NFS_IDLE;
    CurrentPath = "/";
    ControlState = NCS_START;
  };
  SClientInfo()
  {
    Reset();
  };
};

class CFtpServer
{
public: //Constructor
  CFtpServer();
  
public: //Interface
  void    Start();
  void    Loop();

protected: //Help functions
  bool    GetEmptyClientInfo(int32_t& Pos);
  void    CheckClient(SClientInfo& Client);
  void    DisconnectClient(SClientInfo& Client);
  void    GetControlData(SClientInfo& Client);
  String  GetFirstArgument(SClientInfo& Client);
  String  ConstructPath(SClientInfo& Client);
  bool    GetFileName(String CurrentDir, String FilePath, String& FileName, bool& IsDir);
  bool    GetParentDir(String FilePath, String& ParentDir);

  void    ProcessCommand(SClientInfo& Client);
  bool    Process_USER(SClientInfo& Client);
  bool    Process_PASS(SClientInfo& Client);
  bool    Process_QUIT(SClientInfo& Client);
  bool    Process_SYST(SClientInfo& Client);
  bool    Process_FEAT(SClientInfo& Client);
  bool    Process_HELP(SClientInfo& Client);
  bool    Process_PWD(SClientInfo& Client);
  bool    Process_CDUP(SClientInfo& Client);
  bool    Process_CWD(SClientInfo& Client);
  bool    Process_MKD(SClientInfo& Client);
  bool    Process_RMD(SClientInfo& Client);
  bool    Process_TYPE(SClientInfo& Client);
  bool    Process_PASV(SClientInfo& Client);
  bool    Process_PORT(SClientInfo& Client);
  bool    Process_LIST(SClientInfo& Client);

public: //Config
  String  mServerUsername;
  String  mServerPassword;
  
protected: //Variables
  WiFiServer mFtpServer;
  WiFiServer mFtpDataServer;
  SClientInfo mClientInfo[2];
  
};

#endif

