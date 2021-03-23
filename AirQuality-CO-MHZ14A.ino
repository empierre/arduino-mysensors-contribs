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
 * DESCRIPTION
 *
 *  MH-Z14 CO2 sensor
 * 
 *  Wiring:
 *   PWM                    PIN6
 *   TXD         PAD18
 *   RXD         PAD19
 *   VCC         PAD17      VCC
 *   GND         PAD16      GND
 *   Analog
 *   HD
 *
 *	From: http://davidegironi.blogspot.fr/2014/01/co2-meter-using-ndir-infrared-mh-z14.html
 * 	  MH-Z14 has a PWM output, with a sensitivity range of 0ppm to 2000ppm CO2, an accurancy of ±200ppm.
 * 	  The cycle is 1004ms±5%, given the duty cicle Th (pulse high), Tl is 1004-Th, we can convert it to CO2 value using the formula:
 *	  CO2ppm = 2000 * (Th - 2ms) /(Th + Tl - 4ms)
 * 	From: http://airqualityegg.wikispaces.com/Sensor+Tests
 *	  - response time is less than 30 s
 *   - 3 minute warm up time
 *	datasheet: http://www.futurlec.com/Datasheet/Sensor/MH-Z14.pdf
* Contributor: epierre
**/


#include <MySensor.h>  
#include <SPI.h>

#define CHILD_ID_AIQ 0
#define AIQ_SENSOR_ANALOG_PIN 6

unsigned long SLEEP_TIME = 30*1000; // Sleep time between reads (in milliseconds)

float valAIQ =0.0;
float lastAIQ =0.0;

MySensor gw;
MyMessage msg(CHILD_ID_AIQ, V_LEVEL);
MyMessage msg2(CHILD_ID_AIQ, V_UNIT_PREFIX);

void setup()  
{
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ Sensor CO2 MH-Z14A", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_AIQ, S_AIR_QUALITY);  
  gw.send(msg2.set("ppm"));
  
  gw.sleep(SLEEP_TIME);
  
  pinMode(AIQ_SENSOR_ANALOG_PIN, INPUT);
   
}

void loop() { 

  //unsigned long duration = pulseIn(AIQ_SENSOR_ANALOG_PIN, HIGH);
  while(digitalRead(AIQ_SENSOR_ANALOG_PIN) == HIGH) {;}
  //wait for the pin to go HIGH and measure HIGH time
  unsigned long duration = pulseIn(AIQ_SENSOR_ANALOG_PIN, HIGH);
  
  //Serial.print(duration/1000); Serial.println(" ms ");
  //from datasheet
	//CO2 ppm = 2000 * (Th - 2ms) / (Th + Tl - 4ms)
	//  given Tl + Th = 1004
	//        Tl = 1004 - Th
	//        = 2000 * (Th - 2ms) / (Th + 1004 - Th -4ms)
	//        = 2000 * (Th - 2ms) / 1000 = 2 * (Th - 2ms)
  long co2ppm = 2 * ((duration/1000) - 2);
  //Serial.print(co2ppm);
  if ((co2ppm != lastAIQ)&&(abs(co2ppm-lastAIQ)>=10)) {
      gw.send(msg.set((long)ceil(co2ppm)));
      lastAIQ = ceil(co2ppm);
  }
  
  //Serial.println();
  
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  gw.sleep(SLEEP_TIME); //sleep for: sleepTime
}
