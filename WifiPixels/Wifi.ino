/*
 */

#include <ESP8266WiFi.h>

#if(1)
const char* ssid = "Fietswiel";
const char* password = "power678";
IPAddress ip(192, 168, 2, 150);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(172, 168, 2, 254);

#endif

#if(0)
const char* ssid = "EllipsBV";
const char* password = "4Ru5ujA2";
IPAddress ip(172, 16, 30, 10);
IPAddress subnet(255, 255, 0, 0);
IPAddress gateway(172, 16, 1, 10);
#endif

#define CONNECTION_TIMEOUT     5000

WiFiServer server(80);
WiFiClient ConnectedClient;
bool Connected(false);
bool HTTPClient(false);
unsigned long ConnectTime(0);

void WifiSetup() 
{
  //Disabled the predefined AP of the microcontroller !
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(100);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
}

void WifiLoop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    if (Connected)
    {
      Serial.println("WiFi Disconnected !");
      server.close();
      Connected = false;
    }
    return;
  }

  if (!Connected)
  {
    Serial.print("WiFi Connected: ");    
    Serial.println(WiFi.localIP());
    server.begin();
    Connected = true;
  }

  // Check if a client has connected
  if (!HTTPClient)
  {
    ConnectedClient = server.available();
    if (!ConnectedClient) 
    {
      return;
    }
    Serial.print("New Client: ");
    Serial.println(ConnectedClient.remoteIP());
    ConnectTime = millis();
    HTTPClient = true;
  }

  if (!ConnectedClient.connected())
  {
    Serial.println("Client Disconnected.");
    HTTPClient = false;
    return;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - ConnectTime >= CONNECTION_TIMEOUT)
  {
    Serial.println("Timeout! Client Disconnected.");
    ConnectedClient.stop();
    HTTPClient = false;
    delay(10);
    return;
  }

  //Check if there's data received.
  int32_t BytesReceivedCount = ConnectedClient.available();
  if (BytesReceivedCount < 12)
    return;

  String Page = ConnectedClient.readStringUntil('\r');
  Page.trim();
  ConnectedClient.flush();

  if (Page.indexOf("GET ") != 0 || Page.indexOf("HTTP") < 6 )
  {
    Serial.println("Invalid HTTP! Client Disconnected.");
    ConnectedClient.stop();
    HTTPClient = false;
    delay(10);
    return;    
  }

  Page = Page.substring(5, Page.indexOf("HTTP"));
  Page.trim();
  
  int32_t SeperatorPage = Page.indexOf("/");
  int32_t Channel = 1;
  if (SeperatorPage > 0)
  {
    String ChannelString = Page.substring(0, SeperatorPage);
    ChannelString.toUpperCase();
    Serial.print("Channel : ");    
    Serial.println(ChannelString);

    Page = Page.substring(SeperatorPage+1);
    if (ChannelString.compareTo("METER") == 0) 
      Channel = 2;
    if (ChannelString.compareTo("XMAS") == 0) 
      Channel = 3;
  }
  Serial.println(Channel);
  
  int32_t Seperator = Page.indexOf("=");
  String Name = Page;
  String Value = "";
  if (Seperator >= 0)
  {
    Name = Page.substring(0, Seperator);
    Name.trim();
    Value = Page.substring(Seperator+1);
    Name.trim();
  }
 
  Serial.print("Name : ");    
  Serial.println(Name);
  Serial.print("Value : ");    
  Serial.println(Value);

  if (Name.length() > 1)
  {
    if (Channel == 1)
      LedPixels1.LedPixelsSetVariable(Name, Value);
    if (Channel == 2)
      LedPixels2.LedPixelsSetVariable(Name, Value);
    if (Channel == 3)
      LedPixels3.LedPixelsSetVariable(Name, Value);
    WifiSendOK();
  }
  else
  {
    WifiSendFullPage(Channel);
  }

  Serial.println("Responds send. Client Disconnected.");
  ConnectedClient.stop();
  HTTPClient = false;
  delay(10);
}

void WifiSendOK()
{
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "</html>\n";

  ConnectedClient.print(s);
  Serial.println("Sending OK only.");
}

void WifiSendFullPage(int32_t Channel)
{
  int32_t ValueRed = 0;
  int32_t ValueBlue = 0;
  int32_t ValueGreen = 0;
  int32_t ValueIntensity = 0;
  int32_t ValueRotationSpeed = 0;
  int32_t ValueDimSpeed = 0;
  int32_t ValueMode = 0;  
  if (Channel == 1)
  {
    ValueRed = LedPixels1.mValueRed;
    ValueBlue = LedPixels1.mValueBlue;
    ValueGreen = LedPixels1.mValueGreen;
    ValueIntensity = LedPixels1.mValueIntensity;
    ValueRotationSpeed = LedPixels1.mValueRotationSpeed;
    ValueDimSpeed = LedPixels1.mValueDimSpeed;
    ValueMode = LedPixels1.mValueMode;
  }
  if (Channel == 2)
  {
    ValueRed = LedPixels2.mValueRed;
    ValueBlue = LedPixels2.mValueBlue;
    ValueGreen = LedPixels2.mValueGreen;
    ValueIntensity = LedPixels2.mValueIntensity;
    ValueRotationSpeed = LedPixels2.mValueRotationSpeed;
    ValueDimSpeed = LedPixels2.mValueDimSpeed;
    ValueMode = LedPixels2.mValueMode;
  }
  if (Channel == 3)
  {
    ValueRed = LedPixels3.mValueRed;
    ValueBlue = LedPixels3.mValueBlue;
    ValueGreen = LedPixels3.mValueGreen;
    ValueIntensity = LedPixels3.mValueIntensity;
    ValueRotationSpeed = LedPixels3.mValueRotationSpeed;
    ValueDimSpeed = LedPixels3.mValueDimSpeed;
    ValueMode = LedPixels3.mValueMode;
  }
  
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<head>";
  s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    s += "<script>";
      s += "function SendChange(Name, Value) {";
        s += "var xhr = new XMLHttpRequest();";
        if (Channel == 2)
          s += "xhr.open('GET', '/METER/' + Name + '=' + Value, true);";
        else if (Channel == 3)
          s += "xhr.open('GET', '/XMAS/' + Name + '=' + Value, true);";
        else
          s += "xhr.open('GET', '/CIRCLE/' + Name + '=' + Value, true);";
        s += "xhr.send();";
        s += "document.getElementById(Name+'Slider').value=Value;";
        s += "document.getElementById(Name+'Number').value=Value;";
      s += "}";
    s += "</script>";
  s += "</head><body>";
  if (Channel == 2)
    s += "<h3>Set Colors (Meter)</h3>";
  else if (Channel == 3)
    s += "<h3>Set Colors (XMas)</h3>";
  else
    s += "<h3>Set Colors (Circle)</h3>";
  s += "<table>" ;  
  s += "<tr><td>Red</td><td><input id=\"RedSlider\" type=\"range\" min=\"00\" max=\"255\" step=\"1\" VALUE=\""+String(ValueRed)+"\" oninput=\"SendChange('Red', this.value)\"/></td><td><INPUT id=\"RedNumber\" TYPE=\"NUMBER\" MIN=\"0\" MAX=\"255\" STEP=\"1\" VALUE=\""+String(ValueRed)+"\" style=\"width: 50px;\" oninput=\"SendChange('Red', this.value)\"></td></tr>" ;
  s += "<tr><td>Green</td><td><input id=\"GreenSlider\" type=\"range\" min=\"00\" max=\"255\" step=\"1\" VALUE=\""+String(ValueGreen)+"\" oninput=\"SendChange('Green', this.value)\"/></td><td><INPUT id=\"GreenNumber\" TYPE=\"NUMBER\" MIN=\"0\" MAX=\"255\" STEP=\"1\" VALUE=\""+String(ValueGreen)+"\" style=\"width: 50px;\" oninput=\"SendChange('Green', this.value)\"></td></tr>" ;
  s += "<tr><td>Blue</td><td><input id=\"BlueSlider\" type=\"range\" min=\"00\" max=\"255\" step=\"1\" VALUE=\""+String(ValueBlue)+"\" oninput=\"SendChange('Blue', this.value)\"/></td><td><INPUT id=\"BlueNumber\" TYPE=\"NUMBER\" MIN=\"0\" MAX=\"255\" STEP=\"1\" VALUE=\""+String(ValueBlue)+"\" style=\"width: 50px;\" oninput=\"SendChange('Blue', this.value)\"></td></tr>" ;
  s += "<tr><td>Intensity</td><td><input id=\"IntensitySlider\" type=\"range\" min=\"00\" max=\"255\" step=\"1\" VALUE=\""+String(ValueIntensity)+"\" oninput=\"SendChange('Intensity', this.value)\"/></td><td><INPUT id=\"IntensityNumber\" TYPE=\"NUMBER\" MIN=\"0\" MAX=\"255\" STEP=\"1\" VALUE=\""+String(ValueIntensity)+"\" style=\"width: 50px;\" oninput=\"SendChange('Intensity', this.value)\"></td></tr>" ;
  s += "<tr><td>Rotation Speed</td><td><input id=\"RotationSpeedSlider\" type=\"range\" min=\"1\" max=\"255\" step=\"1\" VALUE=\""+String(ValueRotationSpeed)+"\" oninput=\"SendChange('RotationSpeed', this.value)\"/></td><td><INPUT id=\"RotationSpeedNumber\" TYPE=\"NUMBER\" MIN=\"0\" MAX=\"255\" STEP=\"1\" VALUE=\""+String(ValueRotationSpeed)+"\" style=\"width: 50px;\" oninput=\"SendChange('RotationSpeed', this.value)\"></td></tr>" ;
  s += "<tr><td>Dim Speed</td><td><input id=\"DimSpeedSlider\" type=\"range\" min=\"1\" max=\"255\" step=\"1\" VALUE=\""+String(ValueDimSpeed)+"\" oninput=\"SendChange('DimSpeed', this.value)\"/></td><td><INPUT id=\"DimSpeedNumber\" TYPE=\"NUMBER\" MIN=\"0\" MAX=\"255\" STEP=\"1\" VALUE=\""+String(ValueDimSpeed)+"\" style=\"width: 50px;\" oninput=\"SendChange('DimSpeed', this.value)\"></td></tr>" ;
  s += "</table>" ;
  s += "<hr>" ;
  s += "<H4><input type=\"radio\" name=\"Mode\" value=\"1\" "+String(ValueMode==1?"checked":"")+" onchange=\"SendChange('Mode', '1')\">Single Color<br></H4>";
  s += "<H4><input type=\"radio\" name=\"Mode\" value=\"2\" "+String(ValueMode==2?"checked":"")+" onchange=\"SendChange('Mode', '2')\">Circle Color<br></H4>";
  s += "<H4><input type=\"radio\" name=\"Mode\" value=\"3\" "+String(ValueMode==3?"checked":"")+" onchange=\"SendChange('Mode', '3')\">Hue Wheel<br></H4>";
  s += "<hr>";
  s += "<C><H4>-=[ <a href=\"/Circle/\">Circle</a> ]=-=[ ";
  s += "<a href=\"/Meter/\">1 Meter</a> ]=-=[";
  s += "<a href=\"/Xmas/\">X-Mas</a> ]=-</H4></C>";
  s += "</body>";
  s += "</html>\n";
  ConnectedClient.print(s);
  Serial.println(s);
  Serial.println("Sending full page.");
}

