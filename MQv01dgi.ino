/*
  Vera Arduino MQ135

  connect the sensor as follows :

  A H A   >>> 5V
  B		  >>> A0
  H       >>> GND
  B       >>> 10K ohm >>> GND
  
  Contribution: epierre
  Based on David Gironi http://davidegironi.blogspot.fr/2014/01/cheap-co2-meter-using-mq135-sensor-with.html

  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
*/

#include <Sleep_n0m1.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>  
#include <Sensor.h>  
#include <Wire.h> 

#define CHILD_ID_AIQ 0
#define MQ2_SENSOR 0
#define MQ6_SENSOR 1
#define MQ131_SENSOR 2
#define TGS2600_SENSOR 3
#define MQ135_SENSOR 4
#define DUST_SENSOR_ANALOG_PIN  11
#define DUST_SENSOR_DIGITAL_PIN 13
/************************Hardware Related Macros************************************/
#define 	MQ_SENSOR                    (0)  //define which analog input channel you are going to use
#define         RL_VALUE                     (5)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
/***********************Software Related Macros************************************/
#define         CALIBRATION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
/**********************Application Related Macros**********************************/
#define         GAS_CL2                      (0)
#define         GAS_O3                       (1)  // for AIQ
#define         GAS_CO2                      (2)
#define         GAS_CO                       (3)  // for AIQ
#define         GAS_NH4                      (4)
#define         GAS_CO2H50H                  (5)
#define         GAS_CH3                      (6)
#define         GAS_CH3_2CO                  (7)
#define         GAS_H2                       (8)
#define         GAS_C2H5OH                   (9)
#define         GAS_C4H10                   (10)
#define         GAS_LPG                     (11)
#define         GAS_Smoke                   (12)
#define         GAS_CO_sec                  (13)  // for AIQ
#define         GAS_LPG_sec                 (14)
#define         GAS_CH4                     (15)
#define         GAS_NO2                     (16)  // for AIQ
#define         GAS_SO2                     (17)  // for AIQ
/*****************************Globals***********************************************/
float           COCurve[2]      =  {37793.94418, -3.24294658};  //MQ2
float           H2Curve[2]      =  {957.1355042, -2.07442628};  //MQ2
float           LPGCurve[2]     =  {591.6128784, -1.679699732}; //MQ2
float           SmokeCurve[2]   =  {3426.376355, -2.225037973}; //MQ2
float           LPG_secCurve[2] =  {1051.200149, -2.434978052}; //MQ6
float           CH4Curve[2]     =  {1081.498208, -1.443059209}; //MQ6
float           H2_secCurve[2]  =  {137997.7173, -3.76632598};  //MQ6
float           CL2Curve[2]     =  {56.01727602, -1.359048399}; //MQ131
float           O3Curve[2]      =  {42.84561841, -1.043297135}; //MQ131
float           CO2Curve[2]     =  {113.7105289, -3.019713765}; //MQ135
float           CO_secCurve[2]  =  {726.7809737, -4.040111669}; //MQ135
float           NH4Curve[2]     =  {84.07117895, -4.41107687};  //MQ135
float           CO2H50HCurve[2] =  {74.77989144, 3.010328075};  //MQ135
float           CH3Curve[2]     =  {47.01770503, -3.281901967}; //MQ135
float           CH3_2COCurve[2] =  {7.010800878, -2.122018939}; //MQ135
float           C2H5OHCurve[2]  =  {0.2995093465, -3.148170562}; //TGS2600
float           C4H10Curve[2]   =  {0.3555567714, -3.337882361}; //TGS2600
float           H2_terCurve[2]  =  {0.3417050674, -2.887154835}; //TGS2600
float           Ro              =  10;                 //Ro is initialized to 10 kilo ohms


unsigned long SLEEP_TIME = 30; // Sleep time between reads (in seconds)
//VARIABLES
//float Ro = 10000.0; // this has to be tuned 10K Ohm
float Ro0 = 4.340;    //3.83 this has to be tuned 10K Ohm
float Ro1 = 1.755;    //25.76 this has to be tuned 10K Ohm
float Ro2 = 2.501;    //2.24 this has to be tuned 10K Ohm
float Ro3 = 2.511;    //0.05 this has to be tuned 10K Ohm
float Ro4 = 2.511;    //2.51 this has to be tuned 10K Ohm
int val = 0;          // variable to store the value coming from the sensor
float valAIQ0 =0.0;
float lastAIQ0 =0.0;
float valAIQ1 =0.0;
float lastAIQ1 =0.0;
float valAIQ2 =0.0;
float lastAIQ2 =0.0;
float valAIQ3 =0.0;
float lastAIQ3 =0.0;
float valAIQ4 =0.0;
float lastAIQ4 =0.0;
float calcVoltage = 0;
float dustDensity = 0;

Sleep sleep;
Sensor gw(48,49);  // Arduino Mega initialization

#define CHILD_ID_MQ2 0
#define CHILD_ID_MQ6 1
#define CHILD_ID_MQ131 2
#define CHILD_ID_TGS2600 3


void setup()  
{ 
  //Serial.begin(115200);
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ Sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID_AIQ, 22);  

  Serial.print("Ro -->\n    MQ2:"); 
  Ro0 = MQCalibration(MQ2_SENSOR);
  Serial.println(Ro0);
  Serial.print("    MQ6:"); 
  Ro1 = MQCalibration(MQ6_SENSOR);
  Serial.println(Ro1);
  Serial.print("    MQ131:"); 
  Ro2 = MQCalibration(MQ131_SENSOR);
  Serial.println(Ro2);
  Serial.print("    TGZS2600:"); 
  Ro3 = MQCalibration(TGS2600_SENSOR);
  Serial.println(Ro3);
  Serial.print("    MQ135:"); 
  Ro3 = MQCalibration(MQ135_SENSOR);
  Serial.println(Ro4);
 pinMode(DUST_SENSOR_DIGITAL_PIN,OUTPUT); //light on led
}


void loop()      
{     
   //MQ2 CO LPG Smoke
   Serial.print("MQ2    :"); 
   Serial.print("LPG   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR),Ro0,GAS_LPG,MQ2_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("   ");   
   Serial.print("CO    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR),Ro0,GAS_CO_sec,MQ2_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("SMOKE :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR),Ro0,GAS_Smoke,MQ2_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("\n");
   //MQ6
   Serial.print("MQ6    :"); 
   Serial.print("LPG   :");
   Serial.print(MQGetGasPercentage(MQRead(MQ6_SENSOR),Ro1,GAS_LPG_sec,MQ6_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("    "); 
   Serial.print("CH4   :");
   Serial.print(MQGetGasPercentage(MQRead(MQ6_SENSOR),Ro1,GAS_CH4,MQ6_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("\n");
   //MQ131 CL2 O3
   Serial.print("MQ131  :"); 
   Serial.print("CL2   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ131_SENSOR),Ro2,GAS_CL2,MQ131_SENSOR) );
   Serial.print( "ppm" );
      Serial.print("    ");   
   Serial.print("O3    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ131_SENSOR),Ro2,GAS_O3,MQ131_SENSOR) );
      gw.sendVariable(CHILD_ID_AIQ,42, (int)ceil(MQGetGasPercentage(MQRead(MQ131_SENSOR),Ro2,GAS_O3,MQ131_SENSOR)));
   Serial.print( "ppm" );
   Serial.print("\n");
   //TGS2600 H2 C2H5OH C4H10   
   Serial.print("TGS2600:"); 
   Serial.print("H2    :"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR),Ro3,GAS_H2,TGS2600_SENSOR) );
   Serial.print( "ppm" );
      Serial.print("  ");   
   Serial.print("C2H5OH:"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR),Ro3,GAS_C2H5OH,TGS2600_SENSOR) );
   Serial.print( "ppm" );
      Serial.print(" ");   
   Serial.print("C4H10 :"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR),Ro3,GAS_C4H10,TGS2600_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("\n");    
   //MQ1365  CO NH4 CH3 CO2
   Serial.print("MQ135  :"); 
   Serial.print("CO2   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR),Ro4,GAS_CO2,MQ135_SENSOR) );
   Serial.print( "ppm" );      
      Serial.print("    ");   
   Serial.print("CO    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR),Ro4,GAS_CO,MQ135_SENSOR) );
   Serial.print( "ppm" );      
   gw.sendVariable(CHILD_ID_AIQ,41, (int)ceil(MQGetGasPercentage(MQRead(MQ135_SENSOR),Ro4,GAS_CO,MQ135_SENSOR)));
      Serial.print("     ");   
   Serial.print("CH3   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR),Ro4,GAS_CH3,MQ135_SENSOR) );
   Serial.print( "ppm" );      
      Serial.print("    ");   
   Serial.print("NH4    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR),Ro4,GAS_NH4,MQ135_SENSOR) );
   Serial.print( "ppm" );      
   Serial.print("\n");  

  digitalWrite(DUST_SENSOR_DIGITAL_PIN,LOW); // power on the LED
  delayMicroseconds(280);
  uint16_t voMeasured = analogRead(DUST_SENSOR_ANALOG_PIN);// Get DUST value
  delayMicroseconds(40);
  digitalWrite(DUST_SENSOR_DIGITAL_PIN,HIGH); // turn the LED off
  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);  // Adapt to devic voltage
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  //  dustDensity = (0.17 * calcVoltage - 0.1)*1000;
  dustDensity = (0.17 * calcVoltage - 0.1);
  Serial.print("Dust   :raw   : ");
  Serial.print(voMeasured);
  Serial.print("    Voltage: ");
  Serial.print(calcVoltage);
  Serial.print(" - Dust Density: ");
  Serial.println(dustDensity); // unit: ug/m3  
  Serial.print("\n");  
  
  gw.sendVariable(CHILD_ID_AIQ,40, (int)ceil(dustDensity));
   
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(1000); //delay to allow serial to fully print before sleep
  gw.powerDown();
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(SLEEP_TIME * 1000); //sleep for: sleepTime 
}




/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}
 
/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
 
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
 
  return val; 
}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}
 
/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which 
         calculates the ppm (parts per million) of the target gas.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, float ro, int gas_id, int sensor_id)
{
  if (sensor_id == MQ2_SENSOR ) {
    if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,ro,COCurve);      //MQ2
    } else if ( gas_id == GAS_H2 ) {
     return MQGetPercentage(rs_ro_ratio,ro,H2Curve);      //MQ2
    } else if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,ro,LPGCurve);     //MQ2
    } else if ( gas_id == GAS_Smoke ) {
     return MQGetPercentage(rs_ro_ratio,ro,SmokeCurve);   //MQ2
    }    
  } else if (sensor_id == MQ6_SENSOR ){
    if ( gas_id == GAS_LPG_sec ) {
      return MQGetPercentage(rs_ro_ratio,ro,LPG_secCurve);  //MQ6
    } else if ( gas_id == GAS_CH4 ) {
      return MQGetPercentage(rs_ro_ratio,ro,CH4Curve);      //MQ6
    } else if ( gas_id == GAS_H2 ) {
      return MQGetPercentage(rs_ro_ratio,ro,H2_secCurve);   //MQ6
    }        
  } else if (sensor_id == MQ131_SENSOR ){
    if ( gas_id == GAS_CL2 ) {
       return MQGetPercentage(rs_ro_ratio,ro,CL2Curve);     //MQ131
    } else if ( gas_id == GAS_O3 ) {
       return MQGetPercentage(rs_ro_ratio,ro,O3Curve);      //MQ131
    }    
  } else if (sensor_id == MQ135_SENSOR ){
    if ( gas_id == GAS_CO2 ) {
     return MQGetPercentage(rs_ro_ratio,ro,CO2Curve);     //MQ135
    } else if ( gas_id == GAS_NH4 ) {
     return MQGetPercentage(rs_ro_ratio,ro,NH4Curve);     //MQ135
    } else if ( gas_id == GAS_CO2H50H ) {
     return MQGetPercentage(rs_ro_ratio,ro,CO2H50HCurve); //MQ135
    } else if ( gas_id == GAS_CH3 ) {
     return MQGetPercentage(rs_ro_ratio,ro,CH3Curve);     //MQ135
    } else if ( gas_id == GAS_CH3_2CO ) {
     return MQGetPercentage(rs_ro_ratio,ro,CH3_2COCurve); //MQ135
    } else if ( gas_id == GAS_CO_sec ) {
     return MQGetPercentage(rs_ro_ratio,ro,CO_secCurve);  //MQ135
    }
  } else if (sensor_id == TGS2600_SENSOR ){
    if ( gas_id == GAS_C2H5OH ) {
      return MQGetPercentage(rs_ro_ratio,ro,C2H5OHCurve);  //TGS2600
    } else if ( gas_id == GAS_C4H10 ) {
       return MQGetPercentage(rs_ro_ratio,ro,C4H10Curve);   //TGS2600
    } else if ( gas_id == GAS_H2 ) {
       return MQGetPercentage(rs_ro_ratio,ro,H2_terCurve);  //TGS2600
    }    
  }
 
  return 0;
}
 
/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(rs_ro_ratio) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 
int  MQGetPercentage(float rs_ro_ratio, float ro, float *pcurve)
{
  return (double)(pcurve[0] * pow(((double)rs_ro_ratio/ro), pcurve[1]));
}
