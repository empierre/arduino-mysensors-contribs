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
 * Version 1.0 - Henrik EKblad
 * 
 * DESCRIPTION
 * This sketch provides an example how to implement a distance sensor using HC-SR04 
 * Use this sensor to measure KWH and Watt of your house meeter
 * You need to set the correct pulsefactor of your meeter (blinks per KWH).
 * The sensor starts by fetching current KWH value from gateway.
 * Reports both KWH and Watt back to gateway.
 *
 * Unfortunately millis() won't increment when the Arduino is in 
 * sleepmode. So we cannot make this sensor sleep if we also want 
 * to calculate/report watt-number.
 * http://www.mysensors.org/build/pulse_power
 */

#include <SPI.h>
#include <MySensor.h>  

#define ANALOG_INPUT_SENSOR 0                   // The analog input you attached your sensor. 
#define PULSE_FACTOR 1000       // Nummber of blinks per KWH of your meter
#define SLEEP_MODE false        // Watt-value can only be reported when sleep mode is false.
#define MAX_WATT 10000          // Max watt value to report. This filters outliers.
#define CHILD_ID 1              // Id of the sensor child
unsigned long SEND_FREQUENCY = 20000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway.

MySensor gw;
double ppwh = ((double)PULSE_FACTOR)/1000; // Pulses per watt hour
boolean pcReceived = false;
volatile unsigned long pulseCount = 0;   
volatile unsigned long lastBlink = 0;
volatile unsigned long watt = 0;
unsigned long oldPulseCount = 0;   
unsigned long oldWatt = 0;
double oldKwh;
unsigned long lastSend;
int val=0;int oldval=0;

MyMessage wattMsg(CHILD_ID,V_WATT);
MyMessage kwhMsg(CHILD_ID,V_KWH);
MyMessage pcMsg(CHILD_ID,V_VAR1);


void setup()  
{  
  gw.begin(incomingMessage);

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Energy Meter TCRT5000", "1.0");

  // Register this device as power sensor
  gw.present(CHILD_ID, S_POWER);

  // Fetch last known pulse count value from gw
  gw.request(CHILD_ID, V_VAR1);
  
  pulseCount = oldPulseCount = 0;

  lastSend=millis();

  Serial.begin(115200); 
}


void loop()     
{ 
  gw.process();
  unsigned long now = millis();

  val=analogRead(ANALOG_INPUT_SENSOR);
  Serial.println("-");
  Serial.println(val);
  Serial.println("-");
  if ((oldval <100) and (val >100)) {
    pulseCount++;
    oldval=val;
    Serial.println(val);
    Serial.println(" ");
  } else {
        oldval=val;
  }
  delay(100);  
  
  // Only send values at a maximum frequency or woken up from sleep
  bool sendTime = now - lastSend > SEND_FREQUENCY;
  
    // Pulse cout has changed
    if (pulseCount != oldPulseCount) {
      gw.send(pcMsg.set(pulseCount));  // Send pulse count value to gw 
      double kwh = ((double)pulseCount/((double)PULSE_FACTOR));     
      oldPulseCount = pulseCount;
      if (kwh != oldKwh) {
        gw.send(kwhMsg.set(kwh, 4));  // Send kwh value to gw 
        oldKwh = kwh;
      }
    }    
    lastSend = now;
    if (sendTime && !pcReceived) {
    // No count received. Try requesting it again
    gw.request(CHILD_ID, V_VAR1);
    lastSend=now;
  }
  
}

void incomingMessage(const MyMessage &message) {
  if (message.type==V_VAR1) {  
    pulseCount = oldPulseCount = message.getLong();
    Serial.print("Received last pulse count from gw:");
    Serial.println(pulseCount);
    pcReceived = true;
  }
}


