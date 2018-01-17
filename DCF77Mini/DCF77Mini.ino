
#include <ESP8266WebServer.h>

int ledPin = 2;   // LED connected to digital pin 13
int inPin = 16;     // pushbutton connected to digital pin 7
int val = 0;       // variable to store the read value
int LastSwitch = 0;
int LastValue = HIGH;
int CurLed = LOW;
int LastPrint = 0;

#define PULSSTATS_NR_BINS   50
#define PULSSTATS_BINSIZE   20
int EdgeStatsStart[PULSSTATS_NR_BINS]; //50ms bins
int EdgeStatsLen[PULSSTATS_NR_BINS]; //50ms bins

void setup() 
{
  delay(1000); //Wait until esp is ready. Without some libraries don't work well. (eg. dhcp in wifi ap)
  Serial.begin(74880); //equals as default esp
 // put your setup code here, to run once:
  WiFi.mode(WIFI_OFF);

  pinMode(ledPin, OUTPUT);      // sets the digital pin 13 as output
  pinMode(inPin, INPUT);        // sets the digital pin 7 as input

  for (int i=0; i<PULSSTATS_NR_BINS; i++)
  {
    EdgeStatsStart[i] = 0;
    EdgeStatsLen[i] = 0;
  }
  //attachInterrupt(digitalPinToInterrupt(inPin), DCFLevelChange, CHANGE);
}

void loop() 
{
  int NewValue = digitalRead(inPin);     // read the input pin
  if (NewValue != LastValue)
  {
    int CurTime = millis();
    if (NewValue == HIGH)
    {
      //EdgeStats[((CurTime-100)%1000)/PULSSTATS_BINSIZE]++;
      //EdgeStats[((CurTime-200)%1000)/PULSSTATS_BINSIZE]++;      
      EdgeStatsLen[(CurTime%1000)/PULSSTATS_BINSIZE]++;
    }
    else
    {
      EdgeStatsStart[(CurTime%1000)/PULSSTATS_BINSIZE]++;
    }
    if (CurTime - LastPrint > 1000)
    {
      LastPrint = CurTime;
      for (int i=0; i<PULSSTATS_NR_BINS; i++)
        Serial.printf("%04d ", EdgeStatsStart[i]);
      Serial.println("");
      for (int i=0; i<PULSSTATS_NR_BINS; i++)
        Serial.printf("%04d ", EdgeStatsLen[i]);
      Serial.println("");
      Serial.println("");
    }
#if(0)
    if (NewValue == HIGH)
    {
      int Duration = CurTime - LastSwitch;
      if (Duration > 75)
      {
        Serial.printf("Signal Width : %03d %04d %d\r\n", LastSwitch/1000, LastSwitch%1000, Duration);
        if (CurLed == LOW)
          CurLed = HIGH;
        else
          CurLed = LOW;
        digitalWrite(ledPin, CurLed);    // sets the LED to the button's value    
      }
    }
#endif
    LastSwitch = CurTime;
    LastValue = NewValue;
  }
}

void DCFLevelChange()
{
  Serial.printf("Interupt\r\n");
  
}

