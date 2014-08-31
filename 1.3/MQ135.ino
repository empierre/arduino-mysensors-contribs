/*
  Vera Arduino MQ135

  connect the sensor as follows :

  A H A   >>> 5V
  B		  >>> A0
  H       >>> GND
  B       >>> 10K ohm >>> GND
  
  Contribution: epierre
  
  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
*/

#include <Sleep_n0m1.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>  
#include <Sensor.h>  
#include <Wire.h> 

#define CHILD_ID_AIQ 0
#define AIQ_SENSOR_ANALOG_PIN 0
unsigned long SLEEP_TIME = 30; // Sleep time between reads (in seconds)
//VARIABLES
//float Ro = 10000.0;    // this has to be tuned 10K Ohm
float Ro = 0000.0;    // this has to be tuned 10K Ohm
int val = 0;           // variable to store the value coming from the sensor
float valAIQ =0.0;
float lastAIQ =0.0;

Sensor gw;
Sleep sleep;

void setup()  
{ 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ Sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID_AIQ, 22);  
    
}

// get CO ppm
float get_CO (float ratio){
	float ppm = 0.0;
	ppm = 37143 * pow (ratio, -3.178);
	return ppm;
}

void loop()      
{     
  uint16_t val = analogRead(AIQ_SENSOR_ANALOG_PIN);// Get AIQ value
  Serial.println(val);
  
  float Vrl = val * ( 5.00 / 1024.0  );      // V
  float Rs = 20000 * ( 5.00 - Vrl) / Vrl ;   // Ohm 
  int ratio =  Rs/Ro;                        
  
Serial.print ( "Vrl / Rs / ratio:");
Serial.print (Vrl);
Serial.print(" ");
Serial.print (Rs);
Serial.print(" ");
Serial.println(ratio);
Serial.print ( "CO ppm :");
Serial.println(get_CO(ratio));

  valAIQ=get_CO(ratio);
   
  if (valAIQ != lastAIQ) {
      gw.sendVariable(CHILD_ID_AIQ, V_VAR1, (int)ceil(valAIQ));
      lastAIQ = ceil(valAIQ);
  }
  
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(1000); //delay to allow serial to fully print before sleep
  gw.powerDown();
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(SLEEP_TIME * 1000); //sleep for: sleepTime 
}

