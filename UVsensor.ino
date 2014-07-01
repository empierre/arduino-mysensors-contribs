/*
  Vera Arduino UVM-30A

  connect the sensor as follows :

  +   >>> 5V
  -   >>> GND
  out >>> A0     
  
  Contribution: epierre, bulldoglowell

  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
*/

#include <Sleep_n0m1.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>  
#include <Sensor.h>  
#include <Wire.h> 

#define CHILD_ID_UV 0
#define UV_SENSOR_ANALOG_PIN 0
unsigned long SLEEP_TIME = 30; // Sleep time between reads (in seconds)

Sensor gw;
Sleep sleep;
int lastUV = -1;
int uvIndexValue [12] = { 50, 227, 318, 408, 503, 606, 696, 795, 881, 976, 1079, 1170, 3000};
int uvIndex;

void setup()  
{ 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("UV Sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID_UV, S_UV);

}

void loop()      
{     
  uint16_t uv = analogRead(0);// Get UV value
  Serial.println(uv);
  for (int i = 0; i < 13; i++)
  {
    if (uv <= uvIndexValue[i]) 
    {
      uvIndex = i;
      break;
    }
  }
  if (uvIndex != lastUV) {
      gw.sendVariable(CHILD_ID_UV, V_UV, uvIndex);
      lastUV = uvIndex;
  }
  
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(1000); //delay to allow serial to fully print before sleep
  gw.powerDown();
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(SLEEP_TIME * 1000); //sleep for: sleepTime 
}
