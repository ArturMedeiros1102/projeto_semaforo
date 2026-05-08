#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

// ─── Configurações ────────────────────────────────────────────────
const int PINO_LED_RGB    = 48;
const int QUANTIDADE_LEDS = 1;
const int PINO_LAMPADA    = 15;
const char TOPICO_COMANDO[] = "senai/Nicolas/esp32/comando";

// ─── Periféricos ──────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_NeoPixel ledRGB(QUANTIDADE_LEDS, PINO_LED_RGB, NEO_GRB + NEO_KHZ800);

// ─── Estado global ────────────────────────────────────────────────
unsigned long agora         = 0;
unsigned long tempoAnterior = 0;
unsigned long tempoNoturno  = 0;  // controla o piscar noturno

bool fluxoAlto    = false;
bool modoNoturno  = false;
bool ledNoturnoOn = false;        // estado atual do piscar

int faseSemaforo = 0;

// Cores configuráveis via MQTT
int corVerde[3]    = {0, 255, 0};
int corAmarelo[3]  = {255, 255, 0};
int corVermelho[3] = {255, 0, 0};

bool estadoLampada  = false;
bool estadoAnterior = false;

// ─── Protótipos ───────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────
void setup()
{
  configurarDebug();
  configurarLedRGB();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Iniciado");
  delay(2000);
  lcd.clear();

  conectarWiFi();
  configurarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();

  pinMode(PINO_LAMPADA, OUTPUT);

  alterarCorLedRGB(corVerde[0], corVerde[1], corVerde[2]);
  atualizarStatusLcd();
}

// ─────────────────────────────────────────────────────────────────
void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();

  agora = millis();

  // Detecta mudança no estado da lâmpada (fluxo alto)
  if (estadoLampada != estadoAnterior)
  {
    fluxoAlto      = estadoLampada;
    estadoAnterior = estadoLampada;
  }

  if (modoNoturno)
    atualizarNoturno();   // piscar amarelo
  else
    atualizarSemaforo();  // ciclo normal / fluxo alto
}

// ─────────────────────────────────────────────────────────────────
void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
  debugInfo("==============================");
  debugInfo("Mensagem recebida na aplicação");
  debugInfo("==============================");

  if (topico == nullptr)
  {
    debugErro("Tópico MQTT inválido");
    return;
  }

  debugInfo("Tópico: " + String(topico));
  debugInfo("Mensagem: " + mensagem);

  if (strcmp(topico, TOPICO_COMANDO) == 0)
  {
    tratarJsonComando(mensagem);
    return;
  }

  debugErro("Tópico não tratado: " + String(topico));
}

// ─────────────────────────────────────────────────────────────────
void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80);
  ledRGB.clear();
  ledRGB.show();
  debugInfo("Led RGB configurado no GPIO " + String(PINO_LED_RGB));
}

// ─────────────────────────────────────────────────────────────────
void alterarCorLedRGB(int vermelho, int verde, int azul)
{
  vermelho = constrain(vermelho, 0, 255);
  verde    = constrain(verde,    0, 255);
  azul     = constrain(azul,     0, 255);

  ledRGB.setPixelColor(0, ledRGB.Color(vermelho, verde, azul));
  ledRGB.show();

  debugInfo("Cor aplicada — R:" + String(vermelho) +
            " G:" + String(verde) +
            " B:" + String(azul));
}

// ─────────────────────────────────────────────────────────────────
void atualizarStatusLcd()
{
  lcd.clear();
  lcd.setCursor(0, 0);

  if (modoNoturno)
  {
    lcd.print("Modo: NOTURNO");
    lcd.setCursor(0, 1);
    lcd.print("Amarelo piscando");
    return;
  }

  lcd.print("Fluxo: ");
  lcd.print(fluxoAlto ? "ALTO" : "NORMAL");

  unsigned long tVerde    = fluxoAlto ? 7 : 3;
  unsigned long tAmarelo  = 2;
  unsigned long tVermelho = 5;

  lcd.setCursor(0, 1);
  lcd.printf("Verde:    %lus", tVerde);
  lcd.setCursor(0, 2);
  lcd.printf("Amarelo:  %lus", tAmarelo);
  lcd.setCursor(0, 3);
  lcd.printf("Vermelho: %lus", tVermelho);
}

// ─────────────────────────────────────────────────────────────────
void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagem);
  if (erro)
  {
    debugErro("Erro ao deserializar JSON: " + String(erro.c_str()));
    return;
  }

  tratarLed(doc);
  tratarLampada(doc);
  tratarModoNoturno(doc);
}

// ─────────────────────────────────────────────────────────────────
// Semáforo normal / fluxo altoa
void atualizarSemaforo()
{
  unsigned long tempoVerde    = fluxoAlto ? 7000 : 3000;
  unsigned long tempoAmarelo  = 2000;
  unsigned long tempoVermelho = 5000;

  unsigned long tempoFase;
  if      (faseSemaforo == 0) tempoFase = tempoVerde;
  else if (faseSemaforo == 1) tempoFase = tempoAmarelo;
  else                        tempoFase = tempoVermelho;

  if (agora - tempoAnterior >= tempoFase)
  {
    tempoAnterior = agora;
    faseSemaforo  = (faseSemaforo + 1) % 3;

    if      (faseSemaforo == 0) alterarCorLedRGB(corVerde[0],    corVerde[1],    corVerde[2]);
    else if (faseSemaforo == 1) alterarCorLedRGB(corAmarelo[0],  corAmarelo[1],  corAmarelo[2]);
    else                        alterarCorLedRGB(corVermelho[0], corVermelho[1], corVermelho[2]);
  }
}

// ─────────────────────────────────────────────────────────────────
// Piscar amarelo no modo noturno (500ms on / 500ms off)
void atualizarNoturno()
{
  const unsigned long INTERVALO_PISCAR = 500;

  if (agora - tempoNoturno >= INTERVALO_PISCAR)
  {
    tempoNoturno  = agora;
    ledNoturnoOn  = !ledNoturnoOn;

    if (ledNoturnoOn)
      alterarCorLedRGB(corAmarelo[0], corAmarelo[1], corAmarelo[2]);
    else
      alterarCorLedRGB(0, 0, 0); // LED apagado
  }
}

// ─────────────────────────────────────────────────────────────────
void tratarLed(JsonDocument &doc)
{
  // ── Verde ──
  if (doc["led_verde"].is<JsonObject>() &&
      doc["led_verde"]["r"].is<int>() &&
      doc["led_verde"]["g"].is<int>() &&
      doc["led_verde"]["b"].is<int>())
  {
    corVerde[0] = doc["led_verde"]["r"].as<int>();
    corVerde[1] = doc["led_verde"]["g"].as<int>();
    corVerde[2] = doc["led_verde"]["b"].as<int>();
  }
  else debugErro("led_verde inválido ou ausente");

  // ── Amarelo ──
  if (doc["led_amarelo"].is<JsonObject>() &&
      doc["led_amarelo"]["r"].is<int>() &&
      doc["led_amarelo"]["g"].is<int>() &&
      doc["led_amarelo"]["b"].is<int>())
  {
    corAmarelo[0] = doc["led_amarelo"]["r"].as<int>();
    corAmarelo[1] = doc["led_amarelo"]["g"].as<int>();
    corAmarelo[2] = doc["led_amarelo"]["b"].as<int>();
  }
  else debugErro("led_amarelo inválido ou ausente");

  // ── Vermelho ──
  if (doc["led_vermelho"].is<JsonObject>() &&
      doc["led_vermelho"]["r"].is<int>() &&
      doc["led_vermelho"]["g"].is<int>() &&
      doc["led_vermelho"]["b"].is<int>())
  {
    corVermelho[0] = doc["led_vermelho"]["r"].as<int>();
    corVermelho[1] = doc["led_vermelho"]["g"].as<int>();
    corVermelho[2] = doc["led_vermelho"]["b"].as<int>();
  }
  else debugErro("led_vermelho inválido ou ausente");
}

// ─────────────────────────────────────────────────────────────────
void tratarLampada(JsonDocument &doc)
{
  if (doc["lampada"].is<bool>())
  {
    estadoLampada = doc["lampada"].as<bool>();
    digitalWrite(PINO_LAMPADA, estadoLampada);
    debugInfo("Lâmpada: " + String(estadoLampada ? "ligada" : "desligada"));
  }
}

// ─────────────────────────────────────────────────────────────────
void tratarModoNoturno(JsonDocument &doc)
{
  if (!doc["modo_noturno"].is<bool>()) return;

  bool novoModo = doc["modo_noturno"].as<bool>();

  if (novoModo == modoNoturno) return; // sem mudança

  modoNoturno = novoModo;

  if (modoNoturno)
  {
    // Entra no modo noturno: pausa semáforo, inicia piscar
    tempoNoturno = millis();
    ledNoturnoOn = false;
    debugInfo("Modo NOTURNO ativado");
  }
  else
  {
    // Sai do noturno: retoma semáforo do início (fase verde)
    faseSemaforo  = 0;
    tempoAnterior = millis();
    alterarCorLedRGB(corVerde[0], corVerde[1], corVerde[2]);
    debugInfo("Modo NOTURNO desativado — retomando semáforo");
  }

  atualizarStatusLcd();
}
