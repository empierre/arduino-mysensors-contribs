/*
  Arduino Soil Moisture sensor with SHT1x

  connect the sensor as follows :
  
  VCC   -- Red = VCC (3-5VDC)
  GND   -- Black = Ground
  Pin 8 -- Yellow = Clock
  Pin 3 -- Green =  Data.
  
  Contribution: epierre
  Based on David Gironi http://davidegironi.blogspot.fr/2014/01/cheap-co2-meter-using-mq135-sensor-with.html

  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 */

#include <SPI.h>
#include <MySensor.h>  
#include "SHT1x.h"

#define DIGITAL_INPUT_SOIL_SENSOR 3   // Digital input did you attach your soil sensor.  
#define CHILD_ID_TEMP 0   // Id of the sensor child
#define CHILD_ID_HUM 1   // Id of the sensor child
#define sckPin 8 //serial clock

 
// We are using the SHT10
// Humid accuracy +/- 5%
// Steady accuracy between 10-80
// example at 10/90 +/- 6%, 0/100 +/- 7.5%
 
// Temp accuracy +/- .5 degrees celcius
// Temp error increases more as we get farther from 25 celc.
// example: @ 0/50 degrees, +/- 1.2 degrees

 
SHT1x th_sensor(DIGITAL_INPUT_SOIL_SENSOR, sckPin);

MySensor gw;
MyMessage msgtemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msghum(CHILD_ID_HUM, V_HUM);
int lastTempValue = -1;
int lastHumValue = -1;
unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)

void setup()  
{ 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Soil Moisture Sensor SHT1x", "1.0");
    
  // Register all sensors to gw (they will be created as child devices)  
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_HUM, S_HUM);
}
 
void loop()     
{     
  float temp_c;
  float humid;
  // Read values from the sensor
  humid = th_sensor.readHumidity();
  // Since the humidity reading requires the temperature we simply
  // retrieve the reading capture from the readHumidity() call. See the lib.
  temp_c = th_sensor.readTemperatureC();
 

  if (temp_c != lastTempValue) {
    Serial.println(temp_c);
    gw.send(msgtemp.set(temp_c, 1));  // Send the inverse to gw as tripped should be when no water in soil
    lastTempValue = temp_c;
  }
   if (humid != lastHumValue) {
    Serial.println(humid);
    gw.send(msghum.set(humid, 1));  // Send the inverse to gw as tripped should be when no water in soil
    lastHumValue = humid;
  }
  // Power down the radio and arduino until digital input changes.
  gw.sleep(SLEEP_TIME); //sleep a bit
}
