/*
 * LCDDriver.c
 *
 * Created: 4/14/2014 8:11:26 AM
 *  Author: Sheng-Han Chen, Nicholas Barany
 */ 

#include "LCDDriver.h"

//Command values here.
#define LCD_CLR 0x01
#define LCD_HOME 0x02
#define LCD_ENTRY 0x04
#define LCD_DISPLAY 0x08
#define LCD_SHIFT 0x10
#define LCD_FUNCTION 0x20
#define LCD_CGRAM 0x40
#define LCD_DDRAM 0x80

/** The Pin numbers on the Arduino corresponding to the pins on the LCD screen; default values below.
*   Values of A(n) correspond to N + 14, reset is 20.
**/
static int lcd_e = 12;
static int lcd_rs = 11;
static int lcd_rw = 10;
static int lcd_db7 = 7;
static int lcd_db6 = 6;
static int lcd_db5 = 5;
static int lcd_db4 = 4;
static int lcd_db3 = 3;
static int lcd_db2 = 2;
static int lcd_db1 = 1;
static int lcd_db0 = 0;

/**4 bit or 8 bit mode; default 8**/
static int bit_mode = 8;

int LCD_4pin_init(int e, int rs, int rw, int db7, int db6, int db5, int db4);
int LCD_8pin_init(int e, int rs, int rw, int db7, int db6, int db5, int db4,
int db3, int db2, int db1, int db0);
int LCD_send_data(char data);
int LCD_cmd_clr();
int LCD_cmd_home();
int LCD_cmd_entry(int increment, int shift);
int LCD_cmd_display(int display, int cursor, int blink);
int LCD_cmd_shift(int shift, int direction);
int LCD_cmd_function(int bit, int line, int font);
int LCD_cmd_cgram(int addr);
int LCD_cmd_ddram(int addr);
int LCD_newLine();

//Internals
char LCD_io_port(int pin);
int LCD_io_shift(int pin);
int LCD_send(int pin, int bit_one);
int LCD_clear_data();
int LCD_8_bit_cmd(char cmd);
int LCD_4_bit_cmd(char cmd);
int LCD_send_8data(char data);
int LCD_send_4data(char data);
int LCD_send_cmd(char cmd);
int LCD_activate(int pin);

//Initialization for 4 data pins
int LCD_4pin_init(int e, int rs, int rw, int db7, int db6, int db5, int db4) {
    lcd_e = e;
    lcd_rs = rs;
    lcd_rw = rw;
    lcd_db7 = db7;
    lcd_db6 = db6;
    lcd_db5 = db5;
    lcd_db4 = db4;
    
    //Set unused to -1; this will make it so that even if 'send' function is used
    //to these pins, there will be no reaction.
    lcd_db3 = -1;
    lcd_db2 = -1;
    lcd_db1 = -1;
    lcd_db0 = -1;
    
    //Set the corresponding pins to output
    LCD_activate(e);
    LCD_activate(rs);
    LCD_activate(rw);
    LCD_activate(db7);
    LCD_activate(db6);
    LCD_activate(db5);
    LCD_activate(db4);
    
    LCD_cmd_function(0, 0, 0); //Set to 4 bit mode, 1 line, display off
    
    return 1;
}

//Initialization for 8 data pins.
int LCD_8pin_init(int e, int rs, int rw, int db7, int db6, int db5, int db4,
    int db3, int db2, int db1, int db0) {
    
    lcd_e = e;
    lcd_rs = rs;
    lcd_rw = rw;
    lcd_db7 = db7;
    lcd_db6 = db6;
    lcd_db5 = db5;
    lcd_db4 = db4;
    lcd_db3 = db3;
    lcd_db2 = db2;
    lcd_db1 = db1;
    lcd_db0 = db0;
    
    LCD_activate(e);
    LCD_activate(rs);
    LCD_activate(rw);
    LCD_activate(db7);
    LCD_activate(db6);
    LCD_activate(db5);
    LCD_activate(db4);
    LCD_activate(db3);
    LCD_activate(db2);
    LCD_activate(db1);
    LCD_activate(db0);
    
    //Default init takes care of this.
    
    return 1;
}

int LCD_activate(int pin) {
    char port = LCD_io_port(pin);
    int shift = LCD_io_shift(pin);
    
    if(port == 'D') {
        DDRD |= 1 << shift;
    }
    else if (port == 'B') {
        DDRB |= 1 << shift;
    }
    else if (port == 'C') {
        DDRC |= 1 << shift;
    }
    
    return 1;
}

//Figures out which register needs to be set
char LCD_io_port(int pin) {
    if(pin <= 7 && pin >= 0) {
        return 'D';
    }
    else if (pin > 7 && pin <= 13) {
        return 'B';
    }
    else if (pin > 13) {
        return 'C';
    }
    else {
        return 'N';
    }
}

//Figures out how many shifts to set the correct bit
int LCD_io_shift(int pin) {
    char port = LCD_io_port(pin);
    if(port == 'D') {
        return pin;
    }
    else if(port == 'B') {
        return pin - 8;
    }
    else if(port == 'C') {
        return pin - 14;
    }
    else {
        return -1;
    }
}

//Sets/clears the appropriate register bit corresponding to the pin
int LCD_send(int pin, int bit_one) {
    char port = LCD_io_port(pin);
    int shift = LCD_io_shift(pin);
    
    int bit = bit_one ? 1 : 0;
    
    if(port == 'D') {
        if(bit) {
            PORTD |= 1 << shift;
        }
        else {
            PORTD &= ~(1 << shift);
        }
    }
    else if (port == 'B') {
        if(bit) {
            PORTB |= 1 << shift;
        }
        else {
            PORTB &= ~(1 << shift);
        }
    }
    else if (port == 'C') {
        if(bit) {
            PORTC |= 1 << shift;
        }
        else {
            PORTC &= ~(1 << shift);
        }
    }
    
    return 1;
}

//Clears data bits in the register.
int LCD_clear_data() {
    LCD_send(lcd_db7, 0);
    LCD_send(lcd_db6, 0);
    LCD_send(lcd_db5, 0);
    LCD_send(lcd_db4, 0);
    LCD_send(lcd_db3, 0);
    LCD_send(lcd_db2, 0);
    LCD_send(lcd_db1, 0);
    LCD_send(lcd_db0, 0);
    
    return 1;
}

//Sends an 8bit cmd to the LCD screen
int LCD_8_bit_cmd(char cmd) {
    LCD_send(lcd_e, 0);
    LCD_send(lcd_rs, 0);
    LCD_send(lcd_rw, 0);
    LCD_clear_data();   
    
    LCD_send(lcd_db7, cmd & 0x80);
    LCD_send(lcd_db6, cmd & 0x40);
    LCD_send(lcd_db5, cmd & 0x20);
    LCD_send(lcd_db4, cmd & 0x10);
    LCD_send(lcd_db3, cmd & 0x08);
    LCD_send(lcd_db2, cmd & 0x04);
    LCD_send(lcd_db1, cmd & 0x02);
    LCD_send(lcd_db0, cmd & 0x01);
    
    LCD_send(lcd_e, 1);
    _delay_us(1);
    LCD_send(lcd_e, 0);
    
    return 1;
}

//Sends an 4bit cmd to the LCD screen
int LCD_4_bit_cmd(char cmd) {
    //Clears all related bits
    LCD_send(lcd_e, 0);
    LCD_send(lcd_rs, 0);
    LCD_send(lcd_rw, 0);
    LCD_clear_data();
    
    //sets data bits for command part 1
    LCD_send(lcd_db7, cmd & 0x80);
    LCD_send(lcd_db6, cmd & 0x40);
    LCD_send(lcd_db5, cmd & 0x20);
    LCD_send(lcd_db4, cmd & 0x10);
    
    //Strobe the enable bit
    LCD_send(lcd_e, 1);
    _delay_us(1);
    LCD_send(lcd_e, 0);
    
    //Clears the data bits
    LCD_clear_data();
    
    //Sets data bits for command part 2
    LCD_send(lcd_db7, cmd & 0x08);
    LCD_send(lcd_db6, cmd & 0x04);
    LCD_send(lcd_db5, cmd & 0x02);
    LCD_send(lcd_db4, cmd & 0x01);
    
    //Strobe.
    LCD_send(lcd_e, 1);
    _delay_us(1);
    LCD_send(lcd_e, 0);
    
    return 1;
}

//Sends the cmd to the LCD screen
int LCD_send_cmd(char cmd) {
    int ret = 0;
    if(bit_mode == 8) {
        ret = LCD_8_bit_cmd(cmd);
    }
    else {
        ret = LCD_4_bit_cmd(cmd);
    }
    
    return ret;
}

int LCD_send_data(char data) {
    int ret = 0;
    if(bit_mode == 8) {
        ret = LCD_send_8data(data);
    }
    else {
        ret = LCD_send_4data(data);
    }
    
    return ret;
}

int LCD_send_8data(char data) {
    //Clears all related bits
    LCD_send(lcd_e, 0);
    LCD_send(lcd_rw, 0);
    LCD_clear_data();
    
    //Sets the rs bit
    LCD_send(lcd_rs, 1);
    
    //sets data bits for data part 1
    LCD_send(lcd_db7, data & 0x80);
    LCD_send(lcd_db6, data & 0x40);
    LCD_send(lcd_db5, data & 0x20);
    LCD_send(lcd_db4, data & 0x10);
    LCD_send(lcd_db3, data & 0x08);
    LCD_send(lcd_db2, data & 0x04);
    LCD_send(lcd_db1, data & 0x02);
    LCD_send(lcd_db0, data & 0x01);
    
    //Strobe the enable bit
    LCD_send(lcd_e, 1);
    _delay_us(1);
    LCD_send(lcd_e, 0);
    
    //Delay
    _delay_us(50);
    
    return 1;
}

int LCD_send_4data(char data) {
    //Clears all related bits
    LCD_send(lcd_e, 0);
    LCD_send(lcd_rw, 0);
    LCD_clear_data();
    
    //Sets the rs bit
    LCD_send(lcd_rs, 1);
    
    //sets data bits for data part 1
    LCD_send(lcd_db7, data & 0x80);
    LCD_send(lcd_db6, data & 0x40);
    LCD_send(lcd_db5, data & 0x20);
    LCD_send(lcd_db4, data & 0x10);
    
    //Strobe the enable bit
    LCD_send(lcd_e, 1);
    _delay_us(1);
    LCD_send(lcd_e, 0);
    
    //Delay
    _delay_us(50);
    
    //Clears the data bits & rs bit
    LCD_send(lcd_rs, 0);
    LCD_clear_data();
    
    //Reset RS bit
    LCD_send(lcd_rs, 1);
    
    //Sets data bits for data part 2
    LCD_send(lcd_db7, data & 0x08);
    LCD_send(lcd_db6, data & 0x04);
    LCD_send(lcd_db5, data & 0x02);
    LCD_send(lcd_db4, data & 0x01);
    
    //Strobe.
    LCD_send(lcd_e, 1);
    _delay_us(1);
    LCD_send(lcd_e, 0);
    
    //Delay
    _delay_us(50);
    
    return 1;
}

//Clear screen function
int LCD_cmd_clr() {
    int ret = LCD_send_cmd(LCD_CLR);
    _delay_us(4900);
    
    return ret;
}

//Return 'cursor' to home; start writing there.
int LCD_cmd_home() {
    int ret = LCD_send_cmd(LCD_HOME);
    _delay_us(1530);
    
    return ret;
}

//Sets the entry format; increment/decrement after write (increment 1 / 0), shift screen yes/no after write (shift 1/0)
int LCD_cmd_entry(int increment, int shift) {
    int ret = LCD_send_cmd(LCD_ENTRY | increment << 1 | shift);
    
    _delay_us(120);
    
    return ret;
}

//Sets display on or off (display 1 or 0), cursor visible or not (cursor 1 or 0), and cursor blink on or off (blink 1 or 0)
int LCD_cmd_display(int display, int cursor, int blink) {
    int ret = LCD_send_cmd(LCD_DISPLAY | display << 2 | cursor << 1 | blink);
    
    _delay_us(120);
    
    return ret;
}

//Cursor or display shift (shift 0 or 1), to the right or left (direction 1 or 0)
int LCD_cmd_shift(int shift, int direction) {
    int ret = LCD_send_cmd(LCD_SHIFT| shift << 3 | direction << 2);
    
    _delay_us(120);
    
    return ret;
}

//Function set; 8 bit or 4 bit (bit 1 or 0), line number 2 or 1 (line 1 or 0), font size large or small (font 1 or 0)
int LCD_cmd_function(int bit, int line, int font) {
    int ret = LCD_send_cmd(LCD_FUNCTION | bit << 4 | line << 3 | font << 2);
    bit_mode = bit ? 8 : 4;
    
    _delay_us(120);
    
    return ret;
}

int LCD_cmd_cgram(int addr) {
    int ret = LCD_send_cmd(LCD_CGRAM | addr);
    
    _delay_us(120);
    
    return ret;
}

int LCD_cmd_ddram(int addr) {
    int ret = LCD_send_cmd(LCD_DDRAM | addr);
    
    _delay_us(120);
    
    return ret;
}

int LCD_newLine() {
    return LCD_cmd_ddram(0x40);
}
