# Jogo da Memória: Desafio Cerebral com Raspberry Pi Pico W

## 📝 Sobre o Projeto

Este projeto implementa um jogo de memória usando uma matriz de LEDs e um display OLED em um microcontrolador RP2040. O jogo gera sequências de números que o jogador deve memorizar e reproduzir corretamente. A interface inclui botões para entrada do jogador e LEDs RGB para feedback visual.

## 🎯 Funcionalidades

- O jogo inicia com um padrão na matriz de LEDs e uma mensagem no display.

- O jogador deve repetir a sequência de números exibidos no display.

- Cada rodada gera novos números sempre adicionando mais um número a sequência.

- Se o jogador errar, é mostrado no display sua pontuação e o jogo é reiniciado.

## 🔓 Como Usar

1. Instale o SDK do Raspberry Pi Pico caso ainda não tenha.

2. Compile o código usando um ambiente compatível (ex: CMake com o SDK do Pico).

3. Carregue o programa para a placa via UF2 ou usando um debugger compatível.

4. Conecte os periféricos de acordo com o esquema de ligação acima.

5. Inicie o jogo! O display mostrará o primeiro número a ser memorizado, e a matriz de LEDs servirá como seleção da sequencia.

## 📓 Estrutura do Código

- Geração da Sequência: O jogo inicia gerando uma sequência aleatória de números, com a complexidade aumentando progressivamente. A função generate_sequence() é responsável por essa etapa.

- Exibição da Sequência: A sequência é exibida visualmente no display OLED através da função show_sequence().

- Entrada do Usuário: O usuário deve repetir a sequência pressionando os botões correspondentes (A, B, Joystick), e mostrando o valor escolhido visualmente por meio da matriz de LEDs 5x5. As interrupções GPIO capturam as entradas do usuário.

- Verificação da Resposta: O sistema verifica se a sequência de botões pressionada pelo usuário corresponde à sequência original gerada. A função gpio_irq_handler() lida com a lógica de verificação e o funcionamento dos botões.

- Feedback e Progressão: Se a sequência estiver correta, o jogo avança, aumentando a dificuldade (comprimento da sequência). Se estiver incorreta, o jogo termina, exibindo a pontuação final e reiniciando o jogo. As funções correct_answer() e wrong_answer() fornecem feedback visual através do LED RGB.

## 🛠️ Requisitos

- Raspberry Pi Pico W
- LEDs RGB (Vermelho e Verde)
- Matriz de LEDs (5x5)
- Display SSD1306 I2C
- Três botões conectados aos pinos GPIO (22, 4 e 5)

## ⚙️ Compilação e Execução

1. Clone o repositório do projeto:
   ```sh
   git clone https://github.com/thalissoncastrog/embedded-system-memory-game.git
   cd embedded-system-memory-game
   ```
2. Crie um diretório de build e entre nele:
   ```sh
   mkdir build
   cd build
   ```
3. Execute o comando CMake para configurar a compilação:
   ```sh
   cmake ..
   ```
4. Compile o projeto:
   ```sh
   make
   ```
5. Faça o upload do binário gerado para a Raspberry Pi Pico.

## 👥 Colaboradores

1. **Adão Thalisson Castro Guimarães** - [GitHub](https://github.com/thalissoncastrog)

## 📜 Licença

Este projeto está licenciado sob a Licença MIT. Para mais detalhes, consulte o arquivo LICENSE.
