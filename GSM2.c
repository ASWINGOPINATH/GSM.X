/*
 * File:   GSM2.c
 * Author: aswin
 *
 * Created on 15 February, 2024, 4:16 PM
 */
#include <xc.h>

#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define BAUDRATE 9600
void LCD_command(unsigned char cmd);
void LCD_data(unsigned char data);
void LCD_init();
void LCD_clear();
void LCD_string(const char *str);
void UART_Init() {
    TRISC6 = 0; 
    TRISC7 = 1; 
    TXSTA = 0x20; 
    RCSTA = 0x90; 
    SPBRG = (__XTAL_FREQ / (16 * BAUDRATE)) - 1;
}

void UART_SendChar(char data) {
    while (!TXIF); 
    TXREG = data;  
}

char UART_ReceiveChar() {
    while (!RCIF); 
    return RCREG;  
}

void Keypad_Init() {
    
    TRISD = 0xF0; 
    PORTD = 0x00; 
}


char Keypad_Scan() {
    unsigned char col, row;
    const unsigned char keypad[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    for (col = 0; col < 4; col++) {
        PORTD = 0xFF; 
        PORTD &= ~(0x10 << col); 

        for (row = 0; row < 4; row++) {
            if (!(PORTD & (1 << row))) { 
                while (!(PORTD & (1 << row))); 
                return keypad[row][col]; 
            }
        }
    }

    return '\0'; 
}
void LCD_command(unsigned char cmd) {
    PORTD = cmd;
    RB0 = 0; 
    RB1 = 0; 
    RB2 = 1; 
    __delay_ms(1);
    RB2 = 0;
}
void LCD_data(unsigned char data) {
    PORTD = data;
    RB0 = 1; 
    RB1 = 0; 
    RB2 = 1; 
    __delay_ms(1);
    RB2 = 0;
}


void LCD_init() {
    TRISB=0;
    PORTB=0;
    LCD_command(0x38); 
    LCD_command(0x0E); 
    LCD_command(0x01); 
    __delay_ms(2);
    LCD_command(0x06); 
}

void LCD_clear() {
    LCD_command(0x01); 
    __delay_ms(2);
}

void LCD_string(const char *str) {
    while(*str) {
        LCD_data(*str++);
    }
}

void main() {
    char number[12]; 
    unsigned char i = 0;

    UART_Init(); 
    Keypad_Init(); 
    LCD_init();
   
    while (1) {
        char key = Keypad_Scan();

        if (key != '\0' && i < 11) { 
            number[i++] = key; 
            UART_SendChar(key);
             LCD_data(key); 
        }

        if (key == '#' || i >= 11) {
            number[i] = '\0'; 
            UART_Send_String("ATD");
            UART_Send_String(number); 
            UART_SendChar(';');
            break; 
        }
    }
}
