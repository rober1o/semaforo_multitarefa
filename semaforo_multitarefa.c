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

void vSemaforo_normal() // TAREFA PARA EXIBIR O SEMAFORO NO MODO NORMAL
{
    configurar_matriz_leds(); // configurações iniciais para usar o PIO e a matriz de leds
    
    while (true)
    {
        if (modo_noturno) // SE ESTIVER NO MODO NOTUNO DESLIGA A MATRIZ DE LEDS E OS ESTADOS DOS SEMAFOROS
        {
            estado_vermelho = false;
            estado_amarelo = false;
            estado_verde = false;
            desenha_fig(matriz_apagada, BRILHO_PADRAO, pio, sm);
            vTaskDelay(pdMS_TO_TICKS(10)); // ESPERA CURTA PARA EVITAR LOOP
            continue; // VOLTA PARA O INICIO DO LOOP
        }

        // RODA O CÓDIGO ABAIXO SE NÃO TIVER NO MODO NOTURNO

        estado_verde = true; // DEFINE O ESTADO VERDE DO SEMAFORO POR 1 SEGUNDO
        desenha_fig(semaforo_verde, BRILHO_PADRAO, pio, sm);

        for (int i = 0; i < 10; i++) { // 10 × 100ms = 1000ms
            vTaskDelay(pdMS_TO_TICKS(100));
            
            if (modo_noturno) break; // SE EM ALGUM MOMENTO DESSES 1 SEGUNDO O MODO NOTURNO FOR ATIVADO, GARANTE QUE VOLTE PARA O INICIO DO LOOP E APAGA A MATRIZ IMEDIATAMENTE
        }
        
        if (modo_noturno) // VERIFICA SE O MODO NOTURNO FOI ATIVADO, CASO ESTEJA VOLTA PARA O INICIO DO LOOP
            continue;

        estado_verde = false; 
        estado_amarelo = true; // DEFINE O ESTADO AMARELO DO SEMAFORO POR 0,5 SEGUNDOS
        desenha_fig(semaforo_amarelo, BRILHO_PADRAO, pio, sm);
        vTaskDelay(pdMS_TO_TICKS(400));

        if (modo_noturno) // VERIFICA SE O MODO NOTURNO FOI ATIVADO, CASO ESTEJA VOLTA PARA O INICIO DO LOOP
            continue;

        estado_amarelo = false;

        estado_vermelho = true; // DEFINE O ESTADO VERMELHO DO SEMÁFORO POR ATÉ 2 SEGUNDOS
        desenha_fig(semaforo_vermelho, BRILHO_PADRAO, pio, sm);
        
        for (int i = 0; i < 20; i++) { // 20 × 100ms = 2000ms
            vTaskDelay(pdMS_TO_TICKS(100));
            
            if (modo_noturno) break; // SE EM ALGUM MOMENTO DESSES 2 SEGUNDOS O MODO NOTURNO FOR ATIVADO, GARANTE QUE VOLTE PARA O INICIO DO LOOP E APAGA A MATRIZ
        }
        
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
        if (!modo_noturno) // CASO O MODO NOTURNO SEJA DESLIGADO, DESLIGA TAMBÉM O LED RGB E RETORNA PARA O INICIO DO LOOP
        {
            gpio_put(LED_VERDE, false);
            gpio_put(LED_VERMELHO, false);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        gpio_put(LED_VERDE, true); // DEIXA O LED ALTERNADO ENTRE ACESO E APAGADO NO PERIODO DE 2 SEGUNDOS
        gpio_put(LED_VERMELHO, true);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(LED_VERDE, false);
        gpio_put(LED_VERMELHO, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vAtualizarDisplay() // TAREFA PARA ATUALIZAR INFORMAÇÕES NO DISPLAY
{
    inicializar_display_i2c(); // CONFIGURAÇÕES INICIAIS PARA USAR O DISPLAY
    char str_y[5];
    int contador = 0;
    bool cor = true;

    while (true)
    {
        if (modo_noturno) // VERIFICA SE O MODO NOTURNO ESTÁ ATIVADO
        {
            ssd1306_fill(&ssd, !cor);
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
            ssd1306_draw_string(&ssd, "CUIDADO", 20, 45); // EXIBE A MENSAGEM CUIDADO 
            draw_ssd1306(lua); // DESENHA UMA LUA PARA INFORMAR QUE O MODO NOTURNO ESTÁ ATIVO
            ssd1306_send_data(&ssd);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        else // VERIFICA SE O MODO NORMAL ESTÁ ATIVADO
        {
            
            if (estado_vermelho) // SE O SEMAFORO ESTIVER NO ESTADO VERMELHO EXIBE VERMELHO NO DISPLAY
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
                ssd1306_draw_string(&ssd, "VERMELHO", 32, 30);
                ssd1306_draw_string(&ssd, "--pare--", 34, 45); 
                draw_ssd1306(sol); // DESENHA O SOL PARA INDICAR QUE ESTÁ NO MODO NORMAL
                ssd1306_send_data(&ssd);
            }
            else if (estado_amarelo) // SE O SEMAFORO ESTIVER NO ESTADO AMARELO EXIBE AMARELO NO DISPLAY
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);

                ssd1306_draw_string(&ssd, "AMARELO", 34, 30);
                ssd1306_draw_string(&ssd, "--atencao--", 16, 45);
                draw_ssd1306(sol);// DESENHA O SOL PARA INDICAR QUE ESTÁ NO MODO NORMAL
                ssd1306_send_data(&ssd);
            }
            else if (estado_verde) // SE O SEMAFORO ESTIVER NO ESTADO VERDE EXIBE VERDE NO DISPLAY
            {
                sprintf(str_y, "%d", contador);
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
                ssd1306_draw_string(&ssd, "VERDE", 44, 30);
                ssd1306_draw_string(&ssd, "--atravesse--", 12, 45);
                draw_ssd1306(sol);// DESENHA O SOL PARA INDICAR QUE ESTÁ NO MODO NORMAL
                ssd1306_send_data(&ssd);
                

            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


void vTocarBuzzer(void *pvParameters) {

    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM); // Configura GPIO 10 como PWM
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Configuração de PWM: ~1.2 kHz com duty de 50%
    pwm_set_clkdiv(slice, 4.0f);
    pwm_set_wrap(slice, 41666);
    pwm_set_chan_level(slice, PWM_CHAN_A, 20833); // 50% duty

    while (true) {
        if (!modo_noturno) { // VERIFICA SE ESTÁ NO MODO NORMAL
            if (estado_vermelho) { // SE ESTIVER NO ESTADO VERMELHO, TOCA O BUZZER POR 0,5 SEGUNDOS E PARA POR 1,5
                pwm_set_enabled(slice, true);
                vTaskDelay(pdMS_TO_TICKS(500));
                pwm_set_enabled(slice, false);
                // DIVIDE O DELAY DE 1500MS EM 5x de 300ms VERIFICANDO SE EM ALGUM MOMENTO O BOTÃO FOI PRESSIONADO
                for (int i = 0; i < 5; i++) {
                    vTaskDelay(pdMS_TO_TICKS(300));
                    if (!estado_vermelho) break;
                }
            } 
            else if (estado_amarelo) { // SE ESTIVER NO ESTADO AMARELO, TOCA O BUZZER POR 50MS 3X INTERCALANDO DE 50 EM 50 MS
                for (int i = 0; i < 3; i++) {
                    if (!estado_amarelo) break;
                    pwm_set_enabled(slice, true);
                    vTaskDelay(pdMS_TO_TICKS(50)); 
                    pwm_set_enabled(slice, false);
                    vTaskDelay(pdMS_TO_TICKS(50)); 
                }
                // Total: 3 × (50+50) = 300ms → Adiciona 100ms final para completar 400ms
                vTaskDelay(pdMS_TO_TICKS(100)); 
            }
                    
            else if (estado_verde) {// SE ESTIVER NO ESTADO VERDE, TOCA O BUZZER POR 100MS E FICA PAUSADO POR 900MS
                pwm_set_enabled(slice, true);
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_set_enabled(slice, false);

                for (int i = 0; i < 9; i++) { // PAUSA DE 900MS VERIFICANDO A CADA 100MS SE O ESTADO VERDE ESTÁ ATIVO
                    vTaskDelay(pdMS_TO_TICKS(100));
                    if (!estado_verde) break;
                }
            } 
            else {
                pwm_set_enabled(slice, false);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        } else { // SE ESTIVER NO MODO NOTURNO TOCA UM BEEP DE 200MS A CADA 2S
            pwm_set_enabled(slice, true);
            vTaskDelay(pdMS_TO_TICKS(200));
            pwm_set_enabled(slice, false);
            for (int i = 0; i < 20; i++) { // PAUSA DE ATÉ 2S VERIFICANDO A CADA 100MS SE O MODO NOTURNO ESTÁ ATIVO
                vTaskDelay(pdMS_TO_TICKS(100));
                if (!modo_noturno) break;
            }
        }
    }
}



/*******************************************************

                      FUNÇÕES AUXILIARES

****************************************************** */

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



void draw_ssd1306(uint32_t *_matriz) { // FUNÇÃO PARA DESENHAR NO DISPLAY COM O CÓDIGO EXPORTADO DO PISKEL
    for (int i = 0; i < 8192; i++) {
        int x = i % 128;       // coluna
        int y = i / 128;       // linha

        if (_matriz[i] > 0x00000000) {
            ssd1306_pixel(&ssd, x, y, true);
        }
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
    stdio_init_all();


    // RODA AS 4 TAREFAS DO SISTEMA
    xTaskCreate(vSemaforo_noturno, "Semaforo noturno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vSemaforo_normal, "Semaforo normal", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vAcionarBotao, "Acionar botão", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vAtualizarDisplay, "Atualiza display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vTocarBuzzer, "Tocar buzzer", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();

    panic_unsupported();
}
