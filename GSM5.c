#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


#define _XTAL_FREQ 20000000 // 20MHz Crystal


#define RS RB0
#define EN RB1
#define D4 RB2
#define D5 RB3
#define D6 RB4
#define D7 RB5


#define KEYPAD_PORT PORTD
#define ROW1 RD0
#define ROW2 RD1
#define ROW3 RD2
#define ROW4 RD3
#define COL1 RD4
#define COL2 RD5
#define COL3 RD6


#define GSM_TX RC6
#define GSM_RX RC7


void LCD_Init();
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char data);
void LCD_String(const char *str);
void LCD_Clear();
char keypad_scan();
void GSM_Init();
void GSM_Send(const char *command);
void GSM_Call(const char *number);
void GSM_EndCall();
void GSM_Redial();

void main() {
    TRISD = 0xF0; // Lower 4 bits are output (Rows), higher 4 bits are input (Columns)
    TRISB = 0; // Port B as output for LCD
    TRISC6 = 0;    // TX Pin as Output
    TRISC7 = 1;    // RX Pin as Input

    LCD_Init();
    GSM_Init();

    while (1) {
        
        char key = keypad_scan();
        if (key != '\0') {
            switch (key) {
                case 'A':
                    GSM_Call("ATD8943882327;");
                    break;
                case 'B':
                    GSM_EndCall();
                    break;
                case 'C':
                    GSM_Redial();
                    break;
            }
        }
    }
}

void LCD_Init() {
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0c);
    LCD_Command(0x06);
    LCD_Command(0x01);
}

void LCD_Command(unsigned char cmd) {
    RS = 0;
    PORTB = cmd;
    EN = 1;
    __delay_ms(4);
    EN = 0;
}

void LCD_Char(unsigned char data) {
    RS = 1;
    PORTB = data;
    EN = 1;
    __delay_ms(4);
    EN = 0;
}

void LCD_String(const char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        LCD_Char(str[i]);
    }
}

void LCD_Clear() {
    LCD_Command(0x01);
}

char keypad_scan() {
    ROW1 = 0; ROW2 = 1; ROW3 = 1; ROW4 = 1;
    if (COL1 == 0)
    {
        __delay_ms(20); 
    while (COL1 == 0);
    return '1'; 
    }
    if (COL2 == 0)
    {
        __delay_ms(20); 
    while (COL2 == 0); 
    return '2';
    }
    if (COL3 == 0)
    {
        __delay_ms(20); 
        while (COL3 == 0);
        return '3'; 
    }

    ROW1 = 1; ROW2 = 0; ROW3 = 1; ROW4 = 1;
    if (COL1 == 0)
    { 
        __delay_ms(20); 
        while (COL1 == 0);
        return '4';
    }
    if (COL2 == 0) 
    { 
        __delay_ms(20);
        while (COL2 == 0); 
        return '5'; 
    }
    if (COL3 == 0)
    { 
        __delay_ms(20);
        while (COL3 == 0);
        return '6'; 
    }

    ROW1 = 1; ROW2 = 1; ROW3 = 0; ROW4 = 1;
    if (COL1 == 0) 
    { 
        __delay_ms(20);
        while (COL1 == 0);
        return '7'; 
    }
    if (COL2 == 0) 
    {
        __delay_ms(20); 
        while (COL2 == 0);
        return '8'; 
    }
    if (COL3 == 0) 
    { 
        __delay_ms(20);
        while (COL3 == 0);
        return '9'; 
    }

    ROW1 = 1; ROW2 = 1; ROW3 = 1; ROW4 = 0;
    if (COL1 == 0) 
    {
        __delay_ms(20); 
        while (COL1 == 0); 
        return 'A'; 
    }
    if (COL2 == 0)
    {
        __delay_ms(20); 
        while (COL2 == 0); 
        return '0'; 
    }
    if (COL3 == 0)
    { 
        __delay_ms(20);
        while (COL3 == 0);
        return 'B'; 
    }

    return '\0'; // No key pressed
}

void GSM_Init() {
    // Initialize UART
    SPBRG = 25; // for 9600 baud rate
    TXSTAbits.SYNC = 0; // Asynchronous mode
    TXSTAbits.BRGH = 1; // High speed
    RCSTAbits.SPEN = 1; // Enable serial port pins
    TXSTAbits.TXEN = 1; // Enable transmitter

    __delay_ms(300); 
    GSM_Send("AT");
    __delay_ms(100);
}

void GSM_Send(const char *command) {
    while (*command != '\0') {
        TXREG = *command++;
        while (!TXSTAbits.TRMT);
    }
    TXREG = '\r';
    while (!TXSTAbits.TRMT);
    TXREG = '\n';
    while (!TXSTAbits.TRMT);
}

void GSM_Call(const char *number) {
    char call_command[20];
    sprintf(call_command, "ATD%s;", number);
    GSM_Send(call_command);
    LCD_Clear();
    LCD_String("Calling...");
}

void GSM_EndCall() {
    GSM_Send("ATH");
    LCD_Clear();
    LCD_String("Call Ended");
}

void GSM_Redial() {
    GSM_Send("ATDL");
    LCD_Clear();
    LCD_String("Redialing...");
}
