/*
 * File:   GSM1.c
 * Author: aswin
 *
 * Created on 15 February, 2024, 12:53 PM
 */


#include <xc.h>

#include <stdlib.h>
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


#define _XTAL_FREQ 2000000  

void UART_Init() {
    TRISC6 = 0; 
    TRISC7 = 1; 

    SPBRG = 12; 
    BRGH = 0; 

    SYNC = 0; 
    SPEN = 1; 
    TXEN = 1; 
    CREN = 1; 
}


void UART_Write(char data) {
    while(!TXIF);
    TXREG = data; 
}


void UART_Write_String(const char *str) {
    while(*str)
        UART_Write(*str++);
}

void send_AT_command(const char *command) {
    UART_Write_String(command);
    UART_Write_String("\r\n");
    __delay_ms(1000); 
}

void make_call(const char *phone_number) {
    char command[20];
    send_AT_command(command);
}

void main() {
    UART_Init();

    while(1) {
      
        make_call("123456789"); 
        __delay_ms(30000); 
    }
}
