/*
 * File:   GSM4.c
 * Author: aswin
 *
 * Created on 16 February, 2024, 12:38 PM
 */


#include <xc.h>
#include <string.h>   
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define KEYPAD_PORTD PORTD
#define KEYPAD_PORTB PORTB
#define ROW1 RD7:RD6:RD5:RD4
#define ROW2 RD3:RD2:RD1:RD0
#define COL1 RB3:RB2
#define COL2 RB1:RB0

char keypad_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

char important_number[] = "8943882327";

void send_command(const char *command) {
    while (*command) {
        TXREG = *command++;
        __delay_us(100); 
    }
    TXREG = '\r';
    __delay_ms(100); 
}


char *read_response() {
    static char response[80];
    unsigned int i = 0;
    while (RCREG != '\n' && i < sizeof(response) - 1) {
        response[i++] = RCREG;
        __delay_us(100); 
    }
    response[i] = '\0';
    return response;
}


int check_ok() {
    char *response = read_response();
    return strncmp(response, "OK", 2) == 0;
}


void dial_and_handle_call(const char *number) {
    send_command("ATD"); 
    send_command(number); 
    send_command(";\r");   

    __delay_ms(5000); 
    if (!check_ok()) {
        
        return;
    }

    
    unsigned long start_time = _ms_counter;
    while (_ms_counter - start_time < 10000) { 
        if (keypad_scan()) {
            
            send_command("ATH\r");
            return;
        }
    }

    
    send_command("ATH\r");
}


char keypad_scan() {
    
    KEYPAD_PORTD = 0xFF;
    KEYPAD_PORTB = 0xF0; 

    
    for (int i = 0; i < 4; i++) {
        KEYPAD_PORTD = ~(1 << i);

        
        char value = KEYPAD_PORTB & 0x0F;

       
        for (int j = 0; j < 4; j++) {
            if (!(value & (1 << j))) {
                return keypad_map[i][j];
            }
        }
    }
