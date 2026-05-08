#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"
#include "LED.h"

const int PINO_LED_RGB = 48;
const int QUANTIDADE_LEDS = 1;
const char TOPICO_COMANDO[] = "senai/Nicolas/esp32/comando";

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_NeoPixel ledRGB(QUANTIDADE_LEDS, PINO_LED_RGB, NEO_GRB + NEO_KHZ800);
Led lampada(15);


unsigned long agora = 0;
unsigned long tempoAnterior = 0;
unsigned long tempoNoturno = 0;

bool fluxoAlto = false;
bool modoNoturno = false;
bool estadoLedNoturno = false;

int faseSemaforo = 0;

int corVerde[3] = {0, 255, 0};
int corAmarelo[3] = {255, 255, 0};
int corVermelho[3] = {255, 0, 0};

bool estadoLampada = false;

//Protótipos das funções
void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void alterarCorLedRGB(int vermelho, int verde, int azul);
void tratarJsonComando(const String &mensagem);
void atualizarStatusLcd();
void tratarLed(JsonDocument &doc);
void tratarLampada(JsonDocument &doc);
void tratarModoNoturno(JsonDocument &doc);
void atualizarSemaforo();
void atualizarNoturno();

void setup()
{
  configurarDebug();
  configurarLedRGB();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Sistema iniciado");
  delay(2000);
  lcd.clear();

  conectarWiFi();
  configurarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();

  alterarCorLedRGB(corVerde[0], corVerde[1], corVerde[2]);
  atualizarStatusLcd();
}

void loop()
{
  lampada.update();

  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();

  agora = millis();

  if(modoNoturno)
    atualizarNoturno();
  else
    atualizarSemaforo();
}

void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
  debugInfo("================================");
  debugInfo("Mensagem recebida na aplicação");
  debugInfo("================================");

  if(topico == nullptr)
  {
    debugErro("Tópico MQTT inválido");
    return;
  }

  debugInfo("Tópico: " + String(topico));
  debugInfo("Mensagem: " + mensagem);

  if(strcmp(topico, TOPICO_COMANDO) == 0)
  {
    tratarJsonComando(mensagem);
    return;
  }

  debugErro("Tópico não tratado: " + String(topico));
}

void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80);
  ledRGB.clear();
  ledRGB.show();
  debugInfo("Led RGB configura no GPIO " + String(PINO_LED_RGB));
}

void alterarCorLedRGB(int vermelho, int verde, int azul)
{
  vermelho = constrain(vermelho, 0, 255);
  verde = constrain(verde, 0, 255);
  azul = constrain(azul, 0, 255);

  ledRGB.setPixelColor(0, ledRGB.Color(vermelho, verde, azul));
  ledRGB.show();

  debugInfo("Cor aplicada — R:" + String(vermelho) +
            " G:" + String(verde) +
            " B:" + String(azul));
}

void atualizarStatusLcd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if(modoNoturno)
  {
    lcd.print("Modo: NOTURNO");
    lcd.setCursor(0, 1);
    lcd.print("Amarelo piscando");
    return;
  }

  lcd.print("Fluxo: ");
  lcd.print(fluxoAlto ? "ALTO" : "NORMAL");

  unsigned long tVerde = fluxoAlto ? 7 : 3;
  unsigned long tAmarelo = 2;
  unsigned long tVermelho = 5;

  lcd.setCursor(0, 1);
  lcd.print("Verde: " + String(tVerde) + "s");
  lcd.setCursor(0, 2);
  lcd.print("Verde: " + String(tAmarelo) + "s");
  lcd.setCursor(0, 3);
  lcd.print("Verde: " + String(tVermelho) + "s");
}

void tratarJsonComando(const String &mensagem)
{
 JsonDocument doc;
 DeserializationError erro = deserializeJson(doc, mensagem);

 if(erro)
 {
  debugErro("Erro ao deserializar JSON: " + String(erro.c_str()));
  return;
 }

 tratarLed(doc);
 tratarLampada(doc);
 tratarModoNoturno(doc);
}

void atualizarSemaforo()
{
  unsigned long tempoVerde = fluxoAlto ? 7000 : 3000;
  unsigned long tempoAmarelo = 2000;
  unsigned long tempoVermelho = 5000;

  unsigned int tempoFase;

  if(faseSemaforo == 0)
   tempoFase = tempoVerde;
  else if(faseSemaforo == 1)
   tempoFase = tempoAmarelo;
  else
   tempoFase = tempoVermelho;

  if(agora - tempoAnterior >= tempoFase)
  {
   faseSemaforo = (faseSemaforo + 1) % 3;
   tempoAnterior = agora;
  }

  if(faseSemaforo == 0)
   alterarCorLedRGB(corVerde[0], corVerde[1], corVerde[2]);
  else if(faseSemaforo == 1)
   alterarCorLedRGB(corAmarelo[0], corAmarelo[1], corAmarelo[2]);
  else
   alterarCorLedRGB(corVermelho[0], corVermelho[1], corVermelho[2]); 
}

void atualizarNoturno()
{
  const unsigned long INTERVALO_PISCAR = 500;

  if (agora - tempoAnterior >= INTERVALO_PISCAR)
  {
    tempoNoturno = agora;
    estadoLedNoturno = !estadoLedNoturno;

     if(estadoLedNoturno)
      alterarCorLedRGB(corAmarelo[0], corAmarelo[1], corAmarelo[2]);
     else
      alterarCorLedRGB(0, 0, 0);
  }
}

void tratarLed(JsonDocument &doc)
{
 if(doc["led_verde"].is<JsonObject>() &&
      doc["led_verde"]["r"].is<int>() &&
      doc["led_verde"]["g"].is<int>() &&
      doc["led_verde"]["b"].is<int>())
   {
    corVerde[0] = doc["led_verde"]["r"].as<int>();
    corVerde[1] = doc["led_verde"]["g"].as<int>();
    corVerde[2] = doc["led_verde"]["b"].as<int>();
   }   
 else
 {
  debugErro("led_verde inválido ou ausente. Verifique led_verde.r, led_verde.g,  led_verde.b,");
  return;
 }

  if(doc["led_amarelo"].is<JsonObject>() &&
      doc["led_amarelo"]["r"].is<int>() &&
      doc["led_amarelo"]["g"].is<int>() &&
      doc["led_amrelo"]["b"].is<int>())
    {
     corAmarelo[0] = doc["led_amarelo"]["r"].as<int>();
     corAmarelo[1] = doc["led_amarelo"]["g"].as<int>();
     corAmarelo[2] = doc["led_amarelo"]["b"].as<int>();
    }  
  else  
  {
   debugErro("led_amarelo inválido ou ausente. Verifique led_amarelo.r, led_amarelo.g,  led_amarelo.b,");
   return;
  }

  if(doc["led_vermelho"].is<JsonObject>() &&
      doc["led_vermelho"]["r"].is<int>() &&
      doc["led_vermelho"]["g"].is<int>() &&
      doc["led_vermelho"]["b"].is<int>())
    {
     corVermelho[0] = doc["led_vermelho"]["r"].as<int>();
     corVermelho[1] = doc["led_vermelho"]["g"].as<int>();
     corVermelho[2] = doc["led_vermelho"]["b"].as<int>();
    }  
  else
  {
  debugErro("led_vermelho inválido ou ausente. Verifique led_vermelho.r, led_vermelho.g,  led_vermelho.b,");
   return;
  }  
}

void tratarLampada(JsonDocument &doc)
{
  if(doc["lampada"].is<bool>())
  {
    estadoLampada = doc["lampada"].as<bool>();
    fluxoAlto = estadoLampada;
    estadoLampada ? lampada.ligar() : lampada.desligar();
    atualizarStatusLcd();
    debugInfo("Lampada " + String(estadoLampada ? "Ligada" : "Desligada"));
  }
}

void tratarModoNoturno(JsonDocument &doc)
{
  if(!doc["modo_noturno"].is<bool>())
   return;

  bool novoModo = doc["modo_noturno"].as<bool>();

  if(novoModo == modoNoturno)
   return;

  modoNoturno = novoModo;
  
  if(modoNoturno)
  {
    tempoNoturno = millis();
    estadoLedNoturno = false;
    debugInfo("Modo noturno ativado.");
  }
  else
  {
    faseSemaforo = 0;
    tempoAnterior = millis();
    alterarCorLedRGB(corVerde[0], corVerde[1], corVerde[2]);
    debugInfo("Modo NOTURNO desativado — retomando semáforo");
  }

  atualizarStatusLcd();
}





