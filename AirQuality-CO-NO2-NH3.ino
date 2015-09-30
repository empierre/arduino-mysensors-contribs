 /*
  Arduino Multiple Air Quality Sensors for MICS-6814 

  connect the sensor as follows when standalone:
  5V
  GDN
  SDA -> A4 (on nano)
  SCL -> A5 (on nano)

  based on     Jacky Zhang, Embedded Software Engineer   qi.zhang@seeed.cc
  Contribution: epierre
    
  Precaution:
     The gasses detected by these gas sensors can be deadly in high concentrations. Always be careful to perform gas tests in well ventilated areas.
 
  Note:
     THESE GAS SENSOR MODULES ARE NOT DESIGNED FOR OR APPROVED FOR ANY APPLICATION INVOLVING HEALTH OR HUMAN SAFETY. THESE GAS SENSOR MODULES ARE FOR EXPERIMENTAL PURPOSES ONLY.

  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
*/

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
float           Ro              =  10000;                          //Ro is initialized to 10 kilo ohms

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
MyMessage msg_nh3(CHILD_ID_NH3, 40);
MyMessage msg_co(CHILD_ID_CO, 40);
MyMessage msg_no2(CHILD_ID_NO2, 40);
MyMessage msg_c3h8(CHILD_ID_C3H8, 40);
MyMessage msg_c4h10(CHILD_ID_C4H10, 40);
MyMessage msg_ch4(CHILD_ID_CH4, 40);
MyMessage msg_h2(CHILD_ID_H2, 40);
MyMessage msg_c2h5oh(CHILD_ID_C2H5OH, 40);

void setup()  
{ 
  gw.begin();
  
  
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ Multi Sensors MiCS", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_NH3, S_AIR_QUALITY);  
  gw.present(CHILD_ID_CO, S_AIR_QUALITY);  
  gw.present(CHILD_ID_NO2, S_AIR_QUALITY);  
  gw.present(CHILD_ID_C3H8, S_AIR_QUALITY);  
  gw.present(CHILD_ID_C4H10, S_AIR_QUALITY);  
  gw.present(CHILD_ID_CH4, S_AIR_QUALITY);  
  gw.present(CHILD_ID_H2, S_AIR_QUALITY);  
  gw.present(CHILD_ID_C2H5OH, S_AIR_QUALITY);  
  
  metric = gw.getConfig().isMetric;
  

}


void loop()      
{     

  float c;
  
    c = mutichannelGasSensor.measure_NH3();
    Serial.print("The concentration of NH3 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm"); 
    gw.send(msg_nh3.set((long int)ceil(c)));

    c = mutichannelGasSensor.measure_CO();
    Serial.print("The concentration of CO is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_co.set((long int)ceil(c)));

    c = mutichannelGasSensor.measure_NO2();
    Serial.print("The concentration of NO2 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_no2.set((long int)ceil(c)));

    c = mutichannelGasSensor.measure_C3H8();
    Serial.print("The concentration of C3H8 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_c3h8.set((long int)ceil(c)));

    c = mutichannelGasSensor.measure_C4H10();
    Serial.print("The concentration of C4H10 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_c4h10.set((long int)ceil(c)));

    c = mutichannelGasSensor.measure_CH4();
    Serial.print("The concentration of CH4 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_ch4.set((long int)ceil(c)));

    c = mutichannelGasSensor.measure_H2();
    Serial.print("The concentration of H2 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_h2.set((long int)ceil(c)));
    
    c = mutichannelGasSensor.measure_C2H5OH();
    Serial.print("The concentration of C2H5OH is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    gw.send(msg_c2h5oh.set((long int)ceil(c)));
   
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(SLEEP_TIME * 1000); //delay to allow serial to fully print before sleep
  //gw.powerDown();
  //sleep.pwrDownMode(); //set sleep mode
  //gw.sleep(SLEEP_TIME * 1000); //sleep for: sleepTime 
}


