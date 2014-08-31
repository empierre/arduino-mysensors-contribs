/*
  Vera Arduino Dust Sensort

  connect the sensor as follows :

  VCC       >>> 5V
  A         >>> A0
  GND       >>> GND

  Based on: http://www.dfrobot.com/wiki/index.php/Sharp_GP2Y1010AU 
  Authors: Cyrille Médard de Chardon (serialC), Christophe Trefois (Trefex)
  Contribution: epierre
  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)

 
*/

#include <Sleep_n0m1.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>  
#include <Sensor.h>  
#include <Wire.h>

#define CHILD_ID_DUST 0
#define DUST_SENSOR_ANALOG_PIN 1

unsigned long SLEEP_TIME = 30; // Sleep time between reads (in seconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUST =0.0;
float lastDUST =0.0;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

Sensor gw;
Sleep sleep;

void setup()  
{
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Dust Sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID_DUST, 23);  
   
}

void loop()      
{    
  uint16_t voMeasured = analogRead(DUST_SENSOR_ANALOG_PIN);// Get DUST value

  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);

  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = (0.17 * calcVoltage - 0.1)*1000;
 
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
  
  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);
  
  Serial.print(" - Dust Density: ");
  Serial.println(dustDensity); // unit: ug/m3
 
  if (ceil(dustDensity) != lastDUST) {
      gw.sendVariable(CHILD_ID_DUST, V_VAR1, (int)ceil(dustDensity));
      lastDUST = ceil(dustDensity);
  }
 
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(1000); //delay to allow serial to fully print before sleep
  gw.powerDown();
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(SLEEP_TIME * 1000); //sleep for: sleepTime
}
