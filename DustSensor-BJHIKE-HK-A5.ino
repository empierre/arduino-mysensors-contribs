/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2017 Sensnology AB
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
 * Arduino Dust Sensor for DfRobot HK-A5 laser sensor
 * connect the sensor as follows :
 *         RX -> TX
 *         TX -> RX
 *        VCC -> VCC
 *        GND -> GND
 *	  
 *  Based on: https://www.dfrobot.com/wiki/index.php/PM2.5_laser_dust_sensor_SKU:SEN0177
 *  Author：Zuyang @ HUST
 *  Modified by Cain for Arduino Hardware Serial port compatibility
 * Datasheet:  https://github.com/Arduinolibrary/DFRobot_PM2.5_Sensor_module/raw/master/HK-A5%20Laser%20PM2.5%20Sensor%20V1.0.pdf
 * Contribution: epierre
 * 
 * The dust sensor used (see purchase guide for latest link):
*/

#include <MySensor.h>  
#include <SPI.h>

#define CHILD_ID_DUST_PM10            0
#define CHILD_ID_DUST_PM25            1
#define DUST_SENSOR_DIGITAL_PIN_PM10  3
#define DUST_SENSOR_DIGITAL_PIN_PM25  6

unsigned long SLEEP_TIME = 30*1000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUSTPM25 =0.0;
float lastDUSTPM25 =0.0;
float valDUSTPM10 =0.0;
float lastDUSTPM10 =0.0;
float PM01Value;
float PM10_0Value;
float PM2_5Value;
float ppmv2_5;
float ppmv10_0;
float ppmv1_0;
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
long concentrationPM25 = 0;
long concentrationPM10 = 0;
int temp=20; //external temperature, if you can replace this with a DHT11 or better 
long ppmv;
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

  
MySensor gw;
MyMessage dustMsgPM10(CHILD_ID_DUST_PM10, V_LEVEL);
MyMessage msgPM10(CHILD_ID_DUST_PM10, V_UNIT_PREFIX);
MyMessage dustMsgPM25(CHILD_ID_DUST_PM25, V_LEVEL);
MyMessage msgPM25(CHILD_ID_DUST_PM25, V_UNIT_PREFIX);


void setup()  
{
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Dust Sensor DFROBOT HK-A5", "1.4");

// Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_DUST_PM10, S_DUST);  
  gw.send(msgPM10.set("ppm"));
  gw.present(CHILD_ID_DUST_PM25, S_DUST);  
  gw.send(msgPM25.set("ppm"));  
  
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM10,INPUT);
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM25,INPUT);
  Serial.begin(9600);   //use serial0
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor

}

void loop()      
{    
 if(Serial.find(0x42)){    //start to read when detect 0x42
    Serial.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10_0Value=transmitPM10(buf); //count PM10 value of the air detector module 
        //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
        //0.08205   = Universal gas constant in atm·m3/(kmol·K)
        ppmv2_5=(PM2_5Value*0.0283168/100/1000) *  (0.08205*temp)/0.01;
        //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
        //0.08205   = Universal gas constant in atm·m3/(kmol·K)
        ppmv10_0=(PM10_0Value*0.0283168/100/1000) *  (0.08205*temp)/0.01;
      }           
    } 
   }
    
  if ((ceil(ppmv2_5) != lastDUSTPM25)&&((long)ppmv2_5>0)) {
      gw.send(dustMsgPM25.set((long)ppmv2_5));
      lastDUSTPM25 = ceil(ppmv2_5);
  }
    
  if ((ceil(ppmv10_0) != lastDUSTPM10)&&((long)ppmv10_0>0)) {
      gw.send(dustMsgPM10.set((long)ppmv10_0));
      lastDUSTPM10 = ceil(ppmv10_0);
  }
 
  //sleep to save on radio
  gw.sleep(SLEEP_TIME);
  

 static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      
      Serial.print("PM1.0: ");  
      Serial.print(PM01Value);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5: ");  
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");     
      
      Serial.print("PM1 0: ");  
      Serial.print(PM10_0Value);
      Serial.println("  ug/m3");   
      Serial.println();
    }
  
}

float conversion25(long concentrationPM25) {
  double pi = 3.14159;
  double density = 1.65 * pow (10, 12);
  double r25 = 0.44 * pow (10, -6);
  double vol25 = (4/3) * pi * pow (r25, 3);
  double mass25 = density * vol25;
  double K = 3531.5;
  return (concentrationPM25) * K * mass25;
}

float conversion10(long concentrationPM10) {
  double pi = 3.14159;
  double density = 1.65 * pow (10, 12);
  double r10 = 0.44 * pow (10, -6);
  double vol10 = (4/3) * pi * pow (r10, 3);
  double mass10 = density * vol10;
  double K = 3531.5;
  return (concentrationPM10) * K * mass10;
}



char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}
