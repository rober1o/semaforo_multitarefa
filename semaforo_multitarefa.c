
#include "semaforo_multitarefa.h"


void vBlinkLed1Task()
{
    gpio_init(led1);
    gpio_set_dir(led1, GPIO_OUT);
    while (true)
    {
        gpio_put(led1, true);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_put(led1, false);
        vTaskDelay(pdMS_TO_TICKS(1223));
    }
}

void vBlinkLed2Task()
{
    gpio_init(led2);
    gpio_set_dir(led2, GPIO_OUT);
    while (true)
    {
        gpio_put(led2, true);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(led2, false);
        vTaskDelay(pdMS_TO_TICKS(2224));
    }
}

void vDisplay3Task()
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    char str_y[5]; // Buffer para armazenar a string
    int contador = 0;
    bool cor = true;
    while (true)
    {
        sprintf(str_y, "%d", contador);                      // Converte em string
        contador++;                                          // Incrementa o contador
        ssd1306_fill(&ssd, !cor);                            // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);        // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);             // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);             // Desenha uma linha
        ssd1306_draw_string(&ssd, "SEMAROTO", 8, 6);         // Desenha uma string
        ssd1306_draw_string(&ssd, "RESIDECH", 20, 16);    // Desenha uma string
        ssd1306_draw_string(&ssd, "  FreeRTOS", 10, 28);     // Desenha uma string
        ssd1306_draw_string(&ssd, "Contador  LEDs", 10, 41); // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 40, 52);            // Desenha uma string
        ssd1306_send_data(&ssd);                             // Atualiza o display
        sleep_ms(735);
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

void desenha_fig(uint32_t *_matriz, uint8_t _intensidade, PIO pio, uint sm)
{
    uint32_t pixel = 0;
    uint8_t r, g, b;

    for (int i = 24; i > 19; i--) // Linha 1
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 15; i < 20; i++) // Linha 2
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (b << 16) | (r << 8) | g;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 14; i > 9; i--) // Linha 3
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 5; i < 10; i++) // Linha 4
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 4; i > -1; i--) // Linha 5
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }
}
int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    // Define o PIO 0 para controle da matriz de LEDs
    pio = pio0;

    // Configura o clock do sistema para 133 MHz
    bool clock_setado = set_sys_clock_khz(133000, false);

    // Inicializa a comunicação serial
    stdio_init_all();

    // Exibe mensagem na serial caso o clock tenha sido configurado com sucesso
    if (clock_setado)
        printf("Clock setado %ld\n", clock_get_hz(clk_sys));

    // Carrega o programa PIO para controle da matriz de LEDs
    int offset = pio_add_program(pio, &Matriz_5x5_program);

    // Obtém um state machine livre para o PIO
    sm = pio_claim_unused_sm(pio, true);

    // Inicializa o programa PIO na matriz de LEDs
    Matriz_5x5_program_init(pio, sm, offset, MATRIZ_PIN);

    desenha_fig(numero_7, BRILHO_PADRAO, pio, sm);

    stdio_init_all();

    xTaskCreate(vBlinkLed1Task, "Blink Task Led1", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBlinkLed2Task, "Blink Task Led2", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplay3Task, "Cont Task Disp3", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}
