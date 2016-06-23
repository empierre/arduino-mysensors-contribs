/************************Hardware Related Macros************************************/
#define         MQ8_SENSOR                   (0)  //define which analog input channel you are going to use
#define         MQ2_SENSOR                   (0)  //define which analog input channel you are going to use
#define         MQ6_SENSOR                   (1)
#define         MQ131_SENSOR                 (2)
#define         TGS2600_SENSOR               (3)
#define         MQ135_SENSOR                 (4)
#define         S2SH12_SENSOR                (15)
#define         DUST_SENSOR_ANALOG_PIN       (11)
#define         DUST_SENSOR_DIGITAL_PIN      (13)
#define         HUMIDITY_SENSOR_DIGITAL_PIN  (6)
#define         MQ136_SENSOR                 (7)
#define         MQ138_SENSOR                 (8)
#define         TGS2602_SENSOR               (14)
#define         HCHO_SENSOR                  (16)
#define         MS2610_SENSOR                (17)
#define         PRESSURE_SENSOR_DIGITAL_PIN  (14)
#define         RL_VALUE                     (990) //define the load resistance on the board, in ohms
/***********************Software Related Macros************************************/
#define         CALIBRATION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
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
float           COCurve[2]      =  {37793.94418, -3.24294658};   //MQ2
float           H2Curve[2]      =  {957.1355042, -2.07442628};   //MQ2
float           LPGCurve[2]     =  {591.6128784, -1.679699732};  //MQ2
float           SmokeCurve[2]   =  {3426.376355, -2.225037973};  //MQ2
float           LPG_secCurve[2] =  {1051.200149, -2.434978052};  //MQ6
float           CH4Curve[2]     =  {1081.498208, -1.443059209};  //MQ6
float           H2_secCurve[2]  =  {137997.7173, -3.76632598};   //MQ6
float           H2sec_Curve[2]  =  {957.1355042, -2.07442628};   //MQ8 -- To be adapted !!!
float           CL2Curve[2]     =  {56.01727602, -1.359048399};  //MQ131
float           O3Curve[2]      =  {42.84561841, -1.043297135};  //MQ131
float           O3_secCurve[2]  =  {45.34696335, 1.743219536};   //MS2610
float           CO2Curve[2]     =  {113.7105289, -3.019713765};  //MQ135
float           CO_secCurve[2]  =  {726.7809737, -4.040111669};  //MQ135
float           NH4Curve[2]     =  {84.07117895, -4.41107687};   //MQ135
float           C2H50H_Curve[2] =  {74.77989144, 3.010328075};   //MQ135 
float           CH3Curve[2]     =  {47.01770503, -3.281901967};  //MQ135
float           CH3_2COCurve[2] =  {7.010800878, -2.122018939};  //MQ135
float           SO2_Curve[2]    =  {40.44109566, -1.085728557};  //MQ136 http://china-total.com/product/meter/gas-sensor/MQ136.pdf
float           CH4_secCurve[2] =  {57.82777729, -1.187494933};  //MQ136 http://china-total.com/product/meter/gas-sensor/MQ136.pdf
float           CO_terCurve[2]  =  {2142.297846, -2.751369226};  //MQ136 http://china-total.com/product/meter/gas-sensor/MQ136.pdf
float           NHEX_Curve[2]  =  {2142.297846, -2.751369226};   //MQ138 (1.8,200) (0.8,1000) (0.28,10000)
float           C6H6_Curve[2]  =  {2142.297846, -2.751369226};   //MQ138 (2.1,200) (1,1000) (0.32,10000)
float           C3H8_Curve[2]  =  {2142.297846, -2.751369226};   //MQ138 (1.8,200) (0.8,1000) (0.28,10000)
float           C2H5OH_terCurve[2] =  {2142.297846, -2.751369226};//MQ138 (3,200) (1.8,1000) (0.7,10000)
float           CH4_terCurve[2] =  {2142.297846, -2.751369226};   //MQ138 (3,200) (1.8,1000) (0.7,10000)
float           C2H5OH_secCurve[2] =  {0.2995093465,-3.148170562};//TGS2600
float           C4H10Curve[2]   =  {0.3555567714, -3.337882361}; //TGS2600
float           H2_terCurve[2]  =  {0.3417050674, -2.887154835}; //TGS2600
float           C7H8Curve[2]    =  {37.22590719,   2.078062258}; //TGS2602     (0.3;1)( 0.8;10) (0.4;30)
float           H2S_Curve[2]    =  {0.05566582614,-2.954075758}; //TGS2602     (0.8,0.1) (0.4,1) (0.25,3)
float           C2H5OH_quarCurve[2] =  {0.5409499131,-2.312489623};//TGS2602  (0.75,1) (0.3,10) (0.17,30)  
float           NH3_Curve[2]    =  {0.585030495,  -3.448654502  }; //TGS2602   (0.8,1) (0.5,10) (0.3,30) 
float           HCHO_Curve[2]   =  {1.478772974,  -2.224808489  }; //HCHO      (0.59,5) (0.41,10) (0.23,40) 
float           H2_quaCurve[2]  =  {2.452065204,-2.282530712};     //HCHO      (0.68,5) (0.59,10) (0.29,40) 
float           C7H8_secCurve[2]=  {4.798168577,   -0.8100009624}; //HCHO Toluene (0.8,5)  (0.5,10)  (0.07,40)
float           C6H6_secCurve[2]=  {5.59434996, -0.6062729607};    //HCHO benzol  (0.25,5) (0.8,10)  (0.09,40)
float           Ro              =  10000;                          //Ro is initialized to 10 kilo ohms
unsigned long SLEEP_TIME = 600; // Sleep time between reads (in seconds)
//VARIABLES
float Ro0 = 4.300;    //MQ2     3.83 this has to be tuned 10K Ohm
float RL0 = 2.897;    //MQ2     
boolean metric = true;

void setup()
{
  Ro0 = MQCalibration(MQ8_SENSOR, 0.5, RL0, H2Curve);

}
void loop()
{
  //MQ8 CO LPG Smoke
  Serial.print("MQ8    :");
  Serial.print("H2   :");
  Serial.print(MQGetGasPercentage(MQRead(MQ8_SENSOR, RL0), Ro0, GAS_H2, MQ8_SENSOR) );
  delay(SLEEP_TIME * 1000); //delay to allow serial to fully print before sleep

}



/****************** MQResistanceCalculation ****************************************
  Input:   raw_adc - raw value read from adc, which represents the voltage
  Output:  the calculated sensor resistance
  Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage across the load resistor and its resistance, the resistance of the sensor could be derived.
************************************************************************************/
float MQResistanceCalculation(int raw_adc, float rl_value)
{
  return  (long)((long)(1024 * 1000 * (long)rl_value) / raw_adc - (long)rl_value);
  ;
}

/***************************** MQCalibration ****************************************
  Input:   mq_pin - analog channel
  Output:  Ro of the sensor
  Remarks: This function assumes that the sensor is in clean air. It use
         MQResistanceCalculation to calculates the sensor resistance in clean air.        .
************************************************************************************/
float MQCalibration(int mq_pin, double ppm, double rl_value, float *pcurve )
{
  int i;
  float val = 0;

  for (i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) {      //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin), rl_value);
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val / CALIBRATION_SAMPLE_TIMES;                 //calculate the average value
  //Ro = Rs * sqrt(a/ppm, b) = Rs * exp( ln(a/ppm) / b )

  return  (long)val * exp((log(pcurve[0] / ppm) / pcurve[1]));

}
/*****************************  MQRead *********************************************
  Input:   mq_pin - analog channel
  Output:  Rs of the sensor
  Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/
float MQRead(int mq_pin, float rl_value)
{
  int i;
  float rs = 0;

  for (i = 0; i < READ_SAMPLE_TIMES; i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin), rl_value);
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs / READ_SAMPLE_TIMES;

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
      return MQGetPercentage(rs_ro_ratio, ro, COCurve);    //MQ2
    } else if ( gas_id == GAS_H2 ) {
      return MQGetPercentage(rs_ro_ratio, ro, H2Curve);    //MQ2
    } else if ( gas_id == GAS_LPG ) {
      return MQGetPercentage(rs_ro_ratio, ro, LPGCurve);   //MQ2
    } else if ( gas_id == GAS_Smoke ) {
      return MQGetPercentage(rs_ro_ratio, ro, SmokeCurve); //MQ2
    }
  } else if (sensor_id == MQ8_SENSOR ) {
    if ( gas_id == GAS_H2 ) {
      return MQGetPercentage(rs_ro_ratio, ro, H2sec_Curve); //MQ8
    }
  } else if (sensor_id == MQ6_SENSOR ) {
    if ( gas_id == GAS_LPG_sec ) {
      return MQGetPercentage(rs_ro_ratio, ro, LPG_secCurve); //MQ6
    } else if ( gas_id == GAS_CH4 ) {
      return MQGetPercentage(rs_ro_ratio, ro, CH4Curve);    //MQ6
    } else if ( gas_id == GAS_H2 ) {
      return MQGetPercentage(rs_ro_ratio, ro, H2_secCurve); //MQ6
    }
  } else if (sensor_id == MQ131_SENSOR ) {
    if ( gas_id == GAS_CL2 ) {
      return MQGetPercentage(rs_ro_ratio, ro, CL2Curve);   //MQ131
    } else if ( gas_id == GAS_O3 ) {
      return MQGetPercentage(rs_ro_ratio, ro, O3Curve);    //MQ131
    }
  } else if (sensor_id == MQ135_SENSOR ) {
    if ( gas_id == GAS_CO2 ) {
      return MQGetPercentage(rs_ro_ratio, ro, CO2Curve);   //MQ135
    } else if ( gas_id == GAS_NH4 ) {
      return MQGetPercentage(rs_ro_ratio, ro, NH4Curve);   //MQ135
    } else if ( gas_id == GAS_C2H5OH ) {
      return MQGetPercentage(rs_ro_ratio, ro, C2H50H_Curve); //MQ135
    } else if ( gas_id == GAS_CH3 ) {
      return MQGetPercentage(rs_ro_ratio, ro, CH3Curve);   //MQ135
    } else if ( gas_id == GAS_CH3_2CO ) {
      return MQGetPercentage(rs_ro_ratio, ro, CH3_2COCurve); //MQ135
    } else if ( gas_id == GAS_CO_sec ) {
      return MQGetPercentage(rs_ro_ratio, ro, CO_secCurve); //MQ135
    }
  } else if (sensor_id == MQ136_SENSOR ) {
    if ( gas_id == GAS_SO2 ) {
      return MQGetPercentage(rs_ro_ratio, ro, SO2_Curve);   //MQ136
    } else if ( gas_id == GAS_CH4 ) {
      return MQGetPercentage(rs_ro_ratio, ro, CH4_secCurve); //MQ136
    } else if ( gas_id == GAS_CO ) {
      return MQGetPercentage(rs_ro_ratio, ro, CO_terCurve); //MQ136
    }
  } else if (sensor_id == MQ138_SENSOR ) {
    if ( gas_id == GAS_C6H6 ) {
      return MQGetPercentage(rs_ro_ratio, ro, C6H6_Curve);  //MQ138
    } else if ( gas_id == GAS_CH4 ) {
      return MQGetPercentage(rs_ro_ratio, ro, CH4_terCurve); //MQ138
    } else if ( gas_id == GAS_C3H8 ) {
      return MQGetPercentage(rs_ro_ratio, ro, C3H8_Curve); //MQ138
    } else if ( gas_id == GAS_NHEX ) {
      return MQGetPercentage(rs_ro_ratio, ro, NHEX_Curve); //MQ138
    }
  } else if (sensor_id == TGS2600_SENSOR ) {
    if ( gas_id == GAS_C2H5OH ) {
      return MQGetPercentage(rs_ro_ratio, ro, C2H5OH_terCurve); //TGS2600
    } else if ( gas_id == GAS_C4H10 ) {
      return MQGetPercentage(rs_ro_ratio, ro, C4H10Curve); //TGS2600
    } else if ( gas_id == GAS_H2 ) {
      return MQGetPercentage(rs_ro_ratio, ro, H2_terCurve); //TGS2600
    }
  } else if (sensor_id == TGS2602_SENSOR ) {
    if ( gas_id == GAS_C7H8 ) {
      return MQGetPercentage(rs_ro_ratio, ro, C7H8Curve); //TGS2602
    } else if ( gas_id == GAS_H2S ) {
      return MQGetPercentage(rs_ro_ratio, ro, H2S_Curve); //TGS2602
    } else if ( gas_id == GAS_NH3 ) {
      return MQGetPercentage(rs_ro_ratio, ro, NH3_Curve); //TGS2602
    } else if ( gas_id == GAS_C2H5OH ) {
      return MQGetPercentage(rs_ro_ratio, ro, C2H5OH_quarCurve); //TGS2602
    }
  } else if (sensor_id == S2SH12_SENSOR) {
    if ( gas_id == GAS_SO2 ) {
      //return MQGetPercentage(rs_ro_ratio,ro,C2H5OHCurve);  //2SH12
      return rs_ro_ratio;
    }
  } else if (sensor_id == HCHO_SENSOR) {
    if ( gas_id == GAS_HCHO ) {
      //return MQGetPercentage(rs_ro_ratio,ro,HCHO_Curve);  //HCHO
      return rs_ro_ratio;
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
  return (double)(pcurve[0] * pow(((double)rs_ro_ratio / ro), pcurve[1]));
}
