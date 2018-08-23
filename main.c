/*
 * File:   main.c
 * Author: zakrent
 *
 * Created on August 23, 2018, 6:04 PM
 */

#pragma config OSC = IntRC
#pragma config WDT = OFF    
#pragma config CP = OFF       
#pragma config MCLRE = OFF
#pragma config IOSCFS = ON

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 8000000

#define BUT_0 GP3
#define DIS_0 GP0
#define DIS_1 GP2
#define SER_DAT GP4
#define SER_CLK GP5
#define BUZZER GP1

inline void setup(){
    OSCCAL = 0b01000000; //OSCCAL setup manualy because fine tuned (original 0x0C1C)
    OPTION = PS0 | PS1 | PS2;
    CM1CON0 = 0; //Disable comparator
    ANS0 = 0; //Disconnect ADC pins
    ANS1 = 0;
    TRISGPIO = 0b00001000;
    GPIO = 0x00;
}

void serial_send(char byte){
    for(uint8_t i = 0; i < 8; i++){
        SER_CLK = 0;
        __delay_us(1);
        if((byte & 0x01) == 1){
            SER_DAT = 1;
        }
        else{
            SER_DAT = 0;
        }
        __delay_us(1);
        SER_CLK = 1;
        byte = byte >> 1;
    }
    SER_CLK = 0;
    __delay_us(1);
    SER_DAT = 0;
}

char ssd_lookup(uint8_t value){
    switch(value){
        case 0:
            return 0b10000001;
            break;
        case 1:
            return 0b11001111;
            break;
        case 2:
            return 0b10010010;
            break;
        case 3:
            return 0b10000110;
            break;
        case 4:
            return 0b11001100;
            break;
        case 5:
            return 0b10100100;
            break;
        case 6:
            return 0b10100000;
            break;
        case 7:
            return 0b10001111;
            break;
        case 8:
            return 0b10000000;
            break;
        case 9:
            return 0b10000100;
            break;
        default:
            return 0x00;
    }
}

void update_display(uint8_t value){
    uint8_t display_value = 0;
    if(DIS_0 == 1){
        DIS_0 = 0;
        display_value = value - (value/10)*10;
        serial_send(ssd_lookup(display_value));
        DIS_1 = 1;
    }
    else{
        DIS_1 = 0;
        display_value = value/10;
        serial_send(ssd_lookup(display_value));
        DIS_0 = 1;                
    }
}

uint8_t configure(){
    uint8_t time_left = 5;
    while(1){
        while(BUT_0){
            update_display(time_left);
            __delay_ms(1);
        }
        uint8_t time_pressed_counter = 0;
        BUZZER=1;
        while(!BUT_0){
            update_display(time_left);
            __delay_ms(1);
            if(time_pressed_counter < 200){
                time_pressed_counter++;
            }
        }
        BUZZER=0;
        if(time_pressed_counter < 200){
            time_left += 5;
            if(time_left == 100){
                time_left = 0;
            }
        }
        else{
            return time_left;
        }
    }
}

void count(uint8_t time_left){
    uint8_t update_counter = 0;
    while(1){
        if(!BUT_0){
            return;
        }
        update_counter++;
        if(update_counter == 100){
            update_counter = 0;
            if(time_left ==0){
                return;
            }
            time_left--;
        }
        update_display(time_left);
        __delay_ms(10);
    }
}

void ring(){
    DIS_0 = 0;
    DIS_1 = 0;
    while(1){
        if(!BUT_0){
            BUZZER = 0;
             while(!BUT_0){}
             __delay_ms(10);
             return;
        }
        BUZZER = !BUZZER;
        __delay_ms(100);
    }
}

void main(void) {
    setup();
    BUZZER = 1;
    __delay_ms(500);
    BUZZER = 0;
    while(1){
        uint8_t time_set = configure();
        count(time_set);
        ring();
    }
}
