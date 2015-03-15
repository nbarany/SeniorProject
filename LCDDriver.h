/*
 * LCDDriver.h
 *
 * Created: 4/14/2014 8:12:24 AM
 *  Author: Sheng-Han Chen, Nicholas Barany
 */ 


#ifndef LCDDRIVER_H_
#define LCDDRIVER_H_
#define F_CPU 16000000UL //board's clock speed needed for delay.h
#include <stdlib.h>                    // Standard C library
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>

int LCD_4pin_init(int e, int rs, int rw, int db7, int db6, int db5, int db4); //4 bit initialization
int LCD_8pin_init(int e, int rs, int rw, int db7, int db6, int db5, int db4,
int db3, int db2, int db1, int db0); //8 bit pin initialization;
int LCD_send_data(char data); //Sends data to CGRAM/DDRAM depending on last set.
int LCD_cmd_clr(); //Clear screen
int LCD_cmd_home(); //Return to home
int LCD_cmd_entry(int increment, int shift); //Entry settings
int LCD_cmd_display(int display, int cursor, int blink); //Display settings
int LCD_cmd_shift(int shift, int direction); //Shift screen/cursor
int LCD_cmd_function(int bit, int line, int font); //Mode settings
int LCD_cmd_cgram(int addr); //CGRAM addr setting
int LCD_cmd_ddram(int addr); //DDRAM addr setting
int LCD_newLine(); //Move to second line.


#endif /* LCDDRIVER_H_ */
