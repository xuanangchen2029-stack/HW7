#include <math.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define PIN_SHDN 21

#define VREF 3.3f

// More samples per cycle for smoother output
#define UPDATE_HZ 1000.0f
#define DT_US     1000

// Keep away from the rails to reduce apparent clipping
#define WAVE_MIN_V 0.15f
#define WAVE_MAX_V 2.90f

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

// Convert volts to 10-bit MCP4912 code using rounding instead of truncation
static inline uint16_t volts_to_code(float volts) {
    if (volts < 0.0f) volts = 0.0f;
    if (volts > VREF) volts = VREF;

    int code = (int)lroundf((volts / VREF) * 1023.0f);

    if (code < 0) code = 0;
    if (code > 1023) code = 1023;

    return (uint16_t) code;
}

// channel: 0 = DAC A, 1 = DAC B
// value: 0..1023 for MCP4912
void dac_write(unsigned char channel, unsigned short value) {
    uint16_t command = 0;

    // bit15 A/B: 0=A, 1=B
    command |= ((channel & 0x1) << 15);

    // bit14 BUF = 0 (unbuffered)
    // bit13 GA = 1 (1x gain)
    command |= (1u << 13);

    // bit12 SHDN = 1 (active mode)
    command |= (1u << 12);

    // 10-bit data in bits D9..D0, aligned to bits 11..2
    command |= ((value & 0x3FFu) << 2);

    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

float triangle_0_to_1(float t, float period) {
    float phase = fmodf(t, period) / period;

    if (phase < 0.5f) {
        return phase * 2.0f;        // 0 -> 1
    } else {
        return 2.0f - phase * 2.0f; // 1 -> 0
    }
}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(PIN_SHDN);
    gpio_set_dir(PIN_SHDN, GPIO_OUT);
    gpio_put(PIN_SHDN, 1);

    absolute_time_t next_time = get_absolute_time();
    float t = 0.0f;

    while (true) {
        float mid = 0.5f * (WAVE_MAX_V + WAVE_MIN_V);
        float amp = 0.5f * (WAVE_MAX_V - WAVE_MIN_V);

        // 2 Hz sine on DAC A
        // half-sample offset reduces repeated samples right at the peak
        float t_sine = t + 0.5f / UPDATE_HZ;
        float va = mid + amp * sinf(2.0f * (float)M_PI * 2.0f * t_sine);

        // 1 Hz triangle on DAC B
        float tri01 = triangle_0_to_1(t, 1.0f);
        float vb = WAVE_MIN_V + tri01 * (WAVE_MAX_V - WAVE_MIN_V);

        uint16_t code_a = volts_to_code(va);
        uint16_t code_b = volts_to_code(vb);

        dac_write(0, code_a);
        dac_write(1, code_b);

        t += 1.0f / UPDATE_HZ;
        next_time = delayed_by_us(next_time, DT_US);
        sleep_until(next_time);
    }

    return 0;
}
