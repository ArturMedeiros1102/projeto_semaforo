#include <Arduino.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

void tratarMensagemRecebida(const char* topico, const String& mensagem);

void setup()
{
  configurarDebug();
  conectarWiFi();
  configurarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();
}

void tratarMensagemRecebida(const char* topico, const String& mensagem)
{

}