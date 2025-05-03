#include "semaforo_multitarefa.h"

void vAcionarBotao(void *pvParameters)
{
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    bool botao_anterior = true;
    while (true)
    {
        bool botao_atual = gpio_get(BOTAO_A);

        // Verifica transição: de solto (1) para pressionado (0)
        if (botao_anterior && !botao_atual)
        {
            modo_noturno = !modo_noturno; // Alterna o modo
        }

        botao_anterior = botao_atual;

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay para debounce e liberar CPU
    }
}

void vDelayComModoDiurno(TickType_t delay_ms)
{
    const TickType_t passo = 100; // 100ms
    TickType_t total = 0;

    while (total < delay_ms)
    {
        if (!modo_noturno)
            break;
        vTaskDelay(pdMS_TO_TICKS(passo));
        total += passo;
    }
}

void vSemaforo_noturno()
{
    gpio_init(LED_VERDE);
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    while (true)
    {
        if (!modo_noturno)
        {
            gpio_put(LED_VERDE, false);
            gpio_put(LED_VERMELHO, false);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        gpio_put(LED_VERDE, true);
        gpio_put(LED_VERMELHO, true);
        vDelayComModoDiurno(1000); // 1 segundo aceso

        gpio_put(LED_VERDE, false);
        gpio_put(LED_VERMELHO, false);
        vDelayComModoDiurno(1000); // 1 segundo apagado
    }
}

void vDelayComModoNoturno(int tempo_ms)
{
    for (int i = 0; i < tempo_ms; i += 100)
    {
        if (modo_noturno)
            break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vSemaforo_diurno()
{
    while (true)
    {
        if (modo_noturno) {
            estado_vermelho = false;
            estado_amarelo = false;
            estado_verde = false;
            desenha_fig(matriz_apagada, BRILHO_PADRAO, pio, sm);
            vTaskDelay(pdMS_TO_TICKS(100)); // Espera curta só para evitar loop apertado
            continue;
        }

        estado_vermelho = true;
        desenha_fig(semaforo_vermelho, BRILHO_PADRAO, pio, sm);
        vDelayComModoNoturno(7000);
        if (modo_noturno) continue;

        estado_vermelho = false;
        estado_amarelo = true;
        desenha_fig(semaforo_amarelo, BRILHO_PADRAO, pio, sm);
        vDelayComModoNoturno(1000);
        if (modo_noturno) continue;

        estado_amarelo = false;
        estado_verde = true;
        desenha_fig(semaforo_verde, BRILHO_PADRAO, pio, sm);
        vDelayComModoNoturno(4000);
        estado_verde = false;
    }
}

void vDisplay3Task()
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    char str_y[5];
    int contador = 0;
    bool cor = true;

    while (true)
    {
        if (modo_noturno)
        {
            // Exibe o modo noturno no display
            ssd1306_fill(&ssd, !cor);
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
            ssd1306_draw_string(&ssd, "MODO NOTURNO", 20, 16);
            ssd1306_send_data(&ssd);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        else
        {
            // Exibe o modo diurno com as cores do semáforo
            if (estado_vermelho)
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
                ssd1306_draw_string(&ssd, "MODO DIURNO", 20, 16);
                ssd1306_draw_string(&ssd, "VERMELHO", 32, 30);
                ssd1306_send_data(&ssd);
            }
            else if (estado_amarelo)
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
                ssd1306_draw_string(&ssd, "MODO DIURNO", 20, 16);
                ssd1306_draw_string(&ssd, "AMARELO", 34, 30);
                ssd1306_send_data(&ssd);
            }
            else if (estado_verde)
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
                ssd1306_draw_string(&ssd, "MODO DIURNO", 20, 16);
                ssd1306_draw_string(&ssd, "VERDE", 44, 30);
                ssd1306_send_data(&ssd);
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
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

    desenha_fig(semaforo_verde, BRILHO_PADRAO, pio, sm);

    stdio_init_all();

    // xTaskCreate(vBlinkLed1Task, "Blink Task Led1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vSemaforo_noturno, "Blink Task Led2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vSemaforo_diurno, "Semaforo diurno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vAcionarBotao, "Acionar botão", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vDisplay3Task, "Cont Task Disp3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();

    panic_unsupported();
}
