/*
 Use this sensor to measure volume and flow of your house watermeter.
 You need to set the correct pulsefactor of your meter (pulses per m3).
 The sensor starts by fetching current volume reading from gateway (VAR 1).
 Reports both volume and flow back to gateway.

 Sensor on pin analog 0
 
 DFRobot Analog Grayscale Sensor V2
 Watermeter sensus Residia Jet
 
 http://www.dfrobot.com/index.php?route=product/product&product_id=81

Contribution: Hek, adapted by epierre to greyscale sensor water meter

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
*/

#include <MySensor.h>  
#include <SPI.h>

#define ANALOG_INPUT_SENSOR 0                   // The analog input you attached your sensor. 
#define PULSE_FACTOR 1000                       // Nummber of blinks per m3 of your meter (One rotation/liter)
#define SLEEP_MODE false                        // flowvalue can only be reported when sleep mode is false.
#define MAX_FLOW 40                             // Max flow (l/min) value to report. This filetrs outliers.
#define INTERRUPT DIGITAL_INPUT_SENSOR-2        // Usually the interrupt = pin -2 (on uno/nano anyway)
#define CHILD_ID 1                              // Id of the sensor child
unsigned long SEND_FREQUENCY = 20000;           // Minimum time between send (in seconds). We don't want to spam the gateway.

MySensor gw;
MyMessage flowMsg(CHILD_ID,V_FLOW);
MyMessage volumeMsg(CHILD_ID,V_VOLUME);
MyMessage pcMsg(CHILD_ID,V_VAR1);
 
double ppl = ((double)PULSE_FACTOR)/1000;        // Pulses per liter

volatile unsigned long pulseCount = 0;   
volatile unsigned long lastBlink = 0;
volatile double flow = 0;   
boolean pcReceived = false;
unsigned long oldPulseCount = 0;
unsigned long newBlink = 0;   
double oldflow = 0;
double volume;                     
double oldvolume;
unsigned long lastSend;
unsigned long lastPulse;
unsigned long currentTime;
boolean metric;
long lastDebounce = 0;
long debounceDelay = 500;    // Ignore bounces under 1/2 second
int oldval =0;
int val=0;


void setup()  
{  
  gw.begin(incomingMessage); 
  
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Water Meter", "1.0 greyscale");

  // Register this device as Waterflow sensor
  gw.present(CHILD_ID, S_WATER);       

  pulseCount = oldPulseCount = 0;

  // Fetch last known pulse count value from gw
   gw.request(CHILD_ID, V_VAR1);

  lastSend = millis();
  
  //led blinking
  pinMode(13, OUTPUT);
}


void loop()     
{ 
  gw.process();
  currentTime = millis();

  val=analogRead(0);
  //Serial.println(val);
  if ((oldval <100) and (val >100)) {
    pulseCount++;
    oldval=val;
  } else {
        oldval=val;
  }
  delay(100);	
    // Only send values at a maximum frequency or woken up from sleep
  bool sendTime = currentTime - lastSend > SEND_FREQUENCY;
  
    // Pulse count has changed
    if (pulseCount != oldPulseCount) {
      gw.send(pcMsg.set(pulseCount));                  // Send  volumevalue to gw VAR1
      double volume = ((double)pulseCount/((double)PULSE_FACTOR));     
      oldPulseCount = pulseCount;
      if (volume != oldvolume) {
        gw.send(volumeMsg.set(volume, 3));               // Send volume value to gw
        //Serial.print("V=");
        //Serial.println(volume);
        oldvolume = volume;
      } 
    }
    lastSend = currentTime;
    if (sendTime && !pcReceived) {
    // No count received. Try requesting it again
    gw.request(CHILD_ID, V_VAR1);
    lastSend=currentTime;
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




