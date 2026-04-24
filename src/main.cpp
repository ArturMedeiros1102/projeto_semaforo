
#include <Arduino.h>
#include "WiFiManager.h"






void setup() 
{
  Serial.begin(9600);
  conectarWiFi();
}

void loop() 
{
  garantirWiFiConectado();
}