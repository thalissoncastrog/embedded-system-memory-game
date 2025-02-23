#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "interruptions_counter.pio.h"

#define NUM_PIXELS 25
#define LED_MATRIX_PIN 7

#define BUTTON_A_PIN 5 // Button A = 5
#define BUTTON_B_PIN 6 // Button B = 6

#define RED_LED_PIN 13 // LED RED = 13
#define GREEN_LED_PIN 11 // LED GREEN = 11
#define BLUE_LED_PIN 12 // LED BLUE = 12

#define BUTTON_JOYSTICK_PIN 22 // Button Joystick = 22

#define IS_RGBW false

#define DEBOUNCE_US 200000  // 200ms debounce

#define MAX_NUMBERS 999

static int confirmed_numbers[MAX_NUMBERS];
static int current_index = 0;

static volatile uint COUNTER = 0;

uint8_t led_r = 0; // Red intensity
uint8_t led_g = 0; // Green intensity
uint8_t led_b = 20; // Blue intensity

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

bool* draw_number(int number);
void set_one_led(uint8_t r, uint8_t g, uint8_t b);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
static inline void put_pixel(uint32_t pixel_grb);
void gpio_irq_handler(uint gpio, uint32_t events);

static int sequence[MAX_NUMBERS];
static int sequence_length = 1;
static int current_step = 0;
static int score = 0;

int upper_bound = 9;
int lower_bound = 0;

void generate_sequence() {
    for (int i = 0; i < sequence_length; i++) {
        sequence[i] = rand() % (upper_bound - lower_bound + 1) + lower_bound;;
    }
}

void show_sequence() {
    printf("Sequence: ");
    for (int i = 0; i < sequence_length; i++) {
        printf("%d ", sequence[i]);
    }
    printf("\n");
}

void reset_game() {
    sequence_length = 1;
    current_step = 0;
    score = 0;
    generate_sequence();
    show_sequence();
}

int main()
{
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

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &interruptions_counter_program);

    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_JOYSTICK_PIN, GPIO_IRQ_EDGE_FALL, true);

    interruptions_counter_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);

    set_one_led(led_r, led_g, led_b);

    generate_sequence();
    show_sequence();

    while (true) {
        sleep_ms(1000);

        // if(sizeof(confirmed_numbers) > 0){
        //     for(int i = 0; i < current_index; i++){
        //         printf("Number %d: %d\n", i, confirmed_numbers[i]);
        //     }
        // }

        // for(int i = 0; i < 5; i++){
        //     int value = rand() % (upper_bound - lower_bound + 1) + lower_bound;
            
        //     if(i == 4){
        //         printf("%d\n", value);
        //     }else{
        //         printf("%d, ", value);
        //     }
        // }

        // printf("--------------------------------------------------------------\n");
        
    }
}

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
        // if(current_index < MAX_NUMBERS){
        //     confirmed_numbers[current_index] = COUNTER;
        //     current_index++;
        //     printf("Number %d confirmed\n", confirmed_numbers[current_index-1]);
        // }

        if(COUNTER == sequence[current_step]){
            current_step++;

            gpio_put(RED_LED_PIN, 0);
            gpio_put(GREEN_LED_PIN, 1);
            gpio_put(BLUE_LED_PIN, 0);
            
            if(current_step == sequence_length){
                score++;
                sequence_length++;
                current_step = 0;
                generate_sequence();
                show_sequence();
            }
        }else{

            gpio_put(RED_LED_PIN, 1);
            gpio_put(GREEN_LED_PIN, 0);
            gpio_put(BLUE_LED_PIN, 0);

            printf("Game Over!\nScore: %d\n", score);
            reset_game();
        }

    }

    set_one_led(led_r, led_g, led_b);
    last_interrupt_time = current_time;
}
