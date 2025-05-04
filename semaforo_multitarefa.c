#include "semaforo_multitarefa.h"

/*******************************************************

                      TAREFAS

****************************************************** */

void vAcionarBotao(void *pvParameters) // TAFERA PARA ALTERNA ENTRE MODO DIURNO  NPOTURNO
{
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    bool botao_anterior = true;
    while (true)
    {
        bool botao_atual = gpio_get(BOTAO_A);

        if (botao_anterior && !botao_atual)
        {
            modo_noturno = !modo_noturno; // Alterna o modo
        }

        botao_anterior = botao_atual;

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay para debounce e liberar CPU
    }
}

void vSemaforo_diurno() // TAREFA PARA EXIBIR O SEMAFORNO NO MODO DIURNO
{
    configurar_matriz_leds();
    
    while (true)
    {
        if (modo_noturno) // SE ESTIVER NO MODO NOTUNO DESLIGA A MATRIZ DE LEDS E OS ESTADOS DOS SEMAFOROS
        {
            estado_vermelho = false;
            estado_amarelo = false;
            estado_verde = false;
            desenha_fig(matriz_apagada, BRILHO_PADRAO, pio, sm);
            vTaskDelay(pdMS_TO_TICKS(10)); // Espera curta só para evitar loop apertado
            continue;
        }

        estado_verde = true; // DEFINE O ESTADO VERDE DO SEMAFORO POR 4 SEGUNDOS
        desenha_fig(semaforo_verde, BRILHO_PADRAO, pio, sm);
        vTaskDelay(pdMS_TO_TICKS(1000));
        // vDelayComModoDiurno(1000); // ACIONA O vTaskDelay DE FORMA FRACIONADA, CASO HAJA QUALQUER MUDANÇA ELE É INTERROMPIDO IMEDIATAMENTE

        if (modo_noturno)
            continue;

        estado_verde = false;
        estado_amarelo = true; // DEFINE O ESTADO AMARELO DO SEMAFORO POR 1 SEGUNDOS
        desenha_fig(semaforo_amarelo, BRILHO_PADRAO, pio, sm);
        vTaskDelay(pdMS_TO_TICKS(500));
        // vDelayComModoDiurno(500); // ACIONA O vTaskDelay DE FORMA FRACIONADA, CASO HAJA QUALQUER MUDANÇA ELE É INTERROMPIDO IMEDIATAMENTE
        if (modo_noturno)
            continue;

        estado_amarelo = false;

        estado_vermelho = true; // DEFINE O ESTADO VERMELHO DO SEMAFORO POR 7 SEGUNDOS
        desenha_fig(semaforo_vermelho, BRILHO_PADRAO, pio, sm);
        vTaskDelay(pdMS_TO_TICKS(2000));
        // vDelayComModoDiurno(2000); // ACIONA O vTaskDelay DE FORMA FRACIONADA, CASO HAJA QUALQUER MUDANÇA ELE É INTERROMPIDO IMEDIATAMENTE
        estado_vermelho = false;
    }
}

void vSemaforo_noturno() // TAREFA PARA EXIBIR O SEMAFORO NOTURNO
{
    gpio_init(LED_VERDE);
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    while (true)
    {
        if (!modo_noturno) // CASO O MODO NOTURNO SEJA DESLIGADO, DESLIGA TAMBÉM O LED RGB
        {
            gpio_put(LED_VERDE, false);
            gpio_put(LED_VERMELHO, false);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        desenha_fig(matriz_apagada, BRILHO_PADRAO, pio, sm);
        gpio_put(LED_VERDE, true); // DEIXA O LED ALTERNADO ENTRE ACESO E APAGADO NO PERIODO DE 2 SEGUNDOS
        gpio_put(LED_VERMELHO, true);
        vTaskDelay(pdMS_TO_TICKS(1000));
        // vDelayComModoNoturno(1000); // ACIONA O vTaskDelay DE FORMA FRACIONADA, CASO HAJA QUALQUER MUDANÇA ELE É INTERROMPIDO IMEDIATAMENTE

        gpio_put(LED_VERDE, false);
        gpio_put(LED_VERMELHO, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        // vDelayComModoNoturno(1000); // ACIONA O vTaskDelay DE FORMA FRACIONADA, CASO HAJA QUALQUER MUDANÇA ELE É INTERROMPIDO IMEDIATAMENTE
    }
}

void vAtualizarDisplay() // TAREFA PARA ATUALIZAR INFORMAÇÕES NO DISPLAY
{
    inicializar_display_i2c();
    char str_y[5];
    int contador = 0;
    bool cor = true;

    while (true)
    {
        if (modo_noturno) // VERIFICA SE O MODO NOTURNO ESTÁ ATIVADO, CASO ESTEJA INFORMA QUE ESTÁ NO MODO NOTURNO
        {
            // Exibe o modo noturno no display
            ssd1306_fill(&ssd, !cor);
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
            ssd1306_draw_string(&ssd, "MODO NOITE", 20, 16);
            ssd1306_send_data(&ssd);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        else
        {
            /// VERIFICA SE O MODO NORMAL ESTÁ ATIVADO, CASO ESTEJA INFORMA QUE ESTÁ NO MODO NORMAL
            if (estado_vermelho) // SE O SEMAFORO ESTIVER NO ESTADO VERMELHO EXIBE VERMELHO NO DISPLAY
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
                ssd1306_draw_string(&ssd, "VERMELHO", 32, 30);
                ssd1306_draw_string(&ssd, "--pare--", 34, 45);
                draw_ssd1306(bonequinho);
                ssd1306_send_data(&ssd);
            }
            else if (estado_amarelo) // SE O SEMAFORO ESTIVER NO ESTADO AMARELO EXIBE AMARELO NO DISPLAY
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);

                ssd1306_draw_string(&ssd, "AMARELO", 34, 30);
                ssd1306_draw_string(&ssd, "--atencao--", 16, 45);
                draw_ssd1306(bonequinho);
                ssd1306_send_data(&ssd);
            }
            else if (estado_verde) // SE O SEMAFORO ESTIVER NO ESTADO VERDE EXIBE VERDE NO DISPLAY
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
                ssd1306_draw_string(&ssd, "VERDE", 44, 30);
                ssd1306_draw_string(&ssd, "--atravesse--", 12, 45);
                draw_ssd1306(bonequinho);
                ssd1306_send_data(&ssd);
                

            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


void vTocarBuzzer(void *pvParameters) {
    gpio_set_function(10, GPIO_FUNC_PWM); // Configura GPIO 10 como PWM
    uint slice = pwm_gpio_to_slice_num(10);

    // Configuração de PWM: ~1.2 kHz com duty de 50%
    pwm_set_clkdiv(slice, 4.0f);
    pwm_set_wrap(slice, 41666);
    pwm_set_chan_level(slice, PWM_CHAN_A, 20833); // 50% duty

    while (true) {
        if (!modo_noturno) {
            if (estado_vermelho) {
                pwm_set_enabled(slice, true);
                vTaskDelay(pdMS_TO_TICKS(500));
                pwm_set_enabled(slice, false);
                // Divide o silêncio de 1500ms em 5x de 300ms verificando se em algum momento o botão foi pressionado
                for (int i = 0; i < 5; i++) {
                    vTaskDelay(pdMS_TO_TICKS(300));
                    if (!estado_vermelho) break;
                }
            } 
            else if (estado_amarelo) {
                for (int i = 0; i < 3; i++) {
                    if (!estado_amarelo) break;
                    pwm_set_enabled(slice, true);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    pwm_set_enabled(slice, false);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                vTaskDelay(pdMS_TO_TICKS(200)); // Completa 1s
            } 
            else if (estado_verde) {
                pwm_set_enabled(slice, true);
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_set_enabled(slice, false);

                for (int i = 0; i < 9; i++) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                    if (!estado_verde) break;
                }
            } 
            else {
                pwm_set_enabled(slice, false);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        } else {
            pwm_set_enabled(slice, true);
            vDelayComModoNoturno(200); // 200ms de som
            pwm_set_enabled(slice, false);
            vDelayComModoNoturno(2000); // 2s de silêncio
        }
    }
}



/*******************************************************

                      FUNÇÕES AUXILIARES

****************************************************** */

void vDelayComModoNoturno(int tempo_ms) // FUNÇÃO PARA VERIFICAR SE O MODO NOTURNO ESTÁ ATIVADO DURANTE O DELAY
{
    for (int i = 0; i < tempo_ms; i += 100) // DADO O INTERVALO ELE VERIFICA A CADA 100 MILISSEGUNDOS
    {
        if (!modo_noturno)
            break; // SE O MODO NOTURNO FOR ATIVADO, ELE INTERROMPE AUTOMATICAMENTE O DELAY PARA QUE O SISTEMA SEJA DESLIGADO
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vDelayComModoDiurno(int tempo_ms) // FUNÇÃO PARA VERIFICAR SE O MODO NORMAL ESTÁ ATIVADO DURANTE O DELAY
{
    for (int i = 0; i < tempo_ms; i += 100) // DADO O INTERVALO ELE VERIFICA A CADA 100 MILISSEGUNDOS
    {
        if (modo_noturno)
            break; // SE O MODO NORMAL FOR ATIVADO, ELE INTERROMPE AUTOMATICAMENTE O DELAY PARA QUE O SISTEMA SEJA DESLIGADO
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

void desenha_fig(uint32_t *_matriz, uint8_t _intensidade, PIO pio, uint sm) // FUNÇÃO PARA DESENHAR O SEMAFORO NA MATRIZ
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

void configurar_matriz_leds() // FUNÇÃO PARA CONFIGURAR O PIO PARA USAR NA MATRIZ DE LEDS
{
    bool clock_setado = set_sys_clock_khz(133000, false);
    pio = pio0;
    sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &Matriz_5x5_program);
    Matriz_5x5_program_init(pio, sm, offset, MATRIZ_PIN);
}

void inicializar_display_i2c(){ //FUNÇÃO PARA INICIALIZAR O I2C DO DISPLAY
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void draw_ssd1306(uint32_t *_matriz) {
    for (int i = 0; i < 8192; i++) {
        int x = i % 128;       // coluna
        int y = i / 128;       // linha

        if (_matriz[i] > 0x00000000) {
            ssd1306_pixel(&ssd, x, y, true);
        }
    }

    // Envia os dados desenhados para o display
    // ssd1306_send_data(&ssd);
}



int main()
{
    
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();

    xTaskCreate(vSemaforo_noturno, "Semaforo noturno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vSemaforo_diurno, "Semaforo diurno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vAcionarBotao, "Acionar botão", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vAtualizarDisplay, "Atualiza display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vTocarBuzzer, "Tocar buzzer", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();

    panic_unsupported();
}
