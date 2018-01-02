/*
*/

#include "ESP8266FtpServer.h"

//Config defines
#define FTP_DEBUG


//library includes
#include <ESP8266WiFi.h>

//debug
#undef DBG
#undef DBGLN
#undef DBGF

#ifdef FTP_DEBUG
  #define DBG(...) Serial.print( __VA_ARGS__ )
  #define DBGLN(...) Serial.println( __VA_ARGS__ )
  #define DBGF(...) Serial.printf( __VA_ARGS__ )
#else
  #define DBG(...)
  #define DBGLN(...)
  #define DBGF(...)
#endif

CFtpServer::CFtpServer()
  : mFtpServer( FTP_PORT_CONTROL )
  , mFtpDataServer( FTP_PORT_DATA )
{
}

void CFtpServer::Start()
{
#if(0)
  DBGLN("=============================================");
  DBGLN("Testing");
  String ParentDir = "";
  String FilePath = "////";
  GetParentDir(FilePath, ParentDir);
  DBGF("FilePath \"%s\"\r\n", FilePath.c_str());
  DBGF("ParentDir \"%s\"\r\n", ParentDir.c_str());
  DBGLN("=============================================");
#endif
#if(1)
  bool    GetParentDir(String FilePath, String& ParentDir);

#endif
  mFtpServer.begin();
  mFtpDataServer.begin();
}

void CFtpServer::Loop()
{
  if (mFtpServer.hasClient())
  {
    int32_t Pos = -1;
    if (!GetEmptyClientInfo(Pos))
    {
      WiFiClient OverflowClient = mFtpServer.available();
      if (!OverflowClient.connected()) //Client disconnected before we could handle
      {
        DBGLN("A new ftp connection disconnected before we could handle.");
        return;
      }
      OverflowClient.println( "10068 Too many users !");
      OverflowClient.flush();
      OverflowClient.stop();
      return;
    }

    //send welcome message
    mClientInfo[Pos].ClientConnection = mFtpServer.available();
    if (!mClientInfo[Pos].ClientConnection.connected()) //Client disconnected before we could handle
    {
      DBGLN("A new data connection disconnected before we could handle.");
      return;
    }
    mClientInfo[Pos].FtpState = NFS_WAITFORUSERNAME;
    mClientInfo[Pos].ClientConnection.println( "220 --==[ Welcome ]==--");
  }

  for (int32_t i=0; i<FTP_MAX_CLIENTS; i++)
  {
    if (mClientInfo[i].InUse)
    {
      CheckClient(mClientInfo[i]);
    }
  }

  if (mFtpDataServer.hasClient())
  {
    WiFiClient C = mFtpDataServer.available();
    if (!C.connected())
      return;

    IPAddress Lip = C.localIP();
    IPAddress Rip = C.remoteIP();
    uint16_t Lport = C.localPort();
    uint16_t Rport = C.remotePort();
    DBG("Local:");
    DBG(C.localIP());
    DBG(":");
    DBG(Lport);
    DBG("  ");
    DBG(Rip);
    DBG(":");
    DBG(Rport);
    DBGLN("");
  }
}

bool CFtpServer::GetEmptyClientInfo(int32_t& Pos)
{
  for (int32_t i=0; i<FTP_MAX_CLIENTS; i++)
  {
    if (!mClientInfo[i].InUse)
    {
      Pos = i;
      mClientInfo[i].InUse = true;
      return true;
    }
  }

  return false;
}

void CFtpServer::CheckClient(SClientInfo& Client)
{
  //is still connected?
  if (!Client.ClientConnection.connected())
    DisconnectClient(Client);

  //Check Data Connection
  
  //Check for new control data
  GetControlData(Client);
}

void CFtpServer::DisconnectClient(SClientInfo& Client)
{
  if (Client.ClientConnection.connected())
  {
    Client.ClientConnection.println("221 Goodbye");
    Client.ClientConnection.flush();
  }
  Client.ClientConnection.stop();
  Client.Reset();
}

void CFtpServer::GetControlData(SClientInfo& Client)
{
  while(1)
  {
    int32_t NextChr = Client.ClientConnection.read();
    if (NextChr < 0 || NextChr > 0xFF)
      break;

    //<CR> or <LF>
    if (NextChr == '\r' || NextChr == '\n')
    {
      if (Client.ControlState == NCS_START) //initial spaces
        continue;

      ProcessCommand(Client);
      Client.ControlState = NCS_START;
      continue;
    }

    // unwanted characters (control ascii and high-ascii)
    if (NextChr < 0x20 || NextChr > 0x7F)
      continue;

    //space seperator
    if (NextChr == ' ')
    {
      if (Client.ControlState == NCS_START) //initial spaces
        continue;

      if (Client.ControlState == NFS_COMMAND) //initial spaces
      {
        Client.ControlState = NFS_ARGUMENTS;
        Client.Arguments = "";
        continue;  
      }
    }
    
    if (Client.ControlState == NCS_START)
    {
      Client.ControlState = NFS_COMMAND;
      Client.Command = "";
    }

    if (Client.ControlState == NFS_COMMAND)
      Client.Command += (char) NextChr;      

    if (Client.ControlState == NFS_ARGUMENTS)
      Client.Arguments += (char) NextChr;      
  }
}

String CFtpServer::GetFirstArgument(SClientInfo& Client)
{
  int32_t Start = 0;
  int32_t End = Client.Arguments.length();
  for (int32_t i = End-1; i>= 0; i--)
  {
    if (Client.Arguments.charAt(i) == ' ')
      End = i;
  }

  return Client.Arguments.substring(Start, End);
}

String CFtpServer::ConstructPath(SClientInfo& Client)
{
  String Path = GetFirstArgument(Client);
  Path.replace("\\", "/");

  if (Client.Arguments.length() == 0)
    Path = "/";
  else if (!Client.Arguments.startsWith("/")) //its a absolute path
    Path = Client.CurrentPath + Path;

  Path += "/";
  while (Path.indexOf("//") >= 0)
    Path.replace("//", "/");

  return Path;
}

// return false if filepath is not in current dir
// Stores the filename or subdirectory in FileName.
bool CFtpServer::GetFileName(String CurrentDir, String FilePath, String& FileName, bool& IsDir)
{
  int32_t FilePathSize = FilePath.length();
  int32_t CurrentDirSize = CurrentDir.length();
  
  if (FilePathSize <= CurrentDirSize)
    return false;

  if (FilePath.indexOf(CurrentDir) != 0)
    return false;

  int32_t NextSlash = FilePath.indexOf('/', CurrentDirSize);
  
  //Check if there's more after the slash.
  //but if it's ends on a slash it's still a file with it's name ending on a /
  //That's SPIFFS specific, since it does not has directories.
  if (NextSlash < 0 || FilePathSize == NextSlash+1)
  {
    FileName = FilePath.substring(CurrentDirSize);
    IsDir = false;
  }
  else
  {
    FileName = FilePath.substring(CurrentDirSize, NextSlash);
    IsDir = true;
  }
  
  return true;
}

bool CFtpServer::GetParentDir(String FilePath, String& ParentDir)
{
  String Path = FilePath;
  Path.replace("\\", "/");
  while (Path.indexOf("//") >= 0)
    Path.replace("//", "/");

  if (Path.endsWith("/"))
    Path.remove(Path.length()-1);

  int32_t Pos = -1;
  int32_t LastSlash = Path.indexOf('/');
  while (LastSlash >= 0)
  {
    DBGF("LastSlash \"%d\"\r\n", LastSlash);
    Pos = LastSlash;
    LastSlash = Path.indexOf('/', LastSlash+1);
  }

  if (Pos < 0)
  {
    ParentDir = FilePath;
    return false;
  }

  ParentDir = FilePath.substring(0, Pos+1);  
  return true;
}

void CFtpServer::ProcessCommand(SClientInfo& Client)
{
  //preprocess
  Client.Command.trim();
  Client.Arguments.trim();
  const String& cmd(Client.Command); //for easy access

  if (cmd.equalsIgnoreCase("USER") && Process_USER(Client)) return;
  if (cmd.equalsIgnoreCase("PASS") && Process_PASS(Client)) return;
  if (Client.FtpState < NFS_WAITFORCOMMAND)
  {
    Client.ClientConnection.println( "530 Login needed.");
    return;
  }

  if (cmd.equalsIgnoreCase("QUIT") && Process_QUIT(Client)) return;
  if (cmd.equalsIgnoreCase("SYST") && Process_SYST(Client)) return;
  if (cmd.equalsIgnoreCase("FEAT") && Process_FEAT(Client)) return;
  if (cmd.equalsIgnoreCase("HELP") && Process_HELP(Client)) return;
  if (cmd.equalsIgnoreCase("PWD") && Process_PWD(Client)) return;
  if (cmd.equalsIgnoreCase("CDUP") && Process_PWD(Client)) return;
  if (cmd.equalsIgnoreCase("CWD") && Process_PWD(Client)) return;
  if (cmd.equalsIgnoreCase("MKD") && Process_PWD(Client)) return;
  if (cmd.equalsIgnoreCase("RMD") && Process_PWD(Client)) return;
  if (cmd.equalsIgnoreCase("TYPE") && Process_TYPE(Client)) return;
  if (cmd.equalsIgnoreCase("PASV") && Process_PASV(Client)) return;
  if (cmd.equalsIgnoreCase("PORT") && Process_PORT(Client)) return;
  if (cmd.equalsIgnoreCase("LIST") && Process_LIST(Client)) return;

  Client.ClientConnection.printf( "500 Unknown command %s.\n\r", cmd.c_str());  
}

bool CFtpServer::Process_USER(SClientInfo& Client)
{
  if (Client.FtpState > NFS_WAITFORPASSWORD)
  {
    Client.ClientConnection.println( "530 Changing user is not allowed.");
    return true;
  }
  
  //check if user exists
  bool UsernameOK(false);
  if (mServerUsername.length() == 0) //we accept every username
    UsernameOK = true;
  else if (mServerUsername.equalsIgnoreCase(Client.Arguments))
    UsernameOK = true;

  //username is wrong, we request password, but always will be rejected.
  if (!UsernameOK)
  {
    Client.ClientConnection.println( "331 OK. Password required");
    Client.FtpState = NFS_WAITFORPASSWORD_USER_REJECTED;
    return true;
  }

  if (mServerPassword.length() == 0) //we don't need a password
  {
    Client.ClientConnection.println( "230 OK.");
    Client.FtpState = NFS_WAITFORCOMMAND;
    return true;    
  }
  else
  {
    Client.ClientConnection.println( "331 OK. Password required");
    Client.FtpState = NFS_WAITFORPASSWORD;
    return true;
  }
}

bool CFtpServer::Process_PASS(SClientInfo& Client)
{
  if (Client.FtpState <= NFS_WAITFORUSERNAME)
  {
    Client.ClientConnection.println( "503 Login with USER first.");
    return true;
  }
  if (Client.FtpState > NFS_WAITFORPASSWORD)
  {
    Client.ClientConnection.println( "230 Already logged in.");
    return true;
  }

  if (Client.FtpState == NFS_WAITFORPASSWORD_USER_REJECTED || !mServerPassword.equals(Client.Arguments))
  {
    Client.ClientConnection.println( "530 Username/Password wrong.");
    DisconnectClient(Client);
    return true;
  }
  
  Client.ClientConnection.println( "230 Logged in.");
  Client.FtpState = NFS_WAITFORCOMMAND;
  return true;
}

bool CFtpServer::Process_QUIT(SClientInfo& Client)
{
  DisconnectClient(Client);
  return true;  
}

bool CFtpServer::Process_SYST(SClientInfo& Client)
{
  Client.ClientConnection.println( "215 UNIX esp8266.");
  return true;
}

bool CFtpServer::Process_FEAT(SClientInfo& Client)
{
  Client.ClientConnection.println( "211 No Features.");
  return true;
}


bool CFtpServer::Process_HELP(SClientInfo& Client)
{
  Client.ClientConnection.println( "214 Ask the whizzkid for help.");
  return true;
}

bool CFtpServer::Process_PWD(SClientInfo& Client)
{
  Client.ClientConnection.printf( "257 \"%s\" Current Directory.\r\n", Client.CurrentPath.c_str());
  return true;
}

bool CFtpServer::Process_CDUP(SClientInfo& Client)
{
  String NewPath;
  GetParentDir(Client.CurrentPath, NewPath);

  Client.CurrentPath = NewPath;
  Client.ClientConnection.printf( "250 Directory successfully changed.\r\n");  
  return true;
}

bool CFtpServer::Process_CWD(SClientInfo& Client)
{
  String NewPath = GetFirstArgument(Client);

  //check if path exist
  
  Client.CurrentPath = NewPath;
  Client.ClientConnection.printf( "250 Directory successfully changed.\r\n");  
  return true;
}

bool CFtpServer::Process_MKD(SClientInfo& Client)
{
  return false;
}

bool CFtpServer::Process_RMD(SClientInfo& Client)
{
  return false;
}

bool CFtpServer::Process_TYPE(SClientInfo& Client)
{
  const String& arg(Client.Arguments); //for easy access

  if (arg.equalsIgnoreCase("A"))
    Client.ClientConnection.println( "200 TYPE is now ASII.");
  else if (arg.equalsIgnoreCase("I"))
    Client.ClientConnection.println( "200 TYPE is now 8-bit binary.");
  else
    Client.ClientConnection.println( "504 Unknow TYPE.");

  return true;
}

bool CFtpServer::Process_PASV(SClientInfo& Client)
{
  IPAddress LocalIP = Client.ClientConnection.localIP();
  Client.ClientConnection.println( "227 Entering Passive Mode ("+ String(LocalIP[0]) + "," + String(LocalIP[1])+","+ String(LocalIP[2])+","+ String(LocalIP[3])+","+String( FTP_PORT_DATA >> 8 ) +","+String ( FTP_PORT_DATA & 255 )+").");

  return true;
}

bool CFtpServer::Process_PORT(SClientInfo& Client)
{
  return false;
}

bool CFtpServer::Process_LIST(SClientInfo& Client)
{
  return true;
  Client.ClientConnection.println( "150 Accepted data connection.");
  Client.ClientConnection.println( "226 0 matches total.");
//"425 No data connection"
//"550 Can't open directory " + String(mCwdName)
  return true;
}


