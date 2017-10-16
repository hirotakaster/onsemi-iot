/*
============================================================================
 Name : main.cpp
 Author : hirotakaster
 Version :
 Copyright : Your copyright notice
 Description : Exe source file

Required:                   
    Connect Power through USB/External supply 12v
             ___________
            |   IDK     |
            |___________|
 
 ============================================================================
*/

#include "mbed.h"
#include "Shields.h"

NHD_C0216CZ lcd;

DigitalOut led(p15);

int main() {

	lcd.init();
	lcd.displayString("GPIO led");

    while(1) {
    	led = !led;
    	wait(1);
    }
}
