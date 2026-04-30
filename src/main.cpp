/*
Autor: Nicolas Marcelino Lopes.
Descrição: MQTT fundamentos.
Data: 24/04/2026.
Versão: 1.0.
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

const int PINO_LED_RGB = 48;
const int QUANTIDADE_LEDS = 1;

Adafruit_NeoPixel ledRGB(
    QUANTIDADE_LEDS,
    PINO_LED_RGB,
    NEO_GRB + NEO_KHZ800 // TODO: Não explicou
);

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void alterarCorLedRGB(int vermelho, int verde, int azul);
void tratarJsonLedRGB(const String &mensagem);

void setup()
{
  configurarDebug();

  configurarLedRGB(); // TODO: Explicar na próxima aula

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

void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
}

void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80); //Colocamos a qtd de brilho para o Led de 0 a 255
  ledRGB.clear();
  ledRGB.show(); //TODO: EXPLICAR NA PRÓXIMA 
}

void alterarCorLedRGB(int vermelho, int verde, int azul)
{
}

void tratarJsonLedRGB(const String &mensagem)
{
}