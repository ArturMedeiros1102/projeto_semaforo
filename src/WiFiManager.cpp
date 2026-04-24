//! WiFiManager.cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "WiFiManager.h"
#include "secrets.h"

bool wifiEstaConectado ()
{
    return WiFi.status() == WL_CONNECTED;
}

void conectarWiFi()
{
  Serial.println("===========================");
  Serial.println("Iniciando conexão WiFi...");
  Serial.println("===========================");
  
  //Configura o ESP32 como station, ou seja
  //ele vai se conectar a um roteador existente
  WiFi.mode(WIFI_STA);         
  
  //Inicia a conexão com SSID e senha
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  Serial.println("Conectando");

  int tentativasWiFi = 0;
  const int maxTentativasWiFi = 30;

  //Aguarda a conexão por até 30 tentativas
  while(WiFi.status() != WL_CONNECTED && tentativasWiFi < maxTentativasWiFi)
  {
    delay(500);
    Serial.print(".");
    tentativasWiFi++;
  }

  Serial.println();

  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connectado com sucesso!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  }

  else
  {
    Serial.println("Falha ao conectar no WiFi.");
    Serial.println("Verifique SSID, senha e sinal de rede.");
  }
}


void garantirWiFiConectado()
{
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    conectarWiFi();
  }

  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Não foi possível reconectar ao WiFi.");
  }
}