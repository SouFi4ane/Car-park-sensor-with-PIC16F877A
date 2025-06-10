#define _XTAL_FREQ 4000000  // 4 MHz crystal

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7
#define LCDBIT TRISD

#define TRIG RB0
#define ECHO RB1

#include <xc.h>
#include <stdio.h>

// CONFIGURATION BITS
#pragma config FOSC = HS, WDTE = OFF, PWRTE = ON, BOREN = ON
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF

// ============ LCD FUNCTIONS ============

void Lcd_SetBit(char data_bit) {
    D4 = (data_bit >> 0) & 1;
    D5 = (data_bit >> 1) & 1;
    D6 = (data_bit >> 2) & 1;
    D7 = (data_bit >> 3) & 1;
}

void Lcd_Cmd(char a) {
    RS = 0;
    Lcd_SetBit(a);
    EN = 1;
    __delay_ms(4);
    EN = 0;
}

void Lcd_Clear() {
    Lcd_Cmd(0);
    Lcd_Cmd(1);
}

void Lcd_Set_Cursor(char row, char col) {
    char pos = (row == 1) ? (0x80 + col - 1) : (0xC0 + col - 1);
    Lcd_Cmd(pos >> 4);
    Lcd_Cmd(pos & 0x0F);
}

void Lcd_Start() {
    LCDBIT = 0x00;  // TRISD = 0 (output)
    Lcd_SetBit(0x00);
    __delay_ms(20);

    Lcd_Cmd(0x03); __delay_ms(5);
    Lcd_Cmd(0x03); __delay_ms(11);
    Lcd_Cmd(0x03);
    Lcd_Cmd(0x02);  // Set 4-bit mode

    Lcd_Cmd(0x02);
    Lcd_Cmd(0x08);
    Lcd_Cmd(0x00);
    Lcd_Cmd(0x0C);  // Display ON, cursor OFF
    Lcd_Cmd(0x00);
    Lcd_Cmd(0x06);  // Entry mode
}

void Lcd_Print_Char(char data) {
    char lower = data & 0x0F;
    char upper = data >> 4;
    RS = 1;
    Lcd_SetBit(upper); EN = 1; __delay_us(50); EN = 0;
    Lcd_SetBit(lower); EN = 1; __delay_us(50); EN = 0;
}

void Lcd_Print_String(char *str) {
    while (*str) {
        Lcd_Print_Char(*str++);
    }
}

// ============ ULTRASONIC FUNCTIONS ============

void init_timer1() {
    T1CON = 0x10; // Prescaler 1:2, Timer1 ON
}

unsigned int get_distance() {
    unsigned int time;

    TRIG = 1;
    __delay_us(10);
    TRIG = 0;

    while (!ECHO);               // Wait for echo to go high
    TMR1H = 0; TMR1L = 0;
    T1CONbits.TMR1ON = 1;

    while (ECHO);                // Wait for echo to go low
    T1CONbits.TMR1ON = 0;

    time = (TMR1H << 8) | TMR1L;

    return time / 116;           // Accurate formula for 4 MHz (0.5 µs tick)
}

// ============ MAIN FUNCTION ============

void main() {
    // Set directions
    TRISB0 = 0;  // TRIG
    TRISB1 = 1;  // ECHO
    TRISD = 0x00; // LCD (D4–D7 + RS, EN)

    init_timer1();
    Lcd_Start();

    // Initial message
    Lcd_Set_Cursor(1, 1);
    Lcd_Print_String("System Ready");
    Lcd_Set_Cursor(2, 1);
    Lcd_Print_String("Ultrasonic OK");
    __delay_ms(2000);
    Lcd_Clear();

    char buffer[16];

    while (1) {
        unsigned int distance = get_distance();

        sprintf(buffer, "Distance=%2d cm", distance);
        Lcd_Set_Cursor(1, 1);
        Lcd_Print_String("                "); // Clear line
        Lcd_Set_Cursor(1, 1);
        Lcd_Print_String(buffer);

        __delay_ms(500);
    }
}