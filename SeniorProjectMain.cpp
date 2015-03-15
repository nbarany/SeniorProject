#include "LCDDriver.h"
#include "RotaryEncoder.h"
#include <avr/io.h>

//menu global variable flags
int volume = -40;
int is_pressed = 0;
int is_released = 0;
int pos = 0;
char buf[20];
//buf[0] = '\0';

int LCDscreen_init()
{
	//Wait for startup
	_delay_ms(50);
	LCD_4pin_init(12, 11, 10, 7, 6, 5, 4);
	
	//Functions set. 4 bit, 2 lines, display on
	LCD_cmd_function(0, 1, 1);
	
	//Display ON, Cursor off, Blink off
	LCD_cmd_display(1, 0, 0);
	
	//Display CLR
	LCD_cmd_clr();
	
	//Entry Mode (Shift off, increment)
	LCD_cmd_entry(1, 0);
	//Initialization END.
	return 0;
}

int lcd_write_str(char buf[]) //find length of string, send 1 char at a time for each index of str until it ends
{
	int i=0;
	int len;
	
	len = strlen(buf);
	for(i=0; i<len; i++)
	{
           LCD_send_data(buf[i]);
	}
	return 0;
}


RotaryEncoder encoder(A2, A3);
volatile int count = 0;
int menu[5] = {0,0,0,0,0};

void menu_update()
{
  switch(menu[1])
  {
    case 0:
       
       Serial.print(volume,DEC);
       Serial.print("\n");
       //adjust volume
       if(pos)   //if knob is turned right or left
      {
        volume += pos;
        pos = 0; 
        
        Serial.print(volume,DEC);
        Serial.print("\n");
        //temp = 10pow(volume/10)
        //safeWriteReg((pow(10.0,(double(volume)/10.0))),I2C_ADDR_ADAU1702w,SAFELOAD_DATA1, SAFELOAD_ADDR1, ****addressofVolume****);lcd
        //LCD_cmd_clr();
        //delay(100);
        sprintf(buf, "Volume: %d dB", volume);
        lcd_write_str(buf);
      }  
      else if(is_pressed == 1)
      {
         //one menu level deeper
         menu[0]++;   
         menu[1]++;
         is_pressed = 0;   
      }
    break;
    case 1:
       //Speaker Setup Branch
       if(is_released == 1)
       {
         //LCD_cmd_clr(); 
         //delay(100);
         sprintf(buf, "Speaker Setup");
          lcd_write_str(buf);
          is_released = 0;     
       }
       //Scrolling Criteria  
       if(pos > 0)  
       {
          menu[1] += pos;
          pos = 0;
          //LCD_cmd_clr();
          //delay(100);
          sprintf(buf, "Crossover");
          lcd_write_str(buf);
       }
       else if(pos < 0)
       {
          menu[1] = 4;
          pos = 0;
          //LCD_cmd_clr();
          //delay(100);
          sprintf(buf, "Speaker Prot");
          lcd_write_str(buf);
       }
       else if(is_pressed == 1)
       {
         menu[0]++;
         menu[2]++;
         is_pressed = 0;
       }      
    break;   
  }    
       
}

void setup() {
  // put your setup code here, to run once:
  //initialization
  LCDscreen_init();  // init LCD
  pinMode(2, INPUT_PULLUP);
  Serial.begin(57600);     //Enable Serial Transfer of Data
  sei();
  PCICR |= (1 << PCIE1);   // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
  PCICR |= (1 << PCIE2);   //This enables Pin Change Interrupt 2 that covers Port D.
  PCIFR |= (1 << PCIF2);   //Enables a flag in response to logic change to trigger ISR2. Flag cleared after completion of ISR. 
  PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);  // This enables the interrupt for pin 2 and 3 of Port C.
  PCMSK2 |= (1 << PCINT18);                   // This enables the interrupt for pin 2 of Port D.
}


ISR(PCINT1_vect) {
  noInterrupts();
  encoder.tick(); // just call tick() to check the state.
  menu_update();
  interrupts();
}

//Debouncing issue, ISR keeps getting called on logic change
ISR(PCINT2_vect) {               
   noInterrupts();
   if(digitalRead(2) == 0)   //active low button aka press down
   {
     is_pressed = 1;
     menu_update();
   }
   else                      // button release 
   {
     is_released = 1;
     menu_update();
   }
   interrupts();
}

void loop() {
  // put your main code here, to run repeatedly:
  

  int newPos = encoder.getPosition();
  if (newPos != pos) {
    //Serial.print(newPos);
    //Serial.println();
    pos = newPos;
    
  }
  
}
