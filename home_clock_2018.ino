// DS1302_Serial_Easy (C)2010 Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
//
// Adopted for DS1302RTC library by Timur Maksimov 2014
//
// A quick demo of how to use my DS1302-library to 
// quickly send time and date information over a serial link
//
// I assume you know how to connect the DS1302.
// DS1302:  CE pin    -> Arduino Digital 27
//          I/O pin   -> Arduino Digital 29
//          SCLK pin  -> Arduino Digital 31
//          VCC pin   -> Arduino Digital 33
//          GND pin   -> Arduino Digital 35

//  to set date +T%s > /dev/ttyACM0

#include <Time.h>
//#include <DS1302RTC.h>
#include <HC7Segment.h>
#include <TimedAction.h>

#define LM35Pin A7
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
  pinMode(18, OUTPUT);    
  pinMode(19, OUTPUT);
  analogReference(INTERNAL);
  pinMode(3, OUTPUT);
  Serial.begin(9600);
  beep(50);     
}

void loop()
{
  timedAction.check();
  digitalWrite(18, HIGH);
  digitalWrite(19, HIGH);
  char buffer[80];     // the code uses 70 characters.
  char charBuf[20];    // for thermometer
  char decBuf[4];      // for thermometer
  int k;
  Temperature = analogRead(LM35Pin) / 9.31;
  decimal=Temperature*100;
  decimal=decimal%100;
 
  //make string for serial
  itoa (Temperature,charBuf,10);
  strcat( charBuf, "." );
  itoa (decimal,decBuf,10);
  strcat( charBuf,decBuf);

  
  for ( k=0; k<2000; k++)
  {
    HC7Segment.vDisplay_Number(hour()*100+minute(),3);
    timedAction.check();  
  }
  digitalWrite(19, LOW);
  
  for ( k=0; k<2000; k++)
   {
    HC7Segment.vDisplay_Number( cityTemperature*10,2); 
    timedAction.check();
   }
   
  for ( k=0; k<2000; k++)
   {
    HC7Segment.vDisplay_Number( Temperature*10,2);
    timedAction.check();
   }
   
  Serial.println(charBuf); //temperature
 
  //hourly chime
  if((minute()==0) && (hour()>4) && second()<4)
  {
    beep(50);
    beep(50);
    beep(50);
  }
  //every 30 minutes
  if( (minute()==30)&& (second()<4) && (hour()>4) )
  {
  beep(50);  
  }

  /*
  Serial.print("UNIX Time: ");
  Serial.print(RTC.get());

  if (! RTC.read(tm)) {
    Serial.print("  Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print(", DoW = ");
    Serial.print(tm.Wday);
    Serial.println();
  } else {
    Serial.println("DS1302 read error!  Please check the circuitry.");
    Serial.println();
    delay(9000);
  }
  */

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


