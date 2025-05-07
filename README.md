# SEMÁFORO MULTITAREFA

O projeto tem por finalidade criar um semáforo acessível que opere no modo normal e noturno, oferecendo sinais visuais e sonoros garantindo a segurança de todos, incluindo deficientes visuais. Além disso o projeto usa conceitos de multitarefas para gerenciar diferentes periféricos ao mesmo tempo.

## Componentes Utilizados


1. **Botão Pushbutton**
2. **Display OLED 1306**
3. **Buzzer**
4. **Matriz de LED 5x5 WS2812** 
5. **Led RGB**

## Funcionalidade

Ao inciar o programa o semaforo operará no modo normal da segunte forma:

Acende a luz verde  na matriz de leds por 4 s e toca um beep no buzzer por 1000ms, no display aparece a imagem de um sol e a mensagem Verde --atravesse--
Apaga o Verde e Acende a luz amarela  na matriz de leds por 1000 ms segundos e toca  beeps rápidos de 100ms intermitentes, no display aparece a imagem de um sol e a mensagem Amarelo --atenção--
Apaga o amarelo e Acende a luz vermelha na matriz de leds por 7 s segundos e toca um beep continuo por 500ms com pausa de 1500ms, no display aparece a imagem de um sol e a mensagem Vermelho --pare--

Caso o botão A seja pressionado, alterna para o modo noturno, que operará da segunte forma

Luz amarela do led RGB piscando em um periodo de 2s, beep lento de 200ms a cada 2s, no display aparece a imagem de uma lua e a mensagem CUIDADO

### Como Usar

#### Usando a BitDogLab

- Clone este repositório: git clone https://github.com/rober1o/semaforo_multitarefa.git;
- Usando a extensão Raspberry Pi Pico importar o projeto;
- Ajuste o diretório do FreeRTOS, conforme seu computador
- Compilar o projeto;
- Plugar a BitDogLab usando um cabo apropriado e gravar o código.

## Demonstração

<!-- TODO: adicionar link do vídeo -->
Vídeo demonstrando as funcionalidades da solução implementada: [Demonstração](https://youtu.be/nhr6UkEngDA)
