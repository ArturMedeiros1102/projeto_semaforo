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

const char TOPICO_COMANDO[] = "senai/Nicolas/esp32/comando";

Adafruit_NeoPixel ledRGB(
    QUANTIDADE_LEDS,
    PINO_LED_RGB,
    NEO_GRB + NEO_KHZ800 // Constante de configuração.
);

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void alterarCorLedRGB(int vermelho, int verde, int azul);
void tratarJsonComando(const String &mensagem);

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

void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagem); // A função deserializeJson é usada para analisar a string JSON contida na variável mensagem e armazenar os dados em um objeto JsonDocument chamado doc. Se ocorrer um erro durante a análise, ele será armazenado na variável erro.
  if (erro)
  {
    debugErro("Erro ao deserializar JSON: " + String(erro.c_str()));
    return;
  }

  if (doc["led"].is<JsonObject>()) // Verifica se o campo "led" existe e é um objeto JSON. Se não for, exibe uma mensagem de erro e retorna da função.
  {
    if (!doc["led"]["r"].is<int>() ||
        !doc["led"]["g"].is<int>() ||
        !doc["led"]["b"].is<int>()) // Verifica se os campos "r", "g" e "b" existem dentro do objeto "led". Se algum desses campos estiver faltando, exibe uma mensagem de erro e retorna da função.
    {
      debugErro("JSON INVÁLIDO. Use led.r, led.g e led.b para configurar a cor do Led RGB.");
      return;
    }
    else
    {
      int vermelho = doc["led"]["r"].as<int>(); // Extrai o valor do componente vermelho do objeto JSON e o armazena na variável vermelho. O método as<int>() é usado para converter o valor JSON para um tipo inteiro.
      int verde = doc["led"]["g"].as<int>();    // Extrai o valor do componente verde do objeto JSON e o armazena na variável verde. O método as<int>() é usado para converter o valor JSON para um tipo inteiro.
      int azul = doc["led"]["b"].as<int>();     // Extrai o valor do componente azul do objeto JSON e o armazena na variável azul. O método as<int>() é usado para converter o valor JSON para um tipo inteiro.

      alterarCorLedRGB(vermelho, verde, azul); // Chama a função alterarCorLedRGB, passando os valores de vermelho, verde e azul extraídos do JSON para configurar a cor do Led RGB.
    }
  }
}