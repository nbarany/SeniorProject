//VERSION 1.0.0
//REVISION DAY: 3/18/2015
//CURRENT PROBLEMS: DOUBLE BACK FUNCTION ONLY GOING UP ONE TIME INSTEAD OF TWO TIMES
//                  ALSO ONLY BI-AMP BRANCH IS FULLY UPDATED CORRECTLY
#include "LCDDriver.h"
#include "RotaryEncoder.h"
#include <avr/io.h>

//menu global variable flags
int volume = -40;
int is_pressed = 0;
int is_released = 0;
int turn_dir = 0;
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

void back()
{
    //go up a level
    is_pressed = 0;
    menu[menu[0]] = 0;    
    menu[0]--;
    
    Serial.print(menu[0]);
  Serial.print(menu[1]);
  Serial.print(menu[2]);
  Serial.print(menu[3]);
  Serial.println(menu[4]);
        
}
void dubback()
{
    //go up 2 levels (for confirm levels)
    menu[menu[0]] = 0;
    menu[0]--;
    menu[menu[0]] = 0;
    menu[0]--;
     Serial.print(menu[0]);
  Serial.print(menu[1]);
  Serial.print(menu[2]);
  Serial.print(menu[3]);
  Serial.println(menu[4]);
}

void menu_update()
{
  Serial.print(F("turn_dir: "));
  Serial.println(turn_dir);
  Serial.print(F("is_pressed: "));
  Serial.println(is_pressed);
  Serial.print(F("is_released: "));
  Serial.println(is_released);
  switch(menu[1])
  {
    case 0:
       if(turn_dir)   //if knob is turned right or left
       {
          volume += turn_dir;     
          Serial.print(F("Volume:"));
          Serial.println(volume);
          //temp = 10pow(volume/10)
          //safeWriteReg((pow(10.0,(double(volume)/10.0))),I2C_ADDR_ADAU1702w,SAFELOAD_DATA1, SAFELOAD_ADDR1, ****addressofVolume****);lcd
          sprintf(buf, "Volume: %d dB", volume);
       }  
       else if(is_pressed == 1)
       {
         //one menu level deeper
         menu[0]++;   
         menu[1]++;
         is_pressed = 0;   
       }
    break; //break menu[1]
    
    case 1:
      
         switch(menu[2])
         {
             case 0:
             //Speaker Setup Branch
                 if(is_released == 1)
                 {
                    sprintf(buf, "Speaker Setup");
                    is_released = 0;     
                 }
                 //Scrolling Criteria  
                 else if(turn_dir == 1)  
                 {
                    menu[1] += turn_dir;
                    sprintf(buf, "Crossover");
                 }
                 else if(turn_dir == -1)
                 {
                    menu[1] = 4;   //Wrap around left side of menu
                    sprintf(buf, "Speaker Prot");
                 }
                 else if(is_pressed == 1)
                 {
                   menu[0]++;
                   menu[2]++;
                   is_pressed = 0;
                   sprintf(buf, "Bi-Amp");
                 }        
	     break;
               
             case 1:
             //Bi-Amp
             
                 switch(menu[3])
                 {
                     case 0:
                     //Serial.println(turn_dir);
                     //Bi-amp Mode [2,1,1,0,0]
                     if(is_released == 1)
                     {
                       sprintf(buf, "Bi-Amp");
                       is_released = 0;
                     }
                     else if(turn_dir == 1)  
                     {
                        menu[2] += turn_dir;
                        sprintf(buf, "Stereo + Sub");
                     }
                     else if(turn_dir == -1)
                     {
                        menu[2] = 3;   //Wrap around left side of menu
                        sprintf(buf, "Back");
                     }
                     else if(is_pressed == 1)
                     {
                       menu[0]++;
                       menu[3]++;
                       is_pressed = 0;
                       sprintf(buf, "Confirm: Y?");
                     }
                     break;

                     //CONFIRM BI-AMP MODE
                     case 1:
                     if(is_released == 1)
                     {
                       sprintf(buf, "Confirm: Y?");
                       is_released = 0;
                     }
                     else if(turn_dir == 1 || turn_dir == -1)  
                     {
                        menu[3] = 2;
                        sprintf(buf, "Confirm: N?");
                     }
                     else if(is_pressed == 1)
                     {
                       //Set Bi-Amp Mode
                       //NOSUB_download();
                     }
                     break;
                     
                     //CONFIRM BI-AMP BACK
                     case 2:
                     if(turn_dir == 1 || turn_dir == -1)  
                     {
                        menu[3] = 1;
                        sprintf(buf, "Confirm: Y?");
                     }
                     else if(is_pressed == 1)
                     {
                       //Return to Speaker Setup
                       is_pressed == 0;
                       sprintf(buf, "Speaker Setup");
                       dubback();                       
                     }
                     break;
                                          
                 }
             break;
              
             case 2:
             //Stereo+Sub [2,1,2,0,0]   
                 switch(menu[3])
                     {
                         case 0:
                         //Stereo+Sub 
                         if(turn_dir == 1)  
                         {
                            menu[2] += turn_dir;
                            sprintf(buf, "Back");
                         }
                         else if(turn_dir == -1)
                         {
                            menu[2] += turn_dir;   //Wrap around left side of menu
                            sprintf(buf, "Bi-Amp");
                         }
                         else if(is_pressed == 1)
                         {
                            menu[0]++;
                            menu[3]++;
                            is_pressed = 0;
                         }
                         break;
    
                         //CONFIRM STEREO-SUB MODE
                         case 1:
                         if(is_released == 1)
                         {
                            sprintf(buf, "Confirm: Y?");
                            is_released = 0;
                         }
                         else if(turn_dir == 1 || turn_dir == -1)  
                         {
                            menu[3] = 2;
                            sprintf(buf, "Confirm: N?");
                         }
                         else if(is_pressed == 1)
                         {
                            //Set Stereo-Sub Mode
                            //SUB_download();
                         }
                         break;
                       
                         //CONFIRM Stereo-sub BACK
                         case 2:
                         if(turn_dir == 1 || turn_dir == -1)  
                         {
                            menu[3] = 1;
                            sprintf(buf, "Confirm: Y?");
                         }
                         else if(is_pressed == 1)
                         {
                         //Return to Speaker Setup
                         is_pressed == 0;
                         back();                         
                         }
                         break;							 
                    } // End of Stereo-Sub 
                 break;
            
                 case 3:
                 //Back [2,1,3,0,0]   
					 //Back 
					 if(turn_dir == 1)  
					 {
						menu[2] = 1;
						sprintf(buf, "Bi-Amp");
					 }
					 else if(turn_dir == -1)
					 {
						menu[2] += turn_dir;   //Wrap around left side of menu
						sprintf(buf, "Stereo+Sub");
					 }
					 else if(is_pressed == 1)
					 {
					   //Back function
                                           is_pressed = 0;
					   back();					   
					 }
                         
                 break;
                  
         } 
    break;
   
}
  
  LCD_cmd_clr();  
  delay(100);
  lcd_write_str(buf);
  Serial.print(F("Menu: "));
  Serial.print(menu[0]);
  Serial.print(menu[1]);
  Serial.print(menu[2]);
  Serial.print(menu[3]);
  Serial.println(menu[4]);
  Serial.println(buf);
  Serial.println("");  
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
  interrupts();
}

//Debouncing issue, ISR keeps getting called on logic change
ISR(PCINT2_vect) {               
   noInterrupts();
   if(digitalRead(2) == 0)   //active low button aka press down
   {
     is_pressed = 1;  
   }
   else                      // button release 
   {
     is_released = 1;
   }
   interrupts();
}

void loop() {
  
  
  int newPos = encoder.getPosition();
  if (newPos != pos) {
    //Serial.print(newPos);
    //Serial.println();
    turn_dir = newPos - pos;
    pos = newPos;
  }
  
  //Variable updates depending on whether turn right/left or button 
  if(turn_dir || is_pressed || is_released) //if any menu change interrupt occurs
  {
    menu_update();  //change menu
    turn_dir = 0;   //reset position vector
  }  
}
