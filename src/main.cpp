#include <LiquidCrystal_I2C.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

const int PINO_LED_RGB = 48;
const int QUANTIDADE_LEDS = 1;

const char TOPICO_COMANDO[] = "senai/Nicolas/esp32/comando";

const int PINO_LAMPADA = 15;

int tela = 1;

unsigned long agora;
bool fluxoAlto = false;
unsigned long tempoAnterior = 0;
int faseSemaforo = 0;

int corVerde[3] = {0, 255, 0};
int corAmarelo[3] = {255, 255, 0};
int corVermelho[3] = {255, 0, 0};

bool estadoLampada = false;

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_NeoPixel ledRGB(
    QUANTIDADE_LEDS,
    PINO_LED_RGB,
    NEO_GRB + NEO_KHZ800 // Constante de configuração.
);

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void alterarCorLedRGB(int vermelho, int verde, int azul);
void tratarJsonComando(const String &mensagem);
void atualizarStatusLampada(bool ligada);

void tratarLed(JsonDocument &doc);
void tratarLampada(JsonDocument &doc);
void tratarLcd(JsonDocument &doc);

void atualizarSemaforo();

void setup()
{
  configurarDebug();

  configurarLedRGB(); // TODO: Explicar na próxima aula

  // Inicializar LCD
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
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();
  atualizarSemaforo();

  agora = millis();

  bool estadoAnterior = false;

  if (estadoLampada != estadoAnterior)
  {
    fluxoAlto = estadoLampada;
    atualizarStatusLampada(estadoLampada);
    estadoAnterior = estadoLampada;
  }

  // alterarCorLedRGB(255, 0, 0);     // Exemplo: Configura o Led RGB para vermelho.
  // delay(1000);                     // Aguarda 1 segundo.
  // alterarCorLedRGB(0, 255, 0);     // Exemplo: Configura o Led RGB para verde.
  // delay(1000);                     // Aguarda 1 segundo.
  // alterarCorLedRGB(0, 0, 255);     // Exemplo: Configura o Led RGB para azul.
  // delay(1000);                     // Aguarda 1 segundo.
  // alterarCorLedRGB(255, 255, 255); // Exemplo: Configura o Led RGB para branco.
  // delay(1000);                     // Aguarda 1 segundo.
}

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

  if (strcmp(topico, TOPICO_COMANDO) == 0) // Verifica se o tópico da mensagem recebida é o mesmo que o tópico de comando definido.
  {
    tratarJsonComando(mensagem); // Chama a função tratarJsonComando, passando a mensagem recebida como argumento. Essa função é responsável por interpretar a mensagem JSON e executar as ações correspondentes com base no conteúdo da mensagem.
    return;
  }

  debugErro("Tópico não tratado: " + String(topico));
}

void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80); // Colocamos a qtd de brilho para o Led de 0 a 255
  ledRGB.clear();
  ledRGB.show(); // Atualiza o estado do Led, ou seja, mostra a cor que foi configurada.

  debugInfo("Led RGB configurado no GPIO " + String(PINO_LED_RGB));
}

void alterarCorLedRGB(int vermelho, int verde, int azul)
{
  vermelho = constrain(vermelho, 0, 255); // A função constrain é usada para limitar os valores de vermelho, verde e azul entre 0 e 255, garantindo que eles estejam dentro do intervalo válido para cores RGB.
  verde = constrain(verde, 0, 255);       // A função constrain é usada para limitar os valores de vermelho, verde e azul entre 0 e 255, garantindo que eles estejam dentro do intervalo válido para cores RGB.
  azul = constrain(azul, 0, 255);         // A função constrain é usada para limitar os valores de vermelho, verde e azul entre 0 e 255, garantindo que eles estejam dentro do intervalo válido para cores RGB.

  ledRGB.setPixelColor(0, ledRGB.Color(vermelho, verde, azul)); // Configura a cor do Led RGB. O primeiro parâmetro é o índice do Led (0 para o primeiro), e o segundo é a cor configurada usando a função Color.
  ledRGB.show();                                                // Atualiza o estado do Led para mostrar a nova cor.

  debugInfo("Cor aplicada no Led RGB");
  debugInfo("R: " + String(vermelho)); // Exibe o valor do componente vermelho no console de depuração.
  debugInfo("G: " + String(verde));    // Exibe o valor do componente verde no console de depuração.
  debugInfo("B: " + String(azul));     // Exibe o valor do componente azul no console de depuração.
}

void atualizarStatusLampada(bool ligada)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fluxo:");
  lcd.setCursor(8, 0);
  lcd.print(ligada ? "ALTO" : "NORMAL");
  if (ligada == true)
  {
    lcd.setCursor(0, 1);
    lcd.printf("Verde: %d s", 7);
    lcd.setCursor(0, 2);
    lcd.printf("Amarelo: %d s", 2);
    lcd.setCursor(0, 3);
    lcd.printf("Vermelho: %d s", 5);
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.printf("Verde: %d s", 3);
    lcd.setCursor(0, 2);
    lcd.printf("Amarelo: %d s", 2);
    lcd.setCursor(0, 3);
    lcd.printf("Vermelho: %d s", 5);
  }
}

void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagem); // A função deserializeJson é usada para analisar a string JSON contida na variável mensagem e armazenar os dados em um objeto JsonDocument chamado doc. Se ocorrer um erro durante a análise, ele será armazenado na variável erro.
  if (erro)
  {
    debugErro("Erro ao deserializar JSON: " + String(erro.c_str()));
    return;
  }

  tratarLed(doc);
  tratarLampada(doc);
  tratarLcd(doc);
}

void atualizarSemaforo()
{
  unsigned long tempoVerde = fluxoAlto ? 7000 : 3000;
  unsigned long tempoAmarelo = 2000;
  unsigned long tempoVermelho = 5000;

  unsigned long tempoFase;

  if (faseSemaforo == 0)
    tempoFase = tempoVerde;
  else if (faseSemaforo == 1)
    tempoFase = tempoAmarelo;
  else
    tempoFase = tempoVermelho;

  if (agora - tempoAnterior >= tempoFase)
  {
    tempoAnterior = agora;
    faseSemaforo = (faseSemaforo + 1) % 3;

    if (faseSemaforo == 0)
      alterarCorLedRGB(corVerde[0], corVerde[1], corVerde[2]);
    else if (faseSemaforo == 1)
      alterarCorLedRGB(corAmarelo[0], corAmarelo[1], corAmarelo[2]);
    else
      alterarCorLedRGB(corVermelho[0], corVermelho[1], corVermelho[2]);
  }
}

void tratarLed(JsonDocument &doc)
{
  if (!doc["led_verde"].is<JsonObject>())
  {
    debugErro("led_verde não é um objeto JSON");
    return;
  }

  if (!doc["led_verde"]["r"].is<int>() ||
      !doc["led_verde"]["g"].is<int>() ||
      !doc["led_verde"]["b"].is<int>()) // [CORRIGIDO] Verificação agora bate com o campo que está sendo lido
  {
    debugErro("JSON INVÁLIDO. Use led_verde.r, led_verde.g e led_verde.b para configurar a cor verde.");
    return;
  }

  else
  {
    corVerde[0] = doc["led_verde"]["r"].as<int>();
    corVerde[1] = doc["led_verde"]["g"].as<int>();
    corVerde[2] = doc["led_verde"]["b"].as<int>();
  }

  if (!doc["led_amarelo"].is<JsonObject>())
  {
    debugErro("led_amarelo não é um objeto JSON");
    return;
  }

  if (!doc["led_amarelo"]["r"].is<int>() ||
      !doc["led_amarelo"]["g"].is<int>() ||
      !doc["led_amarelo"]["b"].is<int>()) // [CORRIGIDO] Verificação agora bate com o campo que está sendo lido
  {
    debugErro("JSON INVÁLIDO. Use led_amarelo.r, led_amarelo.g e led_amarelo.b para configurar a cor amarelo.");
    return;
  }

  else
  {
    corAmarelo[0] = doc["led_amarelo"]["r"].as<int>();
    corAmarelo[1] = doc["led_amarelo"]["g"].as<int>();
    corAmarelo[2] = doc["led_amarelo"]["b"].as<int>();
  }

  if (!doc["led_vermelho"].is<JsonObject>())
  {
    debugErro("led_vermelho não é um objeto JSON");
    return;
  }

  if (!doc["led_vermelho"]["r"].is<int>() ||
      !doc["led_vermelho"]["g"].is<int>() ||
      !doc["led_vermelho"]["b"].is<int>()) // [CORRIGIDO] Verificação agora bate com o campo que está sendo lido
  {
    debugErro("JSON INVÁLIDO. Use led_vermelho.r, led_vermelho.g e led_vermelho.b para configurar a cor vermelho.");
    return;
  }

  else
  {
    corVermelho[0] = doc["led_vermelho"]["r"].as<int>();
    corVermelho[1] = doc["led_vermelho"]["g"].as<int>();
    corVermelho[2] = doc["led_vermelho"]["b"].as<int>();
  }
}
void tratarLampada(JsonDocument &doc)
{
  if (doc["lampada"].is<bool>()) // Verifica se o campo "lampada" existe e é um valor booleano. Se não for, essa parte do código será ignorada.
  {
    estadoLampada = doc["lampada"].as<bool>(); // Extrai o valor booleano do campo "lampada" do objeto JSON e o armazena na variável estadoLampada. O método as<bool>() é usado para converter o valor JSON para um tipo booleano.

    digitalWrite(PINO_LAMPADA, estadoLampada); // Configura o estado do pino da lâmpada com base no valor de estadoLampada. Se estadoLampada for true, a lâmpada será ligada (HIGH); se for false, a lâmpada será desligada (LOW).

    atualizarStatusLampada(estadoLampada); // Atualiza o display LCD com o status da lâmpada

    debugInfo("Lâmpada: " + String(estadoLampada ? "ligada" : "desligada")); // Exibe no console de depuração se a lâmpada foi ligada ou desligada com base no valor de estadoLampada.
  }
}

void tratarLcd(JsonDocument &doc)
{
}