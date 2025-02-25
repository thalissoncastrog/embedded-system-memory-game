#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/interruptions_counter.pio.h"
#include "lib/ssd1306_i2c.h"

// Definições de constantes
#define NUM_PIXELS 25
#define LED_MATRIX_PIN 7

#define BUTTON_A_PIN 5 // Button A = 5
#define BUTTON_B_PIN 6 // Button B = 6

#define RED_LED_PIN 13 // LED RED = 13
#define GREEN_LED_PIN 11 // LED GREEN = 11
#define BLUE_LED_PIN 12 // LED BLUE = 12

#define BUTTON_JOYSTICK_PIN 22 // Button Joystick = 22

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SDL 15
#define ADDRESS 0x3C

#define IS_RGBW false

#define DEBOUNCE_US 200000  // 200ms debounce

#define MAX_NUMBERS 20

// Variáveis globais
static int confirmed_numbers[MAX_NUMBERS];
static int current_index = 0;

// Variáveis globais para controlar o estado dos LEDs e o temporizador
volatile bool led_on = false;
volatile alarm_id_t led_alarm_id = -1;

static volatile uint COUNTER = 0; //Contador para mostrar os valores na matriz de LEDs

uint8_t led_r = 0; // Intensidade de vermelho
uint8_t led_g = 0; // Intensidade de verde
uint8_t led_b = 20; // Intensidade de azul

// Variáveis para controlar a sequência de números
static int sequence[MAX_NUMBERS];
static int sequence_length = 1;
static int current_step = 0;
static int score = 0;

int upper_bound = 9;
int lower_bound = 0;

ssd1306_t display;

// Padrões de LEDs para exibir números
bool led_number_0[NUM_PIXELS] = {
    0, 1, 1, 1, 0,
    1, 1, 0, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 0, 1, 1,
    0, 1, 1, 1, 0,
};

bool led_number_1[NUM_PIXELS] = {
    0, 0, 1, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 1, 1, 0, 0, 
    0, 0, 1, 0, 0
};

bool led_number_2[NUM_PIXELS] = {
    1, 1, 1, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    0, 1, 0, 0, 1,
    0, 1, 1, 0, 0
};

bool led_number_3[NUM_PIXELS] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 0, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0
};

bool led_number_4[NUM_PIXELS] = {
    1, 0, 0, 0, 0,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1
};

bool led_number_5[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1
};

bool led_number_6[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1
};

bool led_number_7[NUM_PIXELS] = {
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
};

bool led_number_8[NUM_PIXELS] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0
};

bool led_number_9[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1
};

bool turn_off_all_leds[NUM_PIXELS] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

// Protótipos de funções
void setup();
void init_hardware();

void init_pwm(uint pin);
void set_pwm_duty_cycle(uint pin, uint16_t duty_cycle);

void set_one_led(uint8_t r, uint8_t g, uint8_t b);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
static inline void put_pixel(uint32_t pixel_grb);
bool* draw_number(int number);
int64_t clear_display_callback(alarm_id_t id, void *user_data);

void interruptions_callback_config();
void gpio_irq_handler(uint gpio, uint32_t events);

void correct_answer();
void wrong_answer();

void display_score();

int64_t reset_game();

void show_sequence();
void generate_sequence();


int main()
{
    setup();

    init_hardware();

    interruptions_callback_config();

    generate_sequence();
    show_sequence();

    while (true) {}
}

// Função de configuração dos pinos
void setup(){
    stdio_init_all();

    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(BUTTON_A_PIN);          // Habilita o pull-up interno

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(BUTTON_B_PIN);          // Habilita o pull-up interno

    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT); // Configura o pino como saída

    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT); // Configura o pino como saída

    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT); // Configura o pino como saída

    gpio_init(BUTTON_JOYSTICK_PIN);
    gpio_set_dir(BUTTON_JOYSTICK_PIN, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(BUTTON_JOYSTICK_PIN);          // Habilita o pull-up interno
}

// Função para inicializar o hardware PWM, I2C, display OLED e PIO
void init_hardware(){

    // Inicializa PWM para os LEDs RGB
    init_pwm(RED_LED_PIN);
    init_pwm(GREEN_LED_PIN);
    init_pwm(BLUE_LED_PIN);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SDL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SDL); // Pull up the clock line
    ssd1306_init(&display, WIDTH, HEIGHT, false, ADDRESS, I2C_PORT); // Inicializa o display
    ssd1306_config(&display); // Configura o display
    ssd1306_send_data(&display); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &interruptions_counter_program);

    interruptions_counter_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);
    set_one_led(led_r, led_g, led_b);
}

// Função para configurar as interrupções com callback dos botões A, B e Joystick
void interruptions_callback_config(){
    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_JOYSTICK_PIN, GPIO_IRQ_EDGE_FALL, true);
}

// Função para gerar uma sequência de números aleatóriosS
void generate_sequence(){
    srand(time(NULL));
    for (int i = 0; i < sequence_length; i++) {
        sequence[i] = rand() % (upper_bound - lower_bound + 1) + lower_bound;;
    }
}

// Função para configurar o duty cycle do PWM
void set_pwm_duty_cycle(uint pin, uint16_t duty_cycle){
    pwm_set_gpio_level(pin, duty_cycle);
}

// Função para inicializar o PWM
void init_pwm(uint pin){
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 255);
    pwm_set_gpio_level(pin, 0);
    pwm_set_enabled(slice_num, true);
}

// Função para reiniciar o jogo
int64_t reset_game() {
    sequence_length = 1;
    current_step = 0;
    score = 0;
    generate_sequence();
    show_sequence();

    return 0;
}

// Função para exibir a sequência de números no display OLED
void show_sequence() {
    printf("Sequence: ");
    for (int i = 0; i < sequence_length; i++) {
        printf("%d ", sequence[i]);
    }
    printf("\n");

    bool color = true;
    color = !color;

    ssd1306_fill(&display, !color); // Limpa o display
    ssd1306_rect(&display, 3, 3, 122, 58, color, !color); // Desenha um retângulo
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Sequence: ");
    ssd1306_draw_string(&display, buffer, 8, 5);

    for (int i = 0; i < sequence_length; i++) {
        snprintf(buffer, sizeof(buffer), "%d ", sequence[i]);

        // Calcula a posição x e y para exibir o número
        int x = (i % 11) * 10 + 8;
        int y = (i / 11) * 10 + 20;

        ssd1306_draw_string(&display, buffer, x, y); // Desenha uma string


    }

    ssd1306_send_data(&display); // Atualiza o display

    add_alarm_in_ms(2000, clear_display_callback, NULL, false);
}

// Função para desenhar um número na matriz de LEDs
bool* draw_number(int number)
{
    switch (number)
    {
    case 0:
        return led_number_0; // Retorna o padrão de LEDs para o número 0.
        break;
    case 1:
        return led_number_1; // Retorna o padrão de LEDs para o número 1.
        break;
    case 2:
        return led_number_2; // Retorna o padrão de LEDs para o número 2.
        break;
    case 3:
        return led_number_3; // Retorna o padrão de LEDs para o número 3.
        break;
    case 4:
        return led_number_4; // Retorna o padrão de LEDs para o número 4.
        break;
    case 5:
        return led_number_5; // Retorna o padrão de LEDs para o número 5.
        break;
    case 6:
        return led_number_6; // Retorna o padrão de LEDs para o número 6.
        break;
    case 7:
        return led_number_7; // Retorna o padrão de LEDs para o número 7.
        break;
    case 8:
        return led_number_8; // Retorna o padrão de LEDs para o número 8.
        break;
    case 9:
        return led_number_9; // Retorna o padrão de LEDs para o número 9.
        break;    
    default:
        return turn_off_all_leds; // Retorna um padrão que desliga todos os LEDs se o número for inválido.
        break;
    }
}

// Função para enviar um pixel para a máquina de estado do PIO
static inline void put_pixel(uint32_t pixel_grb)
{
    // Envia o valor do pixel para a máquina de estado do PIO, deslocando 8 bits para a esquerda.
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para criar um valor de 32 bits a partir de componentes RGB separados.
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    // Constrói um inteiro de 32 bits onde os componentes estão na ordem GRB.
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função para definir a cor de um LED na matriz de LEDs
void set_one_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    bool* led_number = draw_number(COUNTER);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_number[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}

// Função de callback do temporizador para apagar o display
int64_t clear_display_callback(alarm_id_t id, void *user_data){
    ssd1306_fill(&display, 0); // Limpa o display
    ssd1306_send_data(&display); // Atualiza o display
    return 0;
}

// Função de callback do temporizador para desligar os LEDs
int64_t led_off_callback(alarm_id_t id, void *user_data) {
    set_pwm_duty_cycle(RED_LED_PIN, 0);
    set_pwm_duty_cycle(GREEN_LED_PIN, 0);
    set_pwm_duty_cycle(BLUE_LED_PIN, 0);
    led_on = false;
    return 0;
}

// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events)
{
    static absolute_time_t last_interrupt_time = {0};
    absolute_time_t current_time = get_absolute_time();

    if (absolute_time_diff_us(last_interrupt_time, current_time) < DEBOUNCE_US) {
        return;  // Ignora interrupções muito próximas
    }

    if (gpio == BUTTON_B_PIN) {
        COUNTER = (COUNTER + 1) % 10;
    } else if (gpio == BUTTON_A_PIN) {
        COUNTER = (COUNTER - 1 + 10) % 10;
    }else if (gpio == BUTTON_JOYSTICK_PIN) {

        if(COUNTER == sequence[current_step]){
            current_step++;
            
            if(current_step == sequence_length){
                score++;
                sequence_length++;
                current_step = 0;
                generate_sequence();
                show_sequence();
            }

            correct_answer();

        }else{
            printf("Game Over!\nScore: %d\n", score);

            wrong_answer();

            display_score();
        }
    }
    
    set_one_led(led_r, led_g, led_b);
    last_interrupt_time = current_time;
}

// Função para exibir a resposta correta
void correct_answer(){
    set_pwm_duty_cycle(RED_LED_PIN, 0);
    set_pwm_duty_cycle(GREEN_LED_PIN, 20);
    set_pwm_duty_cycle(BLUE_LED_PIN, 0);

    if (led_alarm_id != -1) {
        cancel_alarm(led_alarm_id);
    }
    led_alarm_id = add_alarm_in_ms(500, led_off_callback, NULL, false);
}

// Função para exibir a resposta errada
void wrong_answer(){
    set_pwm_duty_cycle(RED_LED_PIN, 20);
    set_pwm_duty_cycle(GREEN_LED_PIN, 0);
    set_pwm_duty_cycle(BLUE_LED_PIN, 0);

    if (led_alarm_id != -1) {
        cancel_alarm(led_alarm_id);
    }
    led_alarm_id = add_alarm_in_ms(500, led_off_callback, NULL, false);
}

// Função para exibir a pontuação
void display_score() {
    char buffer[32];
    ssd1306_fill(&display, 0); // Limpa o display
    snprintf(buffer, sizeof(buffer), "Score: %d", score);
    ssd1306_draw_string(&display, buffer, 10, 20); // Exibe a pontuação
    ssd1306_send_data(&display); // Atualiza o display

    if (led_alarm_id != -1) {
        cancel_alarm(led_alarm_id);
    }

    led_alarm_id = add_alarm_in_ms(2000, reset_game, NULL, false);
}