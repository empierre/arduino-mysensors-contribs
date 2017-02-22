/**
* The MySensors Arduino library handles the wireless radio link and protocol
* between your home built sensors/actuators and HA controller of choice.
* The sensors forms a self healing radio network with optional repeaters. Each
* repeater and gateway builds a routing tables in EEPROM which keeps track of the
* network topology allowing messages to be routed to nodes.
*
* Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
* Copyright (C) 2013-2015 Sensnology AB
* Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
*
* Documentation: http://www.mysensors.org
* Support Forum: http://forum.mysensors.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
*******************************
*
* REVISION HISTORY
* Version 1.0 - epierre
* Converted to 1.4 by Henrik Ekblad
* corrections by brnwshr
* 
* DESCRIPTION
* Arduino Dust Sensort
*
* connect the sensor as follows :
* 
*   VCC       >>> 5V
*   A         >>> A0
*   GND       >>> GND
*   LED       >>> 2
*
* Based on: http://www.dfrobot.com/wiki/index.php/Sharp_GP2Y1010AU 
* Authors: Cyrille Médard de Chardon (serialC), Christophe Trefois (Trefex)
* 
* http://www.mysensors.org/build/dust
* 
*/

#include <MySensor.h>  
#include <SPI.h>
#define CHILD_ID_DUST 0
#define DUST_SENSOR_ANALOG_PIN 0

int ledPower = 2;
const int numreadings = 60;

unsigned long SLEEP_TIME = 10*1000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;// variable to store the value coming from the sensor
float valDUST = 0.0;
float lastDUST =0.0;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

MySensor gw;
MyMessage dustMsg(CHILD_ID_DUST, V_LEVEL);

void setup(){
 gw.begin();
 pinMode(ledPower,OUTPUT);
 // Send the sketch version information to the gateway and Controller
 gw.sendSketchInfo("Dust Sensor", "1.1");
 // Register all sensors to gateway (they will be created as child devices)
 gw.present(CHILD_ID_DUST, S_DUST);  
}

void loop(){
float cumsum = 0;
float temp = 0;
float temp1 = 0;
float cum_density = 0;
     for (int sample = 0; sample < numreadings; sample ++){ // loop reading dust sensor
       digitalWrite(ledPower,LOW); // power on the LED
       delayMicroseconds(samplingTime);
       uint16_t voMeasured = analogRead(DUST_SENSOR_ANALOG_PIN);// Get DUST value
       delayMicroseconds(deltaTime);
        digitalWrite(ledPower,HIGH); // turn the LED off
       // 0 - 5V mapped to 0 - 1023 integer values
       // recover voltage
       temp = voMeasured * (5.0 / 1024.0);
       cumsum = cumsum + temp;// cumulative sum over 60 seconds
       delay(1000);
       Serial.print("reading sample: ");
       Serial.println(sample);
       Serial.print("Raw Signal Value (0-1023): ");
       Serial.println(voMeasured);    
       Serial.print(" - cumulative: ");
       Serial.println(cumsum);
}
// linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
// Chris Nafis (c) 2012
dustDensity = (0.17 * cumsum - 0.1)* (1000/60);
       
Serial.print(" - Dust Density: ");
Serial.println(dustDensity); // unit: ug/m3
Serial.println("#########################################");

//  if (ceil(dustDensity) != lastDUST) {
//      gw.send(dustMsg.set((int)ceil(dustDensity)));
//      lastDUST = ceil(dustDensity);
//  }
gw.send(dustMsg.set((int)ceil(dustDensity)));
 gw.sleep(SLEEP_TIME);
}
