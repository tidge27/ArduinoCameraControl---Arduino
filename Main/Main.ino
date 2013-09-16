/*  This code is for the 2013 version of the 'BT Camera Contoller' built by Tom Garry, and Rory Crispin  */



#include <Wire.h>
#include <Time.h> 

//    Time Variable
time_t t;

//    Outputs
int powerSwitch = 6;       // the pin on the power switch
int IndicatorPin = 9;      // the pin that the indicator LED is attached to
int IRPin = 10;            // the pin that the IR LED is attached to
int FlashPin = 13;         // the pin that the flash optocoupler is attached to
int ValvePin = 12;         // the pin that the solenoid valve transistor is attached to
int ActivePin = 8;         // the pin that tells us when something is working away (e.g. a timelapse sequence)
int CableRelease = 7;      // Cable relese pin

//    Inputs
int TriggerPin = 9;        // the pin that the trigger push button is attached to
int LDRPin = 4;            // the analogue pin that the LDR circuit is attached to
int MicPin = 5;            // the analogue pin that the indicator LED is attached to

int var0;          // Catches the data from the BT module  This is basically the 'mode' for the system
int var1;          // Catches the data from the BT module
int var2;          // Catches the data from the BT module
int var3;          // Catches the data from the BT module
int var4;          // Catches the data from the BT module
int var5;          // Catches the data from the BT module
int var6;          // Catches the data from the BT module
int var7;          // Catches the data from the BT module
int var8;          // Catches the data from the BT module
int var9;          // Catches the data from the BT module

int micval;
int LDRval;


void setup() {

  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  
  
  setTime(1356998400);
  Serial.begin(9600);      // Start up the serial interface

  pinMode(powerSwitch, INPUT);
  pinMode(IndicatorPin, OUTPUT);  // Setup the IndicatorPin as an output
  pinMode(IRPin, OUTPUT);  // Setup the IRPin as an output
  pinMode(FlashPin, OUTPUT);  // Setup the IRPin as an output
  pinMode(ValvePin, OUTPUT);  // Setup the IRPin as an output
  pinMode(ActivePin, OUTPUT);  // Setup the IRPin as an output
  pinMode(CableRelease, OUTPUT); //Cable release
  
  digitalWrite(ActivePin, HIGH);// Indicate setup complete
  delay(200);
  digitalWrite(ActivePin, LOW);// Off again
  while(digitalRead(powerSwitch) == HIGH)  {

  }
  clockrst();
  calsensors();
}

void loop() {
  // see if there's incoming serial data:
  if(digitalRead(powerSwitch) == HIGH)  {
    clockrst();
    digitalWrite(ActivePin, HIGH); 
    while(digitalRead(powerSwitch) == HIGH)  {
      if((now()-t) >= 2)  {
        digitalWrite(5, LOW);
      }
    }
    digitalWrite(ActivePin, LOW); 
  }
  //Here we set upt a loop to continuously check if the device has been left on for more than 5 minuites.
  if((now()-t) >= 300)  {
    digitalWrite(5, LOW);
  }


  while (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    var0 = Serial.parseInt();
    var1 = Serial.parseInt();
    var2 = Serial.parseInt();
    var3 = Serial.parseInt();
    var4 = Serial.parseInt();
    var5 = Serial.parseInt();
    var6 = Serial.parseInt();
    var7 = Serial.parseInt();
    var8 = Serial.parseInt();
    var9 = Serial.parseInt();
    
    if(Serial.read() == '!')  {


      while(Serial.available())  {
        Serial.read();
      }

      digitalWrite(ActivePin, HIGH);
      /*
      Serial.print("Var0: ");    //Debugging stuff ;)
      Serial.println(var0);
      Serial.print("Var1: ");
      Serial.println(var1);
      Serial.print("Var2: ");
      Serial.println(var2);
      Serial.print("Var3: ");
      Serial.println(var3);
      Serial.print("Var4: ");
      Serial.println(var4);
      Serial.print("Var5: ");
      Serial.println(var5);
      Serial.print("Var6: ");
      Serial.println(var6);
      Serial.print("Var7: ");
      Serial.println(var7);
      Serial.print("Var8: ");
      Serial.println(var8);
      Serial.print("Var9: ");
      Serial.println(var9);
      Serial.println("All sent out!");
      */
      
      
      
      
      if(var0 == 1)  {    //Simple snapshot mode
        int delsecs = var1;
        int bulbdel = var2;
        int bulbmd = var3;
        clockrst();
        for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if enough time has passed
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
          } else  {
            if(bulbdel == 0){
                
            } else{ 
              bulbmode(bulbdel);  // Use the bulb function to keep the shutter open
          }
        }
      }
      
      
      
      
      
      
      
      if(var0 == 2)  {    //Simple timplapse mode
        int tlsecs = var1;
        int tlmins = var2;
        int tlhors = var3;
        int tlshot = var4;
        int infinite = var4;
        int accessmove = var5;
        
        if (infinite == 0)  {
          tlshot = 10;
        }
        tlsecs = tlsecs + 60*tlmins + 60*60*tlhors;
        shuttertrig();
        for (int i=2; i<= tlshot; i++)  {  //Start the timelapse loop  This part copunts up the number of shots taken

          if(infinite == 0)  {
            i = 2;
          }

          if((accessmove >= 1)) {
            if(tlsecs >= 2) {
              delay(700);
            } else  {
              delay(tlsecs * 400);
            }
          }
          if(accessmove >= 1) {
            digitalWrite(ValvePin, HIGH);
            delay(accessmove);
            digitalWrite(ValvePin, LOW);  
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
      
      
      
      
      
      if(var0 == 3)  {              //Simple shutter Mic trigger or LDR
        int MicSensitivity = var1;
        int LDRSensitivity = var2;
        int delaytime = var3;
        int persistant = var4;
        boolean bulbflash = var5;
        int normal = 0;
        while(persistant >= 1 || normal == 0){
          normal = 1;

          if(bulbflash) {
            bulbstartend();
          }

          for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if the mic is high or the LDR has changed
            if(analogRead(MicPin) > (micval + MicSensitivity) || analogRead(MicPin) < (micval - MicSensitivity) || analogRead(LDRPin) > (LDRval + LDRSensitivity) || analogRead(LDRPin) < (LDRval - LDRSensitivity))  {
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
      
      
      
      
      
      
      if(var0 == 4)  {    //SFlash trig mic LDR

        int MicSensitivity = var1;
        int LDRSensitivity = var2;
        int dripslengthvar = var3;
        int dripnumber = var4;
        int delaybetweendrips = var5;
        int flashdelay = var6;  // The delay from the flash after 
        
        shuttertrig();
        for(int var = 0; var < dripnumber;var++)  {
          delay(delaybetweendrips);
          driptrig(dripslengthvar);
        }
        for (int v=0; v<= 1;)  {          //Here we set upt a loop to continuously check if the mic is high or the LDR has changed
          if(analogRead(MicPin) > (micval + MicSensitivity) || analogRead(MicPin) < (micval - MicSensitivity) || analogRead(LDRPin) > (LDRval + LDRSensitivity) || analogRead(LDRPin) < (LDRval - LDRSensitivity))  {
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
      
      
      
      
      
      if(var0 == 5)  {    //HDR timelapse mode
        int tlsecs = var1;
        int tlmins = var2;
        int tlhors = var3;
        int tlshot = var4;
        int s1 = var5;
        int s2 = var6;
        int s3 = var7;
        int infinite = var4;
        int accessmove = var8;
        
        if (infinite == 0)  {
          tlshot = 10;
        }
        tlsecs = tlsecs + 60*tlmins + 60*60*tlhors;
        hdrphoto(s1, s2, s3);
        for (int i=2; i<= tlshot; i++)  {  //Start the timelapse loop  This part copunts up the number of shots taken

          if(infinite == 0)  {
            i = 2;
          }

          if((accessmove >= 1)) {
            if(tlsecs >= 2) {
              delay(700);
            } else  {
              delay(tlsecs * 400);
            }
          }
          if(accessmove >= 1) {
            digitalWrite(ValvePin, HIGH);
            delay(accessmove);
            digitalWrite(ValvePin, LOW);  
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

      
      
      
      
      
      
      if(var0 == 9)  {    // This is used to zero the value for both the mic and the LDR channels
        if(var1 == 99)  {
          digitalWrite(5, LOW);
        }
        calsensors();
      }
      
      
      
      
      digitalWrite(ActivePin, LOW);  
    }
    clockrst();  
  }
}

//Start of trigger code
void shuttertrig()  {

  digitalWrite(IndicatorPin, HIGH); 
  digitalWrite(CableRelease, HIGH);   //Get the indicator on for a little visual reference
  //Actual IR LED code goes here
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
    digitalWrite(ValvePin, HIGH);    //Trigger the solenoid valve
    delay(delayvar);                          //A little delay
    digitalWrite(ValvePin, LOW);     //Turn it off again
  }
}  

void clockrst()  {  //Used to set the time variable t to the current time, allowing it to be subtracted later on.
  t = now();
}

void calsensors()  {
  micval = analogRead(MicPin);
  LDRval = analogRead(LDRPin);
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
    delay(del);
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
    //End of IR LED code 
    digitalWrite(IndicatorPin, LOW);
    digitalWrite(CableRelease, LOW);  //Turn the LED off again
  
  
  }  else  {

    del = del/1000;  
    
    digitalWrite(IndicatorPin, HIGH); 
    digitalWrite(CableRelease, HIGH);   //Get the indicator on for a little visual reference
      //Actual IR LED code goes here
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
  //End of IR LED code :)
    digitalWrite(IndicatorPin, LOW);
    digitalWrite(CableRelease, LOW);  //Turn the LED off again   
  }
}
