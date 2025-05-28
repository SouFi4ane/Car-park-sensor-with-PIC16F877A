
#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 20000000  // 20 MHz

// CONFIGURATION
#pragma config FOSC = HS, WDTE = OFF, PWRTE = ON, BOREN = ON
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF

// Pin definitions
#define TRIG1 RD0
#define ECHO1 RD1
#define TRIG2 RD2
#define ECHO2 RD3
#define LED   RB0

void init() {
    TRISD0 = 0; // TRIG1
    TRISD1 = 1; // ECHO1
    TRISD2 = 0; // TRIG2
    TRISD3 = 1; // ECHO2
    TRISB0 = 0; // LED

    TRIG1 = 0;
    TRIG2 = 0;
    LED = 0;

    // Timer0 config: internal clock, prescaler 1:64
    OPTION_REG = 0b10000100;  // T0CS=0, PSA=0, PS=100 (1:64)
}

uint16_t measure_distance(uint8_t trig_bit, uint8_t echo_bit) {
    uint16_t overflow = 0;
    uint16_t count = 0;

    // Trigger pulse
    if (trig_bit == 0) RD0 = 1;
    if (trig_bit == 2) RD2 = 1;
    __delay_us(12);
    if (trig_bit == 0) RD0 = 0;
    if (trig_bit == 2) RD2 = 0;

    // Wait for echo to go HIGH
    while (!(PORTD & (1 << echo_bit)));

    // Start measuring echo HIGH duration
    TMR0 = 0;
    overflow = 0;

    while (PORTD & (1 << echo_bit)) {
        if (TMR0 == 255) {
            TMR0 = 0;
            overflow++;
            if (overflow > 30) break;  // Timeout
        }
    }

    count = (overflow * 256) + TMR0;
    float time_us = count * 12.8; // Each tick is 12.8us (with prescaler 64)
    return (uint16_t)(time_us / 58); // Convert to cm
}

void main() {
    init();
    uint16_t d1, d2;

    while (1) {
        d1 = measure_distance(0, 1); // front: RD0/RD1
        d2 = measure_distance(2, 3); // back: RD2/RD3

        if (d1 < 20 || d2 < 20) {
            // If close → blink LED
            LED = 1;
            __delay_ms(200);
            LED = 0;
            __delay_ms(200);
        } else {
            // Else keep LED solid ON
            LED = 1;
        }

        __delay_ms(100); // optional pause
    }
}
