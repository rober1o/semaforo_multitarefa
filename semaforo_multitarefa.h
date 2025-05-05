#ifndef SEMAFORO_MULTITAREFA // Previne múltiplas inclusões do cabeçalho
#define SEMAFORO_MULTITAREFA

#include <stdio.h>
#include <stdint.h>  // Para tipos uint32_t e uint8_t

// Bibliotecas do Raspberry Pi Pico
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"    // Controle do PIO (Programável I/O)
#include "hardware/clocks.h" // Manipulação de clock


// Bibliotecas para displays e fontes
#include "lib/ssd1306.h"
#include "lib/font.h"

// Arquivos específicos do projeto
#include "pio_wave.pio.h"  // Código PIO
#include "lib/matriz_5X5.h"         // Arrays para exibir na matriz de LED
#include "lib/desenhos.h"


// Bibliotecas para FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"


// Definições de pinos e parâmetros
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define BUZZER_PIN 10  // Pino conectado ao buzzer

// Matriz de LEDs
#define MATRIZ_PIN 7          // Pino da matriz de LEDs
#define NUM_PIXELS 25         // Número de pixels na matriz
#define BRILHO_PADRAO 100     // Brilho padrão

// Definições dos LEDs
#define LED_VERDE 11
#define LED_VERMELHO 13
#define LED_AZUL 12
#define BOTAO_A 5



// Instâncias e variáveis globais
PIO pio;  // Instância do PIO
int sm;    // Máquina de estado do PIO
ssd1306_t ssd; // display ssd
bool modo_noturno = false;
bool estado_verde = true;
bool estado_amarelo = false;
bool estado_vermelho = false;


void vAcionarBotao(void *pvParameters);
void vSemaforo_noturno();
void vSemaforo_normal();
void vAtualizarDisplay();
void configurar_matriz_leds();
void inicializar_display_i2c();
void desenha_fig(uint32_t *_matriz, uint8_t _intensidade, PIO pio, uint sm);
void draw_ssd1306(uint32_t *_matriz);
#endif   // SEMAFORO_MULTITAREFA