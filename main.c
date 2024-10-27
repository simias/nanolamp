#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdbool.h>

#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof((_arr)[0]))

struct led_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

#define NLEDS 36U

static struct led_color leds[NLEDS];

/* LED color as sent on the SPI. We need 6 bits per color bit */
static uint8_t led_spi_encoded[NLEDS * 3 * 6];

static void led_spi_set_bit(uint16_t bitpos, uint8_t b) {
    uint16_t byte = bitpos >> 3;
    unsigned shift = bitpos & 7;

    b = !!b;

    led_spi_encoded[byte] |= b << shift;
}

ISR(PORTB_PORT_vect) {
    /* ACK */
    PORTB.INTFLAGS = PORT_INT0_bm;
}

static uint8_t switch_state = 0;

static bool switch_toggled(void) {
    uint8_t ss = (PORTB.IN & (1U << PIN0_bp));

    if (ss != switch_state) {
        switch_state = ss;
        return true;
    }

    return false;
}

static void spi_init(void) {
    PORTMUX.TWISPIROUTEA &= ~PORTMUX_SPI0_gm;
    PORTMUX.TWISPIROUTEA |= PORTMUX_SPI0_1_bm;

    /* Set MOSI pin direction to output */
    PORTE.DIR |= PIN0_bm;

    /* Set Clock as an output (this is also the built-in LED) */
    PORTE.DIR |= PIN2_bm;


    SPI0.CTRLA = SPI_DORD_bm            /* LSB is transmitted first */
               | SPI_ENABLE_bm          /* Enable module */
               | SPI_MASTER_bm          /* SPI module in Master mode */
               | SPI_PRESC_DIV4_gc;     /* System Clock divided by 4 */

    /* Enable double buffering and MODE0 */
    SPI0.CTRLB = SPI_BUFEN_bm | SPI_MODE_0_bm;
}

static void spi_send_encoded() {
    /* We need at least 50us of zero to reset */
    for (unsigned i = 0; i < 15; i++) {
        while (!(SPI0.INTFLAGS & SPI_DREIF_bm)) {};
        SPI0.DATA = 0;
    }

    for (unsigned i = 0; i < ARRAY_SIZE(led_spi_encoded); i++) {
        /* Wait for data register empty */
        while (!(SPI0.INTFLAGS & SPI_DREIF_bm)) {};
        /* SPI0.DATA = led_spi_encoded[i] | 0xff; */
        SPI0.DATA = led_spi_encoded[i];
    }
}

static void spi_send_leds() {
    /* We need to output the LEDs very fast and without interruption since the
     * protocol uses fixed timings to detect ones and zeroes (it's not real
     * SPI). This chip is not fast enough to do both the encoding and sending at
     * the same time without breaks between bytes, so we need to precompute
     * everything. */
    uint16_t bitpos = 0;

    wdt_reset();
    for (unsigned i = 0; i < ARRAY_SIZE(led_spi_encoded); i++) {
        led_spi_encoded[i] = 0x00;
    }

    for (unsigned led = 0; led < NLEDS; led++) {
        uint32_t c = 0;
        c |= leds[led].g;
        c <<= 8;
        c |= leds[led].r;
        c <<= 8;
        c |= leds[led].b;

        for (unsigned b = 0; b < 24; b++) {
            unsigned high = (c >> (23 - b)) & 1;

            /* It's hard to get clean timing with the Atmel because the SPI
             * tends to insert small pauses between bytes which mess the timings
             * up. Theoretically we could do with only 3 bit per LED bit: 100 or
             * 110, but it doesn't work well in my experiments.
             *
             * Instead I double the SPI clock and do this: 100000 for 0, 111100
             * for 1. That means that the 0 pulses are a bit below spec */
            led_spi_set_bit(bitpos++, 1);
            led_spi_set_bit(bitpos++, high);
            led_spi_set_bit(bitpos++, high);
            led_spi_set_bit(bitpos++, high);
            led_spi_set_bit(bitpos++, 0);
            led_spi_set_bit(bitpos++, 0);
        }
    }

    spi_send_encoded();
}

#define ANIMATION_DELAY 0

static void mode_off(void) {
    for (unsigned i = 0; i < NLEDS; i++) {
        leds[i].r = 0;
        leds[i].g = 0;
        leds[i].b = 0;
        spi_send_leds();
        _delay_ms(ANIMATION_DELAY);
    }
}

#define FULL_RED   250
#define FULL_GREEN 75
#define FULL_BLUE  10

static void mode_full_power(void) {
    for (unsigned i = 0; i < NLEDS; i++) {
        leds[i].r = FULL_RED;
        leds[i].g = FULL_GREEN;
        leds[i].b = FULL_BLUE;
        spi_send_leds();
        _delay_ms(ANIMATION_DELAY);
    }
}

static void mode_mid_power(void) {
    for (unsigned i = 0; i < NLEDS; i++) {
        leds[i].r = FULL_RED / 3;
        leds[i].g = FULL_GREEN / 3;
        leds[i].b = FULL_BLUE / 3;
        spi_send_leds();
        _delay_ms(ANIMATION_DELAY);
    }
}

static void mode_orange(void) {
    for (unsigned i = 0; i < NLEDS; i++) {
        leds[i].r = 220;
        leds[i].g = 30;
        leds[i].b = 0;
        spi_send_leds();
        _delay_ms(ANIMATION_DELAY);
    }
}

static void mode_red(void) {
    for (unsigned i = 0; i < NLEDS; i++) {
        leds[i].r = FULL_RED;
        leds[i].g = 0;
        leds[i].b = 0;
        spi_send_leds();
        _delay_ms(ANIMATION_DELAY);
    }
}

static void mode_red_low_power(void) {
    for (unsigned i = 0; i < NLEDS; i++) {
        leds[i].r = FULL_RED / 8;
        leds[i].g = 0;
        leds[i].b = 0;
        spi_send_leds();
        _delay_ms(ANIMATION_DELAY);
    }
}
typedef void (*mode_handled_t)(void);

static mode_handled_t mode_handlers[] = {
    mode_off,
    mode_full_power,
    mode_mid_power,
    mode_orange,
    mode_red,
    mode_red_low_power,
};

int main(void) {
    unsigned mode= 0;

    cli();

    /* Disable divider to run at full 20MHz */
    _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);

    wdt_enable(WDTO_1S);

    /* rtc_init(); */
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    spi_init();

    /* Use PB0 (D9) as switch input with a pull up */
    PORTB.DIR &= ~(1U << PIN0_bp);
    /* Activate Pull-up + IRQ */
    PORTB.PIN0CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;

    switch_toggled();

    for (unsigned i = 0; i < NLEDS; i+=3) {
        leds[i].r = 0x0;
        leds[i].g = 0x0;
        leds[i].b = 0x0;
    }

    spi_send_leds();

    while (1) {
        /* Wait for switch activity */
        wdt_disable();
        sei();
        sleep_cpu();
        cli();
        wdt_enable(WDTO_1S);

        /* Debounce delay */
        _delay_ms(20);

        if (!switch_toggled()) {
            /* Spurious switch activity, ignore */
            continue;
        }

        mode = (mode + 1) % ARRAY_SIZE(mode_handlers);

        mode_handlers[mode]();
    }

    return 0;
}
