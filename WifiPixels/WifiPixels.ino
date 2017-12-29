/*
 */

#include "LedPixels.h"
#include "FS.h"

CLedPixels LedPixels1;
CLedPixels LedPixels2;
CLedPixels LedPixels3;

void setup() 
{
  Serial.begin(74880);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("=============================================");
  Serial.println("Initialisation");
  Serial.print("Mounting FS...");

  if (!SPIFFS.begin()) 
    Serial.println("Failed !!!");
  else
    Serial.println("Mounted");
  
  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  WifiSetup();

  LedPixels1.Init(0, NEO_GRB, 12, 3);
  LedPixels2.Init(4, NEO_GRBW, 139, 4);
  LedPixels3.Init(5, NEO_GRB, 50, 3);

  Serial.println("Initialisation Finished");
  Serial.println("=============================================");
}

void loop() 
{
  WifiLoop();
  LedPixels1.LedPixelsLoop();
  LedPixels2.LedPixelsLoop();
  LedPixels3.LedPixelsLoop();
}


