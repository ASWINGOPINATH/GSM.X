/*
 * File:   GSM3.c
 * Author: aswin
 *
 * Created on 16 February, 2024, 9:57 AM
 */

#include <xc.h>
#include<string.h>
#include<stdio.h>
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 20000000  

#define RS RB0

#define EN RB1

#define D4 RB2

#define D5 RB3

#define D6 RB4
#define D7 RB5

void lcd_init();
char g[30];
char m[30];
int j=0;
_Bool incoming=0;
_Bool message=0;
char phonenumber[11],c='\0';

void  __interrupt() ISR(){
   if(RCIF) {
       char r=RCREG;
       if(r=='\n'){
           g[j-1]='\0';
           j=0;
         if(!strcmp(g,"RING")){
             incoming=1;
         }
         else if(strstr(g,m)){
            
             message=1;
         }
          
       }
       else{
           g[j]=r;
           j++;
       }
   }
}
void cmd(unsigned char a);
void dat(unsigned char b);
void show(unsigned char *s);
void Init();
void LCD_Clear();
void SendCommand(const char* command);
unsigned char UART_Read();
void Call(const char* phoneNumber);
void Answer();
void End();
void SendMessage(const char* phoneNumber, const char* message);
void receiveSMS();
void UART_Read_String(char* output);
void processSMS(const char* message);
char key();
char rxBuffer[80];

void main() {
    TRISD = 0xf0;    
    TRISB=0;
    Init();
    unsigned int i;
    lcd_init();
    cmd(0x80); 
    show("CALLING");

    
   
    while(1) {
        
        c=key();
        if(c>='0'&&c<='9'){
            for(int i=0;i<=9;i++){
                phonenumber[i]=c;
                c=key();
            }
            phonenumber[10]='\0';
        if(c=='*'){
              cmd(0x80); 
               show("CALLING");
               __delay_ms(1);
              cmd(0x80); 
              show(phonenumber);
              Call(phonenumber);
              break;
        
          }
        if(c=='#'){
                End();
            }
        }
        if(c=='*'){
            if(incoming){
                Answer();
                cmd(0x80); 
                show("ANSWER");
            }
        else if(message){
                SendMessage(phonenumber, "Hello ");
                LCD_Clear();
                show("Message Sent");
            }
            
        else {
            SendCommand("ATDL\r\n");
            cmd(0x80); 
            show("REDIAL");
            
        }
        }
        
        if(c=='#'){
            End();
            LCD_Clear();
            cmd(0x80); 
            show("END");
                   
        }
        
        
    }
  
}
void Init() {
    
    SPEN=1; 
    TXEN= 1; 
    BRGH=1;
    SPBRG=129;
    SYNC=0;
    CREN=1;
    RCIE=1;
    PEIE=1;
    GIE=1;
    __delay_ms(100); 
    SendCommand("AT");
    TXREG='\r';
    __delay_ms(100); 
   
}

unsigned char UART_Read() {
    while(!RCIF); 
    return RCREG; 
}
void SendCommand(const char* command) {
    while(*command != '\0') {
        
        TXREG = *command;
        while(!TXIF); 
        command++;
    }
}
void lcd_init()
{
    cmd(0x02);
    cmd(0x28);
    cmd(0x0c);
    cmd(0x06);
    cmd(0x01);
    __delay_ms(2);
}

void cmd(unsigned char a)
{
    RS=0;
    D4=(a>>4)&1;
    D5=(a>>5)&1;
    D6=(a>>6)&1;
    D7=(a>>7)&1;
    EN=1;
    __delay_ms(1);
    EN=0;
    D4=a&1;
    D5=(a>>1)&1;
    D6=(a>>2)&1;
    D7=(a>>3)&1;
    EN=1;
    __delay_ms(1);
    EN=0;
}

void dat(unsigned char a)
{
    RS=1;
    D4=(a>>4)&1;
    D5=(a>>5)&1;
    D6=(a>>6)&1;
    D7=(a>>7)&1;
    EN=1;
    __delay_ms(1);
    EN=0;
    D4=a&1;
    D5=(a>>1)&1;
    D6=(a>>2)&1;
    D7=(a>>3)&1;
    EN=1;
    __delay_ms(1);
    EN=0;
}

void show(unsigned char *s)
{
    while(*s) {
        dat(*s++);
    }
}


void Call(const char* phoneNumber) {
    SendCommand("ATD"); 
    SendCommand(phoneNumber); 
    SendCommand(";\r\n"); 
}
 
void Answer(){
    SendCommand("ATA\r\n");
}
void End() {
    SendCommand("ATH\r\n");
}

void SendMessage(const char* phoneNumber, const char* message) {
    
    SendCommand("AT+CMGF=1\r"); 
    __delay_ms(1000);
    SendCommand("AT+CMGS=\""); 
    SendCommand(phoneNumber);
    SendCommand("\"\r");
    __delay_ms(1000); 
    SendCommand(message);
    SendCommand("\x1A");
    SendCommand("\r\n");
}
void receiveSMS() {
    SendCommand("AT+CMGF=1"); 
    SendCommand("AT+CNMI=2,2,0,0,0"); 
    __delay_ms(5000); 
    SendCommand("AT+CMGR=1"); 
    UART_Read_String(rxBuffer); 
    processSMS(rxBuffer);
    cmd(0x80); 
    show("RECEIVESMS");
}
void UART_Read_String(char* output) {
    unsigned int i = 0;
    char data;
    do {
        data = UART_Read();
        output[i++] = data;
    } while (data != '\0' && i < 255);
    output[i] = '\0';
}


void processSMS(const char* message) {
   
    SendCommand("Received SMS: ");
    SendCommand(message);
    SendCommand("\r\n");
}
void LCD_Clear()
{
    cmd(0x01);
}

char key(){
    while(1){
        RD0=1;RD1=RD2=RD3=0;
        if(RD4==1){
            while(RD4)
                printf("\a");
            return '1';   
        }
        if(RD5==1){
            while(RD5)
                printf("\a");
            return '2';   
        }
        if(RD7==1){
            while(RD7)
                printf("\a");
            return '3';   
        }
        RD1=1;RD0=RD2=RD3=0;
        if(RD4==1){
            while(RD4)
                printf("\a");
            return '4';   
        }
        if(RD5==1){
            while(RD5)
                printf("\a");
            return '5';   
        }
        if(RD7==1){
            while(RD7)
                printf("\a");
            return '6';   
        }
         RD2=1;RD0=RD1=RD3=0;
        if(RD4==1){
            while(RD4)
                printf("\a");
            return '7';   
        }
        if(RD5==1){
            while(RD5)
                printf("\a");
            return '8';   
        }
        if(RD7==1){
            while(RD7)
                printf("\a");
            return '9';   
        }
         RD3=1;RD0=RD1=RD2=0;
        if(RD4==1){
            while(RD4)
                printf("\a");
            return '*';   
        }
        if(RD5==1){
            while(RD5)
                printf("\a");
            return '0';   
        }
        if(RD7==1){
            while(RD7)
                printf("\a");
            return '#';   
        }
    }
  
}
