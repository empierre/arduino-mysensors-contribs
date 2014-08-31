/*
  Vera Arduino MQ135

  connect the sensor as follows :

  A H A   >>> 5V
  B		  >>> A0
  H       >>> GND
  B       >>> 10K ohm >>> GND
  
  Contribution: epierre
  
  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
*/

#include <Sleep_n0m1.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>  
#include <Sensor.h>  
#include <Wire.h> 

#define CHILD_ID_AIQ 0
#define MQ2_SENSOR_ANALOG_PIN 0
#define MQ6_SENSOR_ANALOG_PIN 1
#define MQ131_SENSOR_ANALOG_PIN 2
#define TGS2600_SENSOR_ANALOG_PIN 3
/************************Hardware Related Macros************************************/
#define 	MQ_SENSOR_ANALOG_PIN         (0)  //define which analog input channel you are going to use
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
#define         GAS_O3                       (1)
#define         GAS_CO2                      (2)
#define         GAS_CO                       (3)
#define         GAS_NH4                      (4)
#define         GAS_CO2H50H                  (5)
#define         GAS_CH3                      (6)
#define         GAS_CH3_2CO                  (7)
#define         GAS_H2                       (8)
#define         GAS_C2H5OH                   (9)
#define         GAS_C4H10                   (10)
#define         GAS_LPG                     (11)
#define         GAS_Smoke                   (12)
#define         GAS_CO_sec                  (13)
#define         GAS_LPG_sec                 (14)
#define         GAS_CH4                     (15)
/*****************************Globals***********************************************/
float           CL2Curve[3]     =  {0.70,0.78,-0.78}; 
float           O3Curve[3]      =  {0.69,0.90,-0.93}; 
float           CO2Curve[3]     =  {1.0,0.34,-0.34}; 
float           COCurve[3]      =  {1.0,0.46,-1.01}; 
float           CO_secCurve[3]  =  {2.3,0.72,-0.34}; 
float           NH4Curve[3]     =  {1.0,0.20,-0.23}; 
float           CO2H50HCurve[3] =  {1.0,0.29,-0.33}; 
float           CH3Curve[3]     =  {1.0,0.20,-0.30}; 
float           CH3_2COCurve[3] =  {1.0,-0.82,0.55}; 
float           H2Curve[3]      =  {0,-0.16,-0.34}; 
float           C2H5OHCurve[3]  =  {0,-0.16,-0.32}; 
float           C4H10Curve[3]   =  {0,-0.15,-0.30}; 
float           LPGCurve[3]     =  {2.3,0.21,-0.47};  
float           SmokeCurve[3]   =  {2.3,0.53,-0.44};
float           LPG_secCurve[3] =  {3,   0,  -0.4}; 
float           CH4Curve[3]     =  {3.3, 0,  -0.38}; 
float           Ro              =  10;                 //Ro is initialized to 10 kilo ohms


unsigned long SLEEP_TIME = 30; // Sleep time between reads (in seconds)
//VARIABLES
//float Ro = 10000.0;    // this has to be tuned 10K Ohm
float Ro0 = 4.340;    //5.28 this has to be tuned 10K Ohm
float Ro1 = 1.755;    //36.94 this has to be tuned 10K Ohm
float Ro2 = 2.501;    //3.61 this has to be tuned 10K Ohm
float Ro3 = 2.511;    //0.04 this has to be tuned 10K Ohm
int val = 0;           // variable to store the value coming from the sensor
float valAIQ0 =0.0;
float lastAIQ0 =0.0;
float valAIQ1 =0.0;
float lastAIQ1 =0.0;
float valAIQ2 =0.0;
float lastAIQ2 =0.0;
float valAIQ3 =0.0;
float lastAIQ3 =0.0;

Sleep sleep;
Sensor gw;

#define CHILD_ID_MQ2 0
#define CHILD_ID_MQ6 1
#define CHILD_ID_MQ131 2
#define CHILD_ID_TGS2600 3


void setup()  
{ 
Serial.begin(115200);

  Serial.print("Ro -->\n    MQ2:"); 
  Ro0 = MQCalibration(MQ2_SENSOR_ANALOG_PIN);
  Serial.println(Ro0);
  Serial.print("    MQ6:"); 
  Ro1 = MQCalibration(MQ6_SENSOR_ANALOG_PIN);
  Serial.println(Ro1);
  Serial.print("    MQ131:"); 
  Ro2 = MQCalibration(MQ131_SENSOR_ANALOG_PIN);
  Serial.println(Ro2);
  Serial.print("    TGZS2600:"); 
  Ro3 = MQCalibration(TGS2600_SENSOR_ANALOG_PIN);
  Serial.println(Ro3);

}

// get CO ppm
float get_CO (float ratio){
	float ppm = 0.0;
	ppm = 37143 * pow (ratio, -3.178);
	return ppm;
}

void loop()      
{     
   //MQ2 CO LPG Smoke
   Serial.print("MQ2    :"); 
   Serial.print("LPG   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR_ANALOG_PIN)/Ro0,GAS_LPG) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("CO    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR_ANALOG_PIN)/Ro0,GAS_CO_sec) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("SMOKE :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR_ANALOG_PIN)/Ro0,GAS_Smoke) );
   Serial.print( "ppm" );
   Serial.print("\n");
   //MQ6
   Serial.print("MQ6    :"); 
   Serial.print("LPG   :");
   Serial.print(MQGetGasPercentage(MQRead(MQ6_SENSOR_ANALOG_PIN)/Ro1,GAS_LPG_sec) );
   Serial.print( "ppm" );
   Serial.print("    "); 
   Serial.print("CH4   :");
   Serial.print(MQGetGasPercentage(MQRead(MQ6_SENSOR_ANALOG_PIN)/Ro1,GAS_CH4) );
   Serial.print( "ppm" );
   Serial.print("\n");
   //MQ131 CL2 O3
   Serial.print("MQ131  :"); 
   Serial.print("CL2   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ131_SENSOR_ANALOG_PIN)/Ro2,GAS_CL2) );
   Serial.print( "ppm" );
      Serial.print("    ");   
   Serial.print("O3    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ131_SENSOR_ANALOG_PIN)/Ro2,GAS_O3) );
   Serial.print( "ppm" );
   Serial.print("\n");
   //TGS2600 H2 C2H5OH C4H10   
   Serial.print("TGS2600:"); 
   Serial.print("H2    :"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR_ANALOG_PIN)/Ro3,GAS_H2) );
   Serial.print( "ppm" );
      Serial.print("    ");   
   Serial.print("C2H5OH:"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR_ANALOG_PIN)/Ro3,GAS_C2H5OH) );
   Serial.print( "ppm" );
      Serial.print("    ");   
   Serial.print("C4H10 :"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR_ANALOG_PIN)/Ro3,GAS_C4H10) );
   Serial.print( "ppm" );
   Serial.print("\n");    


  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(10000); //delay to allow serial to fully print before sleep
  //sleep.pwrDownMode(); //set sleep mode
  //sleep.sleepDelay(SLEEP_TIME * 1000); //sleep for: sleepTime 
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
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_CL2 ) {
     return MQGetPercentage(rs_ro_ratio,CL2Curve);
  } else if ( gas_id == GAS_O3 ) {
     return MQGetPercentage(rs_ro_ratio,O3Curve);
  } else if ( gas_id == GAS_CO2 ) {
     return MQGetPercentage(rs_ro_ratio,CO2Curve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_NH4 ) {
     return MQGetPercentage(rs_ro_ratio,NH4Curve);
  } else if ( gas_id == GAS_CO2H50H ) {
     return MQGetPercentage(rs_ro_ratio,CO2H50HCurve);
  } else if ( gas_id == GAS_CH3 ) {
     return MQGetPercentage(rs_ro_ratio,CH3Curve);
  } else if ( gas_id == GAS_CH3_2CO ) {
     return MQGetPercentage(rs_ro_ratio,CH3_2COCurve);
  } else if ( gas_id == GAS_H2 ) {
     return MQGetPercentage(rs_ro_ratio,H2Curve);
  } else if ( gas_id == GAS_C2H5OH ) {
     return MQGetPercentage(rs_ro_ratio,C2H5OHCurve);
  } else if ( gas_id == GAS_C4H10 ) {
     return MQGetPercentage(rs_ro_ratio,C4H10Curve);
  } else if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_Smoke ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  } else if ( gas_id == GAS_CO_sec ) {
    return MQGetPercentage(rs_ro_ratio,CO_secCurve);
  } else if ( gas_id == GAS_LPG_sec ) {
    return MQGetPercentage(rs_ro_ratio,LPG_secCurve);
  } else if ( gas_id == GAS_CH4 ) {
    return MQGetPercentage(rs_ro_ratio,CH4Curve);
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
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
