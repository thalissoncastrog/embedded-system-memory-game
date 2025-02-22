#include <stdio.h>
#include "pico/stdlib.h"
#include "interruptions_counter.pio.h"

#define NUM_PIXELS 25
#define LED_MATRIX_PIN 7

#define IS_RGBW false

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

int main()
{
    stdio_init_all();

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &interruptions_counter_program);

    interruptions_counter_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);

    set_one_led(led_r, led_g, led_b);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
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
