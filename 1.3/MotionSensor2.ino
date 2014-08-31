#include <Sleep_n0m1.h>
#include <SPI.h>
#include <EEPROM.h>  
#include <RF24.h>
#include <Sensor.h>  
//  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)


#define DIGITAL_INPUT_SENSOR 3   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define CHILD_ID 0   // Id of the sensor child
unsigned long SEND_FREQUENCY = 2;              // Minimum time between send (in seconds). We don't want to spam the gateway.
long lastDebounce = 0;
long debounceDelay = 500;
unsigned long lastSend;
unsigned long currentTime;
unsigned int isTripped =0;

Sensor gw;
Sleep sleep;

void setup()  
{  
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Motion Sensor", "1.0");

  pinMode(DIGITAL_INPUT_SENSOR, INPUT);      // sets the motion sensor digital pin as input
  
  // Register all sensors to gw (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID, S_MOTION);
  
  lastSend = -1;
}

void loop()     
{     
  currentTime = millis();
  
  // Read digital motion value
  boolean tripped = digitalRead(DIGITAL_INPUT_SENSOR) == HIGH; 
        
  Serial.println(tripped);
  Serial.println(currentTime-lastSend);
  if ((isTripped==0) &&(tripped==1)) {
    Serial.println("tripped");
    gw.sendVariable(CHILD_ID, V_TRIPPED, tripped?"1":"0");  // Send tripped value to gw 
    lastSend=currentTime;
    isTripped=1;
  }

  if ((tripped==0) && (isTripped==1) && (currentTime - lastSend > 500*SEND_FREQUENCY)){
    Serial.println("un-tripped");
    gw.sendVariable(CHILD_ID, V_TRIPPED, tripped?"1":"0");  // Send tripped value to gw 
    lastSend=currentTime;
    isTripped=0;
  }

  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(2000); //delay to allow serial to fully print before sleep
  gw.powerDown();
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepInterrupt(INTERRUPT,CHANGE);
}


