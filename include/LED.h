#ifndef LED_h
#define LED_h

#include <Arduino.h>

/**
 * @class Led
 * @brief Classe paraa controle não bloqueante de um LED.
 * Permite ligar, deligar, ligar por tempo determinado,
 * Piscar continuamente e piscar por quantidade definida.
 *
 * @note O método update() deve ser chamado continuamente dentro do loop().
 */
class Led
{
private:
    uint8_t pino;
    bool estado; // atributo
    bool desligarPorTempo = false;
    uint32_t desligarNoMomento;

    uint32_t tempoAnteriorPiscar;
    uint32_t tempoEspera;
    bool estadoPiscar;
    uint16_t repeticoes;

    /**
     * @brief Processa a lógica de piscada do LED.
     */
    void funcaopiscar();

    /**
     * @brief Processa o desligamento temporizado  .
     */
    void funcaoDesligamento();

public:
    /**
     * @brief Constroi um objeto LED.
     * @param pin Número do pino digital onde o LED está conectado.
     */
    Led(uint8_t pin);

    /**
     * @brief Liga o LED continuamente.
     */
    void ligar();

    /**
     * @brief Liga o LED por um tempo determinado.
     * @param tempoligado_ms Tempo, em milissegundos, que o LED ficará ligado.
     */
    void ligar(unsigned long tempoligado_ms);

    /**
     * @brief Desliga o LED e cancela os modos automáticos.
     */
    void desligar();

    /**
     * @brief Inicia a piscada contínua em 1Hz
     */
    void piscar();

    /**
     * @brief Pisca o LED em uma determinada frequência.
     * @param Frequência da piscada em hertz.
     */
    void piscar(float frequencia);

    /**
     * @brief Pisca o LED uma quantidade definida de vezes.
     * @param frequencia Frequência da piscada em hertz.
     * @param repeticoes Quantidade de piscadas completas.
     */
    void piscar(float frequencia, uint16_t repeticoes);

    /**
     * @brief Retorna o estado do LED.
     */
    bool getEstado();

    /**
     * @brief Retorna o pino do LED.
     */
    uint8_t getPino();

    /**
     * @brief Define manualmente o estado do LED.
     * @param estado true para ligado, false para desligado.
     */
    void setEstado(bool estado);

    /**
     * @brief Alterna o estado do LED.
     */
    void alternar(); 

    /**
     * @brief Atualiza as saidas
     */
    void update(); 
};

#endif