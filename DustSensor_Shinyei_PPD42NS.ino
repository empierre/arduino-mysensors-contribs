/*
  Arduino Dust Sensor for Shinyei PPD42NS

  connect the sensor as follows :

  *Please note both analogic and digital must be wired at the same time*

  Analogic connector: 
    VCC       >>> 5V
    Yellow    >>> D8
    GND       >>> GND

  Based on: http://www.howmuchsnow.com/arduino/airquality/grovedust/
  Authors: Chris Nafis Apris 2012
  Datasheet:  http://www.sca-shinyei.com/pdf/PPD42NS.pdf
  Contribution: epierre

  
  The dust sensor used (see purchase guide for latest link):
 
  
*/

#include <MySensor.h>  
#include <SPI.h>

#define CHILD_ID_DUST 0
#define DUST_SENSOR_DIGITAL_PIN 8

unsigned long SLEEP_TIME = 30*1000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUST =0.0;
float lastDUST =0.0;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

MySensor gw;
MyMessage dustMsg(CHILD_ID_DUST, V_DUST_LEVEL);

void setup()  
{
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Dust Sensor", "1.2");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_DUST, S_DUST);  
  
  pinMode(DUST_SENSOR_DIGITAL_PIN,INPUT);
  Serial.begin(115200);
  starttime = millis();
   
}

void loop()      
{    
  
  duration = pulseIn(DUST_SENSOR_DIGITAL_PIN, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print(lowpulseoccupancy);
    Serial.print(",");
    Serial.print(ratio);
    Serial.print(",");
    Serial.println(concentration);
    lowpulseoccupancy = 0;
    starttime = millis();
  }
  
  if (ceil(concentration) != lastDUST) {
      gw.send(dustMsg.set((int)ceil(concentration)));
      lastDUST = ceil(concentration);
  }
 
  gw.sleep(SLEEP_TIME);
}
