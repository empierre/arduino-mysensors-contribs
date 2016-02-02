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
 *    Air Quality Sensors for HCHO sensor
 * 
 *  Wiring:
 * 
 *  sensor side with 7 pin on the right, top pin is 1
 * 1 VCC -> (Arduino) 3V3
 * 2 GND -> (Arduino) GND
 * 6 TX -> (Arduino) Pin 8
 * 7 RX -> (Arduino) Pin 7
 * 
 * based on: terryroy terry.ouyang@gmail.com
 * https://github.com/terryoy/airmonitor
 * Contribution: epierre
 *   
 * Precaution:
 *    The gasses detected by these gas sensors can be deadly in high concentrations. Always be careful to perform gas tests in well ventilated areas.
 * 
 *  Note:
 *     THESE GAS SENSOR MODULES ARE NOT DESIGNED FOR OR APPROVED FOR ANY APPLICATION INVOLVING HEALTH OR HUMAN SAFETY. THESE GAS SENSOR MODULES ARE FOR EXPERIMENTAL PURPOSES ONLY.*
 /*

 */
#include <MySensor.h> 
#include <Wire.h>

/* initialize air module */
#include <SoftwareSerial.h>
#include "AirModule.h"
#define rxPin 8
#define txPin 7
#define CHILD_ID 0

unsigned long SLEEP_TIME = 600; // Sleep time between reads (in seconds)

//VARIABLES
int val = 0;          // variable to store the value coming from the sensor

SoftwareSerial airSerial(8, 7); // RX, TX
char testCmd[7] = {0x42, 0x4d, 0x01, 0x00, 0x00, 0x00, 0x90};
char testResponse[20];

MySensor gw;  // Arduino  initialization
MyMessage msg_hcho(CHILD_ID, V_UNIT_PREFIX);

void setup() {



  Serial.begin(115200);
  airSerial.begin(9600);

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  gw.begin();
    
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ HCHO", "1.1");
  
  gw.present(CHILD_ID, S_AIR_QUALITY);  
  gw.send(msg_hcho.set("ppm"));
}

void loop() {
  // request hcho
  for (int i=0;i<7;i++) {
    airSerial.write(testCmd[i]);
  }
  
  // response handle
  int pos = 0;
  for (int i=0;i<20;i++) testResponse[i]=0x00; // clean buffer
  
  while (airSerial.available() && pos < 20) {
    testResponse[pos] = airSerial.read();
    //Serial.print(testResponse[pos], HEX);
    delay(1);
    
    if (pos == 0 && testResponse[pos] != 0x42) {
      testResponse[pos] = 0; // clear the buffer
    } else if (pos == 1 && testResponse[pos] != 0x4d) {
      testResponse[0] = testResponse[1] = 0;
      pos = 0;
    }
    else {
      pos++;
    }
  }

  // parse data
  if (testResponse[0] == 0x42 && testResponse[1] == 0x4d) {
     //for (int i=0;i<20;i++) Serial.print(testResponse[i], HEX);
    AirResponse air;
    parse_air_response(&air, testResponse);
    //move mg/m3 to ppm, HCHO weight is 30.03   
    long int v=get_read_value(air)*(8,31441*298,15)/(30,03*101,325);
    
    if ((v != val)&&(v>0)) {
      gw.send(msg_hcho.set((long int)v));  
      val=v;
    }
    //Serial.println(get_unit_display(air));    
  }
      
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  gw.sleep(SLEEP_TIME*4); //sleep for: sleepTime
}
