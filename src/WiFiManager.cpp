//* WiFiManager.cpp
#include <Arduino.h>
#include "WiFiManager.h"
#include "secrets.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "DebugManager.h"

void conectarWiFi()
{
  debugInfo("===========================");
  debugInfo("Iniciando conexão WiFi...");
  debugInfo("===========================");

  // Configura o ESP32 como station, ou seja
  // ele vai se conectar a um roteador existente
  WiFi.mode(WIFI_STA);

  // Inicia a conexão com SSID e senha
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  debugInfo("Conectando");

  int tentativasWiFi = 0;
  const int maxTentativasWiFi = 30;

  // Aguarda a conexão por até 30 tentativas
  while (WiFi.status() != WL_CONNECTED && tentativasWiFi < maxTentativasWiFi)
  {
    delay(500);
    debugInfoSemLinha(".");
    tentativasWiFi++;
  }

  debugInfo("");

  if (WiFi.status() == WL_CONNECTED)
  {
    debugInfo("WiFi connectado com sucesso!");
    debugInfo("Endereço IP: ");
    debugInfo(String( WiFi.localIP() ) );
  }

  else
  {
    debugErro("Falha ao conectar no WiFi.");
    debugErro("Verifique SSID, senha e sinal de rede.");
  }
}

void garantirWiFiConectado()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    debugErro("WiFi desconectado. Tentando reconectar...");
    conectarWiFi();
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    debugErro("Não foi possível reconectar ao WiFi.");
  }
}

bool wifiEstaConectado()
{
  return WiFi.status() == WL_CONNECTED;
}