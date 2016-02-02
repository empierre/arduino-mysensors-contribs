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
 *    Air Quality Sensors for MICS-6814 
 * 
 *  Wiring:
 * 
 * 5V
 * GDN
 * SDA -> A4 (on nano)
 * SCL -> A5 (on nano)
 *
 * based on     Jacky Zhang, Embedded Software Engineer   qi.zhang@seeed.cc
 * Contribution: epierre
 *   
 * Precaution:
 *    The gasses detected by these gas sensors can be deadly in high concentrations. Always be careful to perform gas tests in well ventilated areas.
 * 
 *  Note:
 *     THESE GAS SENSOR MODULES ARE NOT DESIGNED FOR OR APPROVED FOR ANY APPLICATION INVOLVING HEALTH OR HUMAN SAFETY. THESE GAS SENSOR MODULES ARE FOR EXPERIMENTAL PURPOSES ONLY.*
 **/

#include <SPI.h>  
#include <MySensor.h>  
#include <Wire.h> 
#include <MutichannelGasSensor.h>

/**********************Application Related Macros**********************************/
#define         GAS_CL2                      (0)
#define         GAS_O3                       (1)  
#define         GAS_CO2                      (2)
#define         GAS_CO                       (3) 
#define         GAS_NH4                      (4)
#define         GAS_CH3                      (6)
#define         GAS_CH3_2CO                  (7)
#define         GAS_H2                       (8)
#define         GAS_C2H5OH                   (9) //Alcohol, Ethanol
#define         GAS_C4H10                   (10)
#define         GAS_LPG                     (11)
#define         GAS_Smoke                   (12)
#define         GAS_CO_sec                  (13)  
#define         GAS_LPG_sec                 (14)
#define         GAS_CH4                     (15)
#define         GAS_NO2                     (16)  
#define         GAS_SO2                     (17) 
#define         GAS_C7H8                    (18) //Toluene
#define         GAS_H2S                     (19) //Hydrogen Sulfide
#define         GAS_NH3                     (20) //Ammonia
#define         GAS_C6H6                    (21) //Benzene
#define         GAS_C3H8                    (22) //Propane
#define         GAS_NHEX                    (23) //n-hexa
#define         GAS_HCHO                    (24) //HCHO / CH2O Formaldehyde
/*****************************Globals***********************************************/

unsigned long SLEEP_TIME = 600; // Sleep time between reads (in seconds)
//VARIABLES
int val = 0;          // variable to store the value coming from the sensor


float calcVoltage = 0;
boolean metric = true; 
//test
float a=0;
boolean pcReceived = false;

#define CHILD_ID_NH3 0
#define CHILD_ID_CO 1
#define CHILD_ID_NO2 2
#define CHILD_ID_C3H8 3
#define CHILD_ID_C4H10 4
#define CHILD_ID_CH4 5
#define CHILD_ID_H2 6
#define CHILD_ID_C2H5OH 7

MySensor gw;  // Arduino  initialization
MyMessage msg_nh3(CHILD_ID_NH3, V_UNIT_PREFIX);
MyMessage msg_co(CHILD_ID_CO, V_UNIT_PREFIX);
MyMessage msg_no2(CHILD_ID_NO2, V_UNIT_PREFIX);
MyMessage msg_c3h8(CHILD_ID_C3H8, V_UNIT_PREFIX);
MyMessage msg_c4h10(CHILD_ID_C4H10, V_UNIT_PREFIX);
MyMessage msg_ch4(CHILD_ID_CH4, V_UNIT_PREFIX);
MyMessage msg_h2(CHILD_ID_H2, V_UNIT_PREFIX);
MyMessage msg_c2h5oh(CHILD_ID_C2H5OH, V_UNIT_PREFIX);

int val_nh3=0;
int val_co=0;
int val_no2=0;
int val_c3h8=0;
int val_c4h10=0;
int val_ch4=0;
int val_h2=0;
int val_c2h5oh=0;

void setup()  
{ 
  gw.begin();
  
  
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ Multi Sensors MiCS", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_NH3, S_AIR_QUALITY);  
  gw.send(msg_nh3.set("ppm"));
  gw.present(CHILD_ID_CO, S_AIR_QUALITY);  
  gw.send(msg_co.set("ppm"));
  gw.present(CHILD_ID_NO2, S_AIR_QUALITY);  
  gw.send(msg_no2.set("ppm"));
  gw.present(CHILD_ID_C3H8, S_AIR_QUALITY);  
  gw.send(msg_c3h8.set("ppm"));
  gw.present(CHILD_ID_C4H10, S_AIR_QUALITY);  
  gw.send(msg_c4h10.set("ppm"));
  gw.present(CHILD_ID_CH4, S_AIR_QUALITY);  
  gw.send(msg_ch4.set("ppm"));
  gw.present(CHILD_ID_H2, S_AIR_QUALITY);  
  gw.send(msg_h2.set("ppm"));
  gw.present(CHILD_ID_C2H5OH, S_AIR_QUALITY);  
  gw.send(msg_c2h5oh.set("ppm"));
  mutichannelGasSensor.begin(0x04);//the default I2C address of the slave is 0x04
  mutichannelGasSensor.powerOn();
  
}


void loop()      
{     

  float c;
  
    c = mutichannelGasSensor.measure_NH3();
    
    if((c>=0)&&(c!=val_nh3)) {
        Serial.print("NH3: ");
        Serial.print(c);
        val_nh3=c;
        gw.send(msg_nh3.set(c,3));        
    }        

    c = mutichannelGasSensor.measure_CO(); 
    
    if((c>=0)&&(c!=val_co)) {
      Serial.print("CO: ");
      Serial.print(c);
        val_co=c;
        gw.send(msg_co.set(c,3));        
    }
        
    c = mutichannelGasSensor.measure_NO2();
    
    if((c>=0)&&(c!=val_no2)) {
      Serial.print("NO2: ");
      Serial.print(c);
        val_no2=c;
        gw.send(msg_no2.set(c,3));
    }    
    
    /*c = mutichannelGasSensor.measure_C3H8();
    Serial.print("The concentration of C3H8 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_c3h8.set(c,3));

    c = mutichannelGasSensor.measure_C4H10();
    Serial.print("The concentration of C4H10 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_c4h10.set(c,3));

    c = mutichannelGasSensor.measure_CH4();
    Serial.print("The concentration of CH4 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_ch4.set(c,3));

    c = mutichannelGasSensor.measure_H2();
    Serial.print("The concentration of H2 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_h2.set(c,3));
    
    c = mutichannelGasSensor.measure_C2H5OH();
    Serial.print("The concentration of C2H5OH is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_c2h5oh.set(c,3));*/
   
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  gw.sleep(SLEEP_TIME*4); //sleep for: sleepTime
}


