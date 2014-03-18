
/*  This code is for the 2013 version of the 'BT Camera Contoller' built by Tom Garry, and Rory Crispin  */



#include <Wire.h>
#include <Time.h>
#include "Servo.h"
#include <EEPROM.h>

// servo stuff
Servo myservox;
Servo myservoy;


//    Time Variable
time_t t;

//    Camera manufacturer

//The following is the list of variables for inputs and outputs...


int cameraType = EEPROM.read(0);   //Please refer to the documentation to find out which camera this value refers to...

//    Outputs
int powerSwitch    = 6;          // the pin on the power switch to check status
int IndicatorPin   = 9;          // the pin that the indicator LED is attached to
int IRPin          = 10;         // the pin that the IR LED is attached to
int FlashPin       = 13;         // the pin that the flash optocoupler is attached to
int PwroutPin      = 12;         // the pin that the power ouput transistor is attached to
int ActivePin      = 8;          // the pin that tells us when something is working away (e.g. a timelapse sequence)
int CableRelease   = 7;          // Cable relese pin

//    Inputs
int TriggerPin = 9;        // the pin that the trigger push button is attached to
int LDRPin[] = {
  16,19};            // the analogue pin that the LDR circuit is attached to
int MicPin[] = {
  17,18};            // the analogue pin that the indicator LED is attached to

long int vars[50];

int micval;
int LDRval;


void setup() {

  myservox.attach(3);
  myservoy.attach(11);

  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);


  setTime(1356998400);
  Serial.begin(9600);      // Start up the serial interface

  pinMode(powerSwitch, INPUT);
  pinMode(IndicatorPin, OUTPUT);  // Setup the IndicatorPin as an output
  pinMode(IRPin, OUTPUT);  // Setup the IRPin as an output
  pinMode(FlashPin, OUTPUT);  // Setup the IRPin as an output
  pinMode(PwroutPin, OUTPUT);  // Setup the IRPin as an output
  pinMode(ActivePin, OUTPUT);  // Setup the IRPin as an output
  pinMode(CableRelease, OUTPUT); //Cable release

  digitalWrite(ActivePin, HIGH);// Indicate setup complete
  delay(200);
  digitalWrite(ActivePin, LOW);// Off again
  while(digitalRead(powerSwitch) == LOW)  {

  }
  clockrst();
  calsensors(0);
  calsensors(1);
}

void loop() {
  // see if there's incoming serial data:
  if(digitalRead(powerSwitch) == LOW)  {
    clockrst();
    digitalWrite(ActivePin, HIGH); 
    while(digitalRead(powerSwitch) == LOW)  {
      if((now()-t) >= 2)  {
        digitalWrite(ActivePin, LOW); 
        digitalWrite(5, LOW);
      }
    }
    digitalWrite(ActivePin, LOW); 
  }
  //Here we set upt a loop to continuously check if the device has been left on for more than 5 minuites.
  if((now()-t) >= 300)  {
    digitalWrite(5, LOW);
  }


  if(Serial.available() > 0)  {
    for(int i=0;i<50;i++){
      vars[i] = 0;
    }
    int a = 50;
    for(int i=0;i<a;i++)  {
      vars[i] = Serial.parseInt();
      if(Serial.read() == '!')  {
        a=i;
      }
    }

    digitalWrite(ActivePin, HIGH);
    /*
      Serial.print("vars[0]: ");    //Debugging stuff ;)
     Serial.println(vars[0]);
     Serial.print("vars[1]: ");
     Serial.println(vars[1]);
     Serial.print("vars[2]: ");
     Serial.println(vars[2]);
     Serial.print("vars[3]: ");
     Serial.println(vars[3]);
     Serial.print("vars[4]: ");
     Serial.println(vars[4]);
     Serial.print("vars[5]: ");
     Serial.println(vars[5]);
     Serial.print("vars[6]: ");
     Serial.println(vars[6]);
     Serial.print("vars[7]: ");
     Serial.println(vars[7]);
     Serial.print("vars[8]: ");
     Serial.println(vars[8]);
     Serial.print("vars[9]: ");
     Serial.println(vars[9]);
     Serial.println("All sent out!");
     */




    if(vars[0] == 1)  {    //Simple snapshot mode
      int delsecs = vars[1];
      int bulbdel = vars[2];
      int bulbmd = vars[3];
      clockrst();
      for (int v=0; v<= 1;)  {          //Here we set up a loop to continuously check if enough time has passed
        if((now()-t) >= delsecs)  {
          v = 5;
        }
        if (Serial.available() > 0) {   // Gives us a chance to quit the loop if the user wants to terminate the operation
          while(Serial.available())  {
            Serial.read();
          }
          v = 5;
        }
      }
      if(bulbmd == 0)  {    
        shuttertrig(); // Take a normal photo.
      } 
      else  {
        if(bulbdel == 0){

        } 
        else{ 
          bulbmode(bulbdel);  // Use the bulb function to keep the shutter open
        }
      }
    }







    if(vars[0] == 2)  {    //Simple timplapse mode
    
    //            ***** Standard Timelapse vars *****
    
      unsigned long tlsecs = vars[1]; //Delay Seconds
      int tlmins = vars[2];           //Delay Delay Mins
      int tlhors = vars[3];           //Delay Hours
      int tlshot = vars[4];           //Number of shots  - 0 for infinite
      int infinite = vars[4];         // Tom is an idiot  - duplicate int
      
     //            ***** Dolly Timelapse var *****
      int accessmove = vars[5];       //Delay for output drive time 
      
     //            ***** Ramping Timelapse vars  linear *****
      int rampstart = vars[6];        //Exposure time for bulb ramping start
      int rampend = vars[7];          //Expusure time for bulm barming end 
      int rampdiff = round(10*(rampend - rampstart) / tlshot);

      if (infinite == 0)  {
        tlshot = 10;
      }
      tlsecs = tlsecs + 60*tlmins + 60*60*tlhors;
      if(rampstart == 0 && rampend == 0)  {
        shuttertrig();
      }  
      else  {
        bulbmode(rampstart);
      }
      for (int i=2; i<= tlshot; i++)  {  //Start the timelapse loop  This part copunts up the number of shots taken

        if(infinite == 0)  {
          i = 2;
        }

        if((accessmove >= 1)) {
          if(tlsecs >= 2) {
            delay(700);
          } 
          else  {
            delay(tlsecs * 400);
          }
        }
        if(accessmove >= 1) {
          digitalWrite(PwroutPin, HIGH);
          delay(accessmove);
          digitalWrite(PwroutPin, LOW);  
        }


        clockrst();
        for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if enough time has passed





          if((now()-t) >= tlsecs)  {
            v = 5;
          }
          if (Serial.available() > 0) {   // Gives us a chance to quit the loop if the user wants to terminate the operation
            while(Serial.available())  {
              Serial.read();
            }
            v = 5;
            i = tlshot + 10;
          }
        }
        if(rampstart == 0 && rampend == 0)  {
          shuttertrig();
        }  
        else  {
          Serial.println(round((rampstart*10 + (rampdiff * i))/10));
          bulbmode(round((rampstart*10 + (rampdiff * i))/10));
        }
      }

    }





    if(vars[0] == 3)  {              //Simple shutter Mic trigger or LDR
      int MicSensitivity = vars[1];  //Microphopne sensitivity
      int LDRSensitivity = vars[2];  //LDR sensitivity
      int delaytime = vars[3];       //Delay after trigger (milsec)
      int persistant = vars[4];      //Persistant mode - repeat indefinitely
      boolean bulbflash = vars[5];   //Bulb flash mode for ultra high speed
      int extorint = vars[6];        //External or internal triggers
      int normal = 0;
      while(persistant >= 1 || normal == 0){
        normal = 1;

        if(bulbflash) {
          bulbstartend();
        }

        for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if the mic is high or the LDR has changed
          if(analogRead(MicPin[extorint]) > (micval + MicSensitivity) || analogRead(MicPin[extorint]) < (micval - MicSensitivity) || analogRead(LDRPin[extorint]) > (LDRval + LDRSensitivity) || analogRead(LDRPin[extorint]) < (LDRval - LDRSensitivity))  {
            delay(delaytime);
            flashtrig();
            if(!bulbflash) {
              shuttertrig();
            }
            v = 5;
          }
          if (Serial.available() > 0) {   // Gives us a chance to quit the loop if the user wants to terminate the operation
            while(Serial.available())  {
              Serial.read();
            }
            v = 5;
            persistant = 0;
          }
        }

        if(persistant >= 1)   {
          delay(1000);
        }

        if(bulbflash) {
          bulbstartend();
        }
      }
    }






    if(vars[0] == 4)  {    //SFlash trig mic LDR

      int MicSensitivity = vars[1];
      int LDRSensitivity = vars[2];
      int dripslengthvar = vars[3];
      int dripnumber = vars[4];
      int delaybetweendrips = vars[5];
      int flashdelay = vars[6];  // The delay from the flash after 
      int extorint = vars[7];        //External or internal triggers
      shuttertrig();
      for(int var = 0; var < dripnumber;var++)  {
        delay(delaybetweendrips);
        driptrig(dripslengthvar);
      }
      for (int v=0; v<= 1;)  {          //Here we set up a loop to continuously check if the mic is high or the LDR has changed
        if(analogRead(MicPin[extorint]) > (micval + MicSensitivity) || analogRead(MicPin[extorint]) < (micval - MicSensitivity) || analogRead(LDRPin[extorint]) > (LDRval + LDRSensitivity) || analogRead(LDRPin[extorint]) < (LDRval - LDRSensitivity))  {
          v = 5;
        }
        if (Serial.available() > 0) {   // Gives us a chance to quit the loop if the user wants to terminate the operation
          while(Serial.available())  {
            Serial.read();
          }
          v = 5;
        }
      }

      if(flashdelay > 0)  {
        delay(flashdelay);                          //A little delay
      }

      flashtrig();
      shuttertrig();
    }





    if(vars[0] == 5)  {    //HDR timelapse mode
      int tlsecs = vars[1];
      int tlmins = vars[2];
      int tlhors = vars[3];
      int tlshot = vars[4];
      int s1 = vars[5];
      int s2 = vars[6];
      int s3 = vars[7];
      int infinite = vars[4];
      int accessmove = vars[8];

      if (infinite == 0)  {
        tlshot = 10;
      }
      tlsecs = tlsecs + 60*tlmins + 60*60*tlhors;
      hdrphoto(s1, s2, s3);
      for (int i=2; i<= tlshot; i++)  {  //Start the timelapse loop  This part counts up the number of shots taken

        if(infinite == 0)  {
          i = 2;
        }

        if(accessmove >= 1) {
          if(tlsecs >= 2) {
            delay(700);
          } 
          else  {
            delay(tlsecs * 400);
          }
        }
        if(accessmove >= 1) {
          digitalWrite(PwroutPin, HIGH);
          delay(accessmove);
          digitalWrite(PwroutPin, LOW);  
        }


        clockrst();
        for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if enough time has passed
          if((now()-t) >= tlsecs)  {
            v = 5;
          }
          if (Serial.available() > 0) {   // Gives us a chance to quit the loop if the user wants to terminate the operation
            while(Serial.available())  {
              Serial.read();
            }
            v = 5;
            i = tlshot + 10;
          }
        }
        hdrphoto(s1, s2, s3);
      }

    }





    if(vars[0] == 6)  {    //Servo timelapse mode
      int sx = vars[1];
      int sy = vars[2];
      int ex = vars[3];
      int ey = vars[4];
      int tlsecs = vars[5];
      int tlmins = 0;
      int tlhors = 0;
      int tlshot = 180;
      int infinite = 180;


      if (infinite == 0)  {
        tlshot = 10;
      }
      tlsecs = tlsecs + 60*tlmins + 60*60*tlhors;
      shuttertrig();
      for (int i=2; i<= tlshot; i++)  {  //Start the timelapse loop  This part copunts up the number of shots taken

        if(infinite == 0)  {
          i = 2;
        }


        if(tlsecs >= 2) {
          delay(700);
        } 
        else  {
          delay(tlsecs * 400);
        }

        if((ex-sx)>0) {
          myservox.write(((ex-sx)/180)*tlshot);
          Serial.println(ex);
          Serial.println(sx);
          Serial.println(ex-sx);
          Serial.println(i);
          Serial.println(((ex-sx)/180)*i);
        } 
        else{
          myservox.write(180-((sx-ex)/180)*i);
          Serial.println(180-((sx-ex)/180)*i);
        }
        if((ey-sy)>0) {
          myservoy.write(((ey-sy)/180)*i);
        } 
        else{
          myservoy.write(180-((sy-ey)/180)*i);
        }




        clockrst();
        for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if enough time has passed





          if((now()-t) >= tlsecs)  {
            v = 5;
          }
          if (Serial.available() > 0) {   // Gives us a chance to quit the loop if the user wants to terminate the operation
            while(Serial.available())  {
              Serial.read();
            }
            v = 5;
            i = tlshot + 10;
          }
        }
        shuttertrig();
      }

    }







    if(vars[0] == 9)  {    // This is used to zero the value for both the mic and the LDR channels

      if(vars[1] == 99)  {    //9,99!  Initiates power off
        digitalWrite(5, LOW);
      }

      if(vars[1] == 1)  {    //9,1!Recalibrate the sensors
        calsensors(0);
        calsensors(1);
      }

      if(vars[1] == 2)  {   //0,2,X!  Set camera type
        EEPROM.write(0, vars[2]);
        cameraType = vars[2];
      }

    }




    digitalWrite(ActivePin, LOW);  
  }

}


//Start of trigger code
void pulseIR()  {
  Serial.println("photo");
  if(cameraType == 1)  {
    Serial.println("cannon");
    for (int i=0; i < 16; i++) {
      digitalWrite(IRPin, HIGH);                       // turn IR on
      pulseOFF(5);                              // half the clock cycle for 38Khz (26.32×10-6s) - e.g. the 'on' part of our wave
      digitalWrite(IRPin, LOW);                                       // pulse for 2000 uS (Microseconds)
      pulseOFF(5);                                    // turn pulse off for 27850 us
    }         
    pulseOFF(7330);
    for (int i=0; i < 16; i++) {
      digitalWrite(IRPin, HIGH);                       // turn IR on
      pulseOFF(5);                              // half the clock cycle for 38Khz (26.32×10-6s) - e.g. the 'on' part of our wave
      digitalWrite(IRPin, LOW);                                       // pulse for 2000 uS (Microseconds)
      pulseOFF(5);                                    // turn pulse off for 27850 us
    }
  }
  else{
    Serial.println("nikkon");
    for (int i=0; i < 2; i++) {

      pulseON(2000);                                      // pulse for 2000 uS (Microseconds)
      pulseOFF(27850);                                    // turn pulse off for 27850 us
      pulseON(390);                                       // and so on
      pulseOFF(1580);
      pulseON(410);
      pulseOFF(3580);
      pulseON(400);
      pulseOFF(63200);
    }        
  }
}
void shuttertrig()  {

  digitalWrite(IndicatorPin, HIGH); 
  digitalWrite(CableRelease, HIGH);   //Get the indicator on for a little visual reference
  //Actual IR LED code goes here
  pulseIR();                      
  //End of IR LED code :)
  delay(100);                          //A little delay (so we can see the LED blinking)
  digitalWrite(IndicatorPin, LOW);
  digitalWrite(CableRelease, LOW);  //Turn the LED off again
}
//*** More IR LED code
void pulseOFF(unsigned long startDelay) {
  unsigned long endDelay = micros() + startDelay;       // create the microseconds to delay for
  while(micros() < endDelay);
}

//*** Even more IR LED code
void pulseON(int pulseTime) {
  unsigned long endPulse = micros() + pulseTime;        // create the microseconds to pulse for
  while( micros() < endPulse) {
    digitalWrite(IRPin, HIGH);                       // turn IR on
    delayMicroseconds(13);                              // half the clock cycle for 38Khz (26.32×10-6s) - e.g. the 'on' part of our wave
    digitalWrite(IRPin, LOW);                        // turn IR off
    delayMicroseconds(13);                              // delay for the other half of the cycle to generate wave/ oscillation
  }
}
//End of Trigger code


void flashtrig()  {
  digitalWrite(FlashPin, HIGH);    //Trigger the flash via optocoupler
  delay(100);                          //A little delay so we can see the LED blinking)
  digitalWrite(FlashPin, LOW);     //Remove the connection again
}

void driptrig(int delayvar)  {
  if(delayvar > 0)  {
    digitalWrite(PwroutPin, HIGH);    //Trigger the solenoid valve
    delay(delayvar);                          //A little delay
    digitalWrite(PwroutPin, LOW);     //Turn it off again
  }
}  

void clockrst()  {  //Used to set the time variable t to the current time, allowing it to be subtracted later on.
  t = now();
}

void calsensors(int intorext)  {
  micval = analogRead(MicPin[intorext]);
  LDRval = analogRead(LDRPin[intorext]);
}

void hdrphoto(int s1, int s2, int s3)  {
  bulbmode(s1);
  delay(350);
  bulbmode(s2);
  delay(350);
  bulbmode(s3);
  delay(350);
}



void bulbstartend() {
  int var = 0;
  if(digitalRead(CableRelease) == LOW)  {
    var = 1;
  }
  if(var == 1)  {
    digitalWrite(CableRelease, HIGH);   //Get the indicator on for a little visual reference
  }
  digitalWrite(IndicatorPin, HIGH); 
  //Actual IR LED code goes here
  pulseIR();                 
  //End of IR LED code 
  if(var != 1)  {
    digitalWrite(CableRelease, LOW);   //Get the indicator on for a little visual reference
  }
  delay(100);
  digitalWrite(IndicatorPin, LOW);
}



void bulbmode(int del)  {
  if (del <= 3000)  {


    digitalWrite(IndicatorPin, HIGH); 
    digitalWrite(CableRelease, HIGH);   //Get the indicator on for a little visual reference
    //Actual IR LED code goes here
    pulseIR();
    delay(del);
    pulseIR();                    
    //End of IR LED code 
    digitalWrite(IndicatorPin, LOW);
    digitalWrite(CableRelease, LOW);  //Turn the LED off again


  }  
  else  {

    del = del/1000;  

    digitalWrite(IndicatorPin, HIGH); 
    digitalWrite(CableRelease, HIGH);   //Get the indicator on for a little visual reference
    //Actual IR LED code goes here
    pulseIR();
    clockrst();
    delay(100);
    digitalWrite(IndicatorPin, LOW); 
    for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if enough time has passed
      if((now()-t) >= del)  {
        v = 5;
      }
    }
    digitalWrite(IndicatorPin, HIGH);
    delay(100);
    pulseIR();                      
    //End of IR LED code :)
    digitalWrite(IndicatorPin, LOW);
    digitalWrite(CableRelease, LOW);  //Turn the LED off again   
  }
}




