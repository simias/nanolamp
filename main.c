#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>

#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof((_arr)[0]))

#define BAUD_RATE 9600UL
#define USART_BAUD_RATE(_br) ((F_CPU * 4 + (_br) / 2) / (_br))

static int uart_putchar(int c) {
    while (!(USART3.STATUS & USART_DREIF_bm)) {
        ;
    }

    USART3.TXDATAL = c;

    return c;
}

static int uart_putchar_stream(char c, FILE *stream) {
    if (c == '\n') {
        uart_putchar('\r');
    }

    return uart_putchar(c);
}

static FILE uart_stream = FDEV_SETUP_STREAM(uart_putchar_stream, NULL, _FDEV_SETUP_WRITE);

static void uart_init(void) {
    USART3.BAUD = USART_BAUD_RATE(BAUD_RATE);
    USART3.CTRLB = USART_TXEN_bm;

    /* Sent USART 3 on PB[5:4] */
    PORTMUX.USARTROUTEA |= PORTMUX_USART3_ALT1_gc;

    /* PIN 4 TX*/
    PORTB.DIR |= PIN4_bm;

    stdout = &uart_stream;
}


static void adc_init(void)
{
    /* PD5 input*/
    /* PORTD.DIR &= ~PIN5_bm; */

    /* Disable digital input buffer */
    PORTD.PIN5CTRL &= ~PORT_ISC_gm;
    PORTD.PIN5CTRL |= PORT_ISC_INPUT_DISABLE_gc;

    /* Disable pull-up resistor */
    PORTD.PIN5CTRL &= ~PORT_PULLUPEN_bm;

    /* Clock divider must be selected so that we don't overclock the ADC. In
     * 10bits according the the datasheet the freq should be between 200 and
     * 1500kHz (200 - 2000kHz in 8bits)
     */
    ADC0.CTRLC = ADC_PRESC_DIV64_gc
               | ADC_REFSEL_VDDREF_gc;

    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */

    /* Select ADC channel */
    ADC0.MUXPOS  = ADC_MUXPOS_AIN5_gc;

    /* Acculumate 64 samples because why not */
    ADC0.CTRLB = ADC_SAMPNUM_ACC64_gc;

}

static uint16_t adc_read(void) {
    ADC0.COMMAND = ADC_STCONV_bm;

    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) {
        ;
    }
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    /* Divide by 64 since we accumulate 64 samples */
    return ADC0.RES >> 6;
}

static void pwm_init(void) {
    /* Use PB0 */
    PORTB.DIR |= PIN0_bm;
    PORTMUX.TCAROUTEA |= PORTMUX_TCA0_PORTB_gc;

    /* Period */
    TCA0.SINGLE.PER = 0xFF;

    /* Duty cycle */
    TCA0.SINGLE.CMP0 = 0x10;

    /* Set divider */
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm;

    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
}

int main(void) {
    cli();

    /* Disable divider to run at full 20MHz, otherwise the default value is
     * 0b1001 which is equal to /6 or 3.333MHz.
     * https://onlinedocs.microchip.com/oxy/GUID-8109B192-2AF1-4902-BA7D-C3C6DA7BDC69-en-US-3/GUID-205EC738-D303-46E3-AAD4-5F1FB6C357A1.html */
    _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);

    uart_init();
    puts("Starting up...");

    adc_init();

    pwm_init();

    for (;;) {
        /* We get a value in the range 0;1023 */
        uint16_t v = adc_read();

        TCA0.SINGLE.CMP0 = v >> 2;

        _delay_ms(100);
    }

    return 0;
}
