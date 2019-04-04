//  to set date +T%s > /dev/ttyACM0

#include <Time.h>
#include <HC7Segment.h>
#include <TimedAction.h>
#include "virtualwire.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

typedef struct messagedata
{
  unsigned char RX_ID;
  int temperature;
  uint16_t  light;
    unsigned int battery;
  unsigned char button;
  
} messagedata;


bool cblank=false;


#define LM35Pin A7
#define WLED A3
const unsigned int MAX_INPUT = 50;

// Set pins:  CE, IO,CLK
//DS1302RTC RTC(17, 15, 2);
/* Include the 7 segment display library */

void blink()
 { 
    if (Serial.available () > 0)   
    processIncomingByte (Serial.read ()); 
   
 }



TimedAction timedAction = TimedAction(100,blink);
/* Pin order for digit select DIO */
const byte u8PinOut_Digit[] = {14,12,13,10};

/* Pin order for segment DIO. The segment order is A,B,C,D,E,F,G,DP */
const byte u8PinOut_Segment[] = {8,9,6,4,5,11,7,16};
/* Create an instance of HC7Segment(). In this example we will be using a 4 digit
common cathode display (CAI5461AH) */
HC7Segment HC7Segment(4, LOW);
//storage for temperature
float Temperature;
float cityTemperature=-1;
int decimal;


void beep(unsigned char delayms){
  analogWrite(3, 70);      // Almost any value can be used except 0 and 255
                           // experiment to get the best tone
  delay(delayms);          // wait for a delayms ms
  analogWrite(3, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms   
}


void setup()
{
   cli();    
   _delay_ms(2000);
    vw_set_rx_pin(2); //PD2 on pin2
    vw_setup(2000); //TRY 1000
    vw_rx_start();
    sei(); 
  pinMode(WLED, OUTPUT); //used for rf rx   
  //pinMode(19, OUTPUT);
  analogReference(INTERNAL);
  pinMode(3, OUTPUT); //Buzzer
  Serial.begin(9600);
  beep(50); 
  //sprintf( mbuffer,"%02d:%02d:%02d#%d.%01d#%u#%d",0,0,0,0,0,0,0);     
}

void loop()
{

  timedAction.check();
  char charBuf[20];    // for thermometer
  char decBuf[4];      // for thermometer
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  int k;
  Temperature = analogRead(LM35Pin) / 9.31;
  decimal=Temperature*100;
  decimal=decimal%100;
 
  //make string for serial
  itoa (Temperature,charBuf,10);
  strcat( charBuf, "." );
  itoa (decimal,decBuf,10);
  strcat( charBuf,decBuf);

  //check for rf message received
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
  messagedata *p = (messagedata *)buf;   
  char buffer[60];
  sprintf( buffer, "T:%4d.%01dC#ADC:%4u#ID:%d#BT:%d\r\n", (p->temperature) >> 4, (unsigned int)((p->temperature) << 12) / 6553,((p->light)>>6),(p->RX_ID),(p->button) );
  Serial.print(buffer);
  //sprintf( buffer,"CLK:%02d.%02d.%02d\r\n",hour(),minute(),second());
  //Serial.print(buffer);
  //setcounter
  
    for ( k=0; k<1800; k++)
    {
      HC7Segment.vDisplay_Number((p->temperature) >> 4,4);
      timedAction.check(); 
      delay(1); 
    }
  }

if(!cblank)
{
  
  for ( k=0; k<600; k++)
  {
    HC7Segment.vDisplay_Number(hour()*100+minute(),3);
    timedAction.check(); 
    delay(1); 
  }
  digitalWrite(19, LOW);
  
  for ( k=0; k<600; k++)
   {
    HC7Segment.vDisplay_Number( cityTemperature*10,2); 
    timedAction.check();
    delay(1);
   }
   
  for ( k=0; k<600; k++)
   {
    HC7Segment.vDisplay_Number( Temperature*10,2);
    timedAction.check();
    delay(1);
   } 
   Serial.println(charBuf);
}  
   
  //Serial.println(charBuf); //temperature 
  //Serial.println(mbuffer);
  
  //slow in blank mode
  if(cblank){
  timedAction.check();
  digitalWrite(WLED,HIGH);
  delay(1000);
  digitalWrite(WLED,LOW);
  
  }
  
  //hourly chime
  if((minute()==0) && (hour()>4) && second()<3)
  {
    beep(50);
    beep(50);
    beep(50);
    delay(700);
  }
  //every 30 minutes
  if( (minute()==30)&& (second()<2) && (hour()>4) )
  {
  beep(50); 
  delay(700); 
  }

}


void processIncomingByte (const byte inByte)
  {
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1407606566; // Aug 9 2014 
  
  switch (inByte)
    {
    
    case '\n':   // end of text
      input_line [input_pos] = 0;  // terminating null byte
      
      // terminator reached! process input_line here ...
      process_data (input_line);
      
      // reset buffer for next time
      input_pos = 0;  
      break;

    case '\r':   // discard carriage return
      break;
      
    case 'S':  // call sinu 
    input_pos = 0;
    for ( int k=0; k<10; k++)
     {
       beep(50);
       delay(50);
       beep(100);
       delay(50);
     } 
     break;

    case 'W':  // call sinu 
       cblank=true;
       break; 
    case 'N':  // cled on 
       digitalWrite(WLED,HIGH);
       break; 
    case 'F':  // call sinu 
       digitalWrite(WLED,LOW);
       break;               
    case 'X':  // call sinu 
       cblank=false;
       break;    
      
    case 'T':   // process Time update
      pctime = Serial.parseInt();
      if( pctime > DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      setTime(pctime);   // set the RTC and the system time to the received value
      input_pos = 0;
      beep(50);  
      }
      break;      

    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;
    }     
  } 
  
void process_data (const char * data)
  {
    cityTemperature=atof(data);
  }  



