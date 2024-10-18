#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdbool.h>

#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof((_arr)[0]))

#define BAUD_RATE 9600UL
#define USART_BAUD_RATE(_br) ((F_CPU * 4 + (_br) / 2) / (_br))

static void uart_init(void) {
    USART3.BAUD = USART_BAUD_RATE(BAUD_RATE);
    USART3.CTRLB = USART_TXEN_bm;

    /* Sent USART 3 on PB[5:4] */
    PORTMUX.USARTROUTEA |= PORTMUX_USART3_ALT1_gc;

    /* PIN 4 TX*/
    PORTB.DIR |= PIN4_bm;
}

static int putchar(int c) {
    while (!(USART3.STATUS & USART_DREIF_bm)) {
        ;
    }

    USART3.TXDATAL = c;

    return c;
}

static int puts(const char *s) {
    int n = 0;
    while (*s) {
        putchar(*s);
        s++;
        n++;
    }

    putchar('\r');
    putchar('\n');

    return n;
}

int main(void) {
    cli();

    /* Disable divider to run at full 20MHz */
    _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);

    uart_init();

    for (;;) {
        puts("Hello world! How are you now?");
        _delay_ms(1000);
    }

    return 0;
}
