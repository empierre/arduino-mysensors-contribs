 /*
  Arduino Multiple Air Quality Sensors

  connect the sensor as follows when standalone:

  A H A   >>> 5V
  B	  >>> A0
  H       >>> GND
  B       >>> 10K/20K ohm >>> GND
  
  Contribution: epierre
  Based on David Gironi http://davidegironi.blogspot.fr/2014/01/cheap-co2-meter-using-mq135-sensor-with.html
  
  Precaution:
     The gasses detected by these gas sensors can be deadly in high concentrations. Always be careful to perform gas tests in well ventilated areas.
 
  Note:
     THESE GAS SENSOR MODULES ARE NOT DESIGNED FOR OR APPROVED FOR ANY APPLICATION INVOLVING HEALTH OR HUMAN SAFETY. THESE GAS SENSOR MODULES ARE FOR EXPERIMENTAL PURPOSES ONLY.

  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
*/

#include <SPI.h>
#include <MySensor.h>  
#include <Wire.h> 
#include <DHT.h>  
#include <Adafruit_BMP085.h>


/************************Hardware Related Macros************************************/
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
float           H2S_secCurve[2] =  {,}; //MQ136 http://www.sensorica.ru/pdf/MQ-136.pdf
float           NH4_secCurve[2] =  {,};   //MQ136 http://www.sensorica.ru/pdf/MQ-136.pdf
float           NHEX_Curve[2]  =  {2142.297846, -2.751369226};   //MQ138 (1.8,200) (0.8,1000) (0.28,10000)
float           C6H6_Curve[2]  =  {2142.297846, -2.751369226};   //MQ138 (2.1,200) (1,1000) (0.32,10000)
float           C3H8_Curve[2]  =  {2142.297846, -2.751369226};   //MQ138 (1.8,200) (0.8,1000) (0.28,10000)
float           C2H5OH_terCurve[2] =  {2142.297846, -2.751369226};//MQ138 (3,200) (1.8,1000) (0.7,10000)
float           CH4_terCurve[2] =  {2142.297846, -2.751369226};   //MQ138 (3,200) (1.8,1000) (0.7,10000)
float           C2H5OH_secCurve[2] =  {0.2995093465,-3.148170562};//TGS2600
float           C4H10Curve[2]   =  {0.3555567714, -3.337882361}; //TGS2600
float           H2_terCurve[2]  =  {0.3417050674, -2.887154835}; //TGS2600
float           C7H8Curve[2]    =  {0.1319857248,   -1.69516241}; //TGS2602     (0.3;1)( 0.08;10) (0.04;30)
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
float RL0 = 2.897;    //MQ2     Elecfreacks Octopus
float Ro1 = 1.755;    //MQ6     25.76 this has to be tuned 10K Ohm
float RL1 = 0.993;    //MQ6     Gas Sensor V1.3 auto-ctrl.com 
float Ro2 = 2.501;    //MQ131   2.24 this has to be tuned 10K Ohm
float RL2 = 0.679;    //MQ131   Sainsmart
float Ro3 = 2.511;    //TGS2600 0.05 this has to be tuned 10K Ohm
float RL3 = 0.893;    //TGS2600 Sainsmart
float Ro4 = 2.511;    //MQ135   2.51 this has to be tuned 10K Ohm
float RL4 = 0.990;    //MQ135   FC-22
float Ro5 = 2.511;    //2SH12   
float RL5 = 4000;     //2SH12   MQ-XL-V2 auto-ctrl.com 
float Ro6 = 2.511;    //TGS2602 0.05 this has to be tuned 10K Ohm
float RL6 = 0.893;    //TGmq136S2602 Gas Sensor V1.3 auto-ctrl.com 
int val = 0;          // variable to store the value coming from the sensor


float calcVoltage = 0;
float dustDensity = 0;
boolean metric = true; 
//DHT11
float lastTemp;
float lastHum;
//BMP085
float lastPressure = -1;
int lastForecast = -1;
char *weather[]={"stable","sunny","cloudy","unstable","thunderstorm","unknown"};
int minutes;
float pressureSamples[180];
int minuteCount = 0;
bool firstRound = true;
float pressureAvg[7];
float dP_dt;
//test
float a=0;
boolean pcReceived = false;


#define CHILD_ID_MQ2 0
#define CHILD_ID_MQ6 1
#define CHILD_ID_MQ131 2
#define CHILD_ID_TGS2600 3
#define CHILD_ID_MQ135 4
#define CHILD_ID_DUST 5
#define CHILD_ID_2SH12 6
#define CHILD_ID_HUM 7
#define CHILD_ID_TEMP 8
#define CHILD_ID_PRESSURE 9
#define CHILD_ID_FORECAST 10
#define CHILD_ID_TGS2602 11

DHT dht;
Adafruit_BMP085 bmp = Adafruit_BMP085();     // Digital Pressure Sensor 
MySensor gw(48,49);  // Arduino Mega initialization
MyMessage msg_dust(CHILD_ID_DUST, 45);      //AqPM10
MyMessage msg_mq2(CHILD_ID_MQ2, 40);        //Smoke
MyMessage pcMsg_mq2(CHILD_ID_MQ2,V_VAR1);
MyMessage msg_mq6(CHILD_ID_MQ6, 41);        //LPG
MyMessage pcMsg_mq6(CHILD_ID_MQ6,V_VAR1);
MyMessage msg_mq131(CHILD_ID_MQ131, 42);    //Aq03
MyMessage pcMsg_mq131(CHILD_ID_MQ131,V_VAR1);
MyMessage msg_tgs2600(CHILD_ID_TGS2600, 43);//AqH2
MyMessage pcMsg_tgs2600(CHILD_ID_TGS2600,V_VAR1);
MyMessage msg_mq135(CHILD_ID_MQ135, 44);    //AqCO
MyMessage pcMsg_mq135(CHILD_ID_MQ135,V_VAR1);
MyMessage msg_2sh12(CHILD_ID_2SH12, 46);    //AqSO2
MyMessage pcMsg_2sh12(CHILD_ID_2SH12,V_VAR1);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage pressureMsg(CHILD_ID_PRESSURE, V_PRESSURE);
MyMessage forecastMsg(CHILD_ID_FORECAST, V_FORECAST);
MyMessage msg_tgs2602(CHILD_ID_TGS2602, 47);//AqH2
MyMessage pcMsg_tgs2602(CHILD_ID_TGS2602,V_VAR1);

void setup()  
{ 
  gw.begin(incomingMessage);
  
/*  gw.request(CHILD_ID_MQ2, V_VAR1);
  gw.request(CHILD_ID_MQ6, V_VAR1);
  gw.request(CHILD_ID_MQ131, V_VAR1);
  gw.request(CHILD_ID_TGS2600, V_VAR1);
  gw.request(CHILD_ID_MQ135, V_VAR1);
  gw.request(CHILD_ID_2SH12, V_VAR1);  
  gw.request(CHILD_ID_TGS2602, V_VAR1);*/
  
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) { }
  }
  
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ Multi Sensors", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_MQ2, S_AIR_QUALITY);  
  gw.present(CHILD_ID_MQ6, S_AIR_QUALITY);  
  gw.present(CHILD_ID_MQ131, S_AIR_QUALITY);  
  gw.present(CHILD_ID_TGS2600, S_AIR_QUALITY);  
  gw.present(CHILD_ID_MQ135, S_AIR_QUALITY);  
  gw.present(CHILD_ID_DUST, S_AIR_QUALITY);  
  gw.present(CHILD_ID_2SH12, S_AIR_QUALITY);  
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_PRESSURE, S_BARO);
  gw.present(CHILD_ID_TGS2602, S_AIR_QUALITY);  
  
  metric = gw.getConfig().isMetric;
  
//  delay(50*1000); //delay to allow serial to fully print before sleep
  Serial.print("Ro -->\n    MQ2:"); 
  Ro0 = MQCalibration(MQ2_SENSOR,10,RL0,SmokeCurve);
  Serial.println(Ro0);
  gw.send(pcMsg_mq2.set((long int)ceil(Ro0)));
  Serial.print("    MQ6:"); 
  Ro1 = MQCalibration(MQ6_SENSOR,10,RL1,LPGCurve);
  Serial.println(Ro1);
  gw.send(pcMsg_mq6.set((long int)ceil(Ro1)));
  Serial.print("    MQ131:"); 
  Ro2 = MQCalibration(MQ131_SENSOR,10,RL2,O3Curve);
  Serial.println(Ro2);
  gw.send(pcMsg_mq131.set((long int)ceil(Ro2)));
  Serial.print("    TGS2600:"); 
  Ro3 = MQCalibration(TGS2600_SENSOR,10,RL3,C2H5OH_secCurve);
  Serial.println(Ro3);
  gw.send(pcMsg_tgs2600.set((long int)ceil(Ro3)));
  Serial.print("    MQ135:"); 
  Ro4 = MQCalibration(MQ135_SENSOR,10,RL4,CO_secCurve);
  Serial.println(Ro4);
    gw.send(pcMsg_mq135.set((long int)Ro4));
  Serial.print("    2SH12:"); 
  Ro5 = MQResistanceCalculation(analogRead(S2SH12_SENSOR),RL5);
  Serial.println(Ro5);
  gw.send(pcMsg_2sh12.set((long int)ceil(Ro5)));
  pinMode(DUST_SENSOR_DIGITAL_PIN,OUTPUT); //light on led
  Serial.print("    TGZS2602:"); 
  Ro6 = MQCalibration(TGS2602_SENSOR,1,RL6,C7H8Curve);
  Serial.println(Ro6);
  gw.send(pcMsg_tgs2602.set((long int)ceil(Ro6)));

}


void loop()      
{     
   //DHT11 Temp+Hum
  delay(dht.getMinimumSamplingPeriod());
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }
  
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);
  }
  
  //BMP085 Pressure
  float pressure = bmp.readPressure()/100;
  float altitude = bmp.readAltitude();
  if (!metric) {
    // Convert to fahrenheit
    temperature = temperature * 9.0 / 5.0 + 32.0;
  }
  int forecast = sample(pressure);
  if (pressure != lastPressure) {
    gw.send(pressureMsg.set(pressure,0));
    lastPressure = pressure;
  }

  if (forecast != lastForecast) {
    gw.send(forecastMsg.set(weather[forecast]));
    lastForecast = forecast;
  }
  
   //MQ2 CO LPG Smoke
   Serial.print("MQ2    :"); 
   Serial.print("LPG   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR,RL0),Ro0,GAS_LPG,MQ2_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("CO    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR,RL0),Ro0,GAS_CO_sec,MQ2_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("SMOKE :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ2_SENSOR,RL0),Ro0,GAS_Smoke,MQ2_SENSOR) );
            gw.send(msg_mq2.set((int)ceil(MQGetGasPercentage(MQRead(MQ2_SENSOR,RL0),Ro0,GAS_Smoke,MQ2_SENSOR))));
   Serial.print( "ppm" );
   Serial.print("\n");   
   //MQ6
   Serial.print("MQ6    :"); 
   Serial.print("LPG   :");
   Serial.print(MQGetGasPercentage(MQRead(MQ6_SENSOR,RL1),Ro1,GAS_LPG_sec,MQ6_SENSOR) );
            gw.send(msg_mq6.set((int)ceil(MQGetGasPercentage(MQRead(MQ6_SENSOR,RL1),Ro1,GAS_LPG,MQ6_SENSOR))));
   Serial.print( "ppm" );
   Serial.print("    "); 
   Serial.print("CH4   :");
   Serial.print(MQGetGasPercentage(MQRead(MQ6_SENSOR,RL1),Ro1,GAS_CH4,MQ6_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("\n");
   //MQ131 CL2 O3
   Serial.print("MQ131  :"); 
//      Serial.print(analogRead(MQ131_SENSOR));
   Serial.print("CL2   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ131_SENSOR,RL2),Ro2,GAS_CL2,MQ131_SENSOR) );
   Serial.print( "ppm" );
      Serial.print("    ");   
   Serial.print("O3    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ131_SENSOR,RL2),Ro2,GAS_O3,MQ131_SENSOR) );
         gw.send(msg_mq131.set((int)ceil(MQGetGasPercentage(MQRead(MQ131_SENSOR,RL2),Ro2,GAS_O3,MQ131_SENSOR))));
   Serial.print( "ppm" );
   Serial.print("\n");
   //TGS2600 H2 C2H5OH C4H10   
   Serial.print("TGS2600:"); 
   Serial.print("H2    :"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR,RL3),Ro3,GAS_H2,TGS2600_SENSOR) );
         gw.send(msg_tgs2600.set((int)ceil(MQGetGasPercentage(MQRead(TGS2600_SENSOR,RL3),Ro3,GAS_H2,TGS2600_SENSOR))));
   Serial.print( "ppm" );
      Serial.print("    ");   
   Serial.print("C2H5OH:"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR,RL3),Ro3,GAS_C2H5OH,TGS2600_SENSOR) );
   Serial.print( "ppm" );
      Serial.print(" ");   
   Serial.print("C4H10 :"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2600_SENSOR,RL3),Ro3,GAS_C4H10,TGS2600_SENSOR) );
   Serial.print( "ppm" );
   Serial.print("\n");    
   //MQ135  CO NH4 CH3 CO2
   Serial.print("MQ135  :"); 
   Serial.print("CO2   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR,RL4),Ro4,GAS_CO2,MQ135_SENSOR) );
   Serial.print( "ppm" );      
      Serial.print("    ");   
   Serial.print("CO    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR,RL4),Ro4,GAS_CO,MQ135_SENSOR) );
   Serial.print( "ppm" );      
   gw.send(msg_mq135.set((int)ceil(MQGetGasPercentage(MQRead(MQ135_SENSOR,RL4),Ro4,GAS_CO,MQ135_SENSOR))));
      Serial.print("     ");   
   Serial.print("CH3   :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR,RL4),Ro4,GAS_CH3,MQ135_SENSOR) );
   Serial.print( "ppm" );      
      Serial.print("    ");   
   Serial.print("NH4    :"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ135_SENSOR,RL4),Ro4,GAS_NH4,MQ135_SENSOR) );
   Serial.print( "ppm" );      
   Serial.print("\n");  
 //2SH12
   Serial.print("2SH12  :"); 
   Serial.print("SO2   :"); 
   a=analogRead(S2SH12_SENSOR);
   Serial.print(a);   Serial.print( " raw " );      
   Serial.print("\n");  
    gw.send(msg_2sh12.set((int)ceil(analogRead(S2SH12_SENSOR))));
 //TGS2602 C7H8
   Serial.print("TGS2602:"); 
   Serial.print("C7H8  :"); 
   Serial.print(MQGetGasPercentage(MQRead(TGS2602_SENSOR,RL6),Ro6,GAS_C7H8,TGS2602_SENSOR) );
         gw.send(msg_tgs2602.set((int)ceil(MQGetGasPercentage(MQRead(TGS2602_SENSOR,RL6),Ro6,GAS_C7H8,TGS2602_SENSOR))));
   Serial.print( "ppm" );
   Serial.print("\n");  
   
  digitalWrite(DUST_SENSOR_DIGITAL_PIN,LOW); // power on the LED
  delayMicroseconds(280);
  uint16_t voMeasured = analogRead(DUST_SENSOR_ANALOG_PIN);// Get DUST value
  delayMicroseconds(40);
  digitalWrite(DUST_SENSOR_DIGITAL_PIN,HIGH); // turn the LED off
  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);  // Adapt to device voltage
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
  
   gw.send(msg_dust.set((int)ceil(dustDensity)));
   
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(SLEEP_TIME * 1000); //delay to allow serial to fully print before sleep
  //gw.powerDown();
  //sleep.pwrDownMode(); //set sleep mode
  //gw.sleep(SLEEP_TIME * 1000); //sleep for: sleepTime 
}




/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage across the load resistor and its resistance, the resistance of the sensor could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc,float rl_value)
{
  return  (long)((long)(1024*1000*(long)rl_value)/raw_adc-(long)rl_value);
;
}
 
/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air.        .
************************************************************************************/ 
float MQCalibration(int mq_pin, double ppm, double rl_value,float *pcurve )
{
  int i;
  float val=0;

  for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin),rl_value);
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBRATION_SAMPLE_TIMES;                   //calculate the average value
  //Ro = Rs * sqrt(a/ppm, b) = Rs * exp( ln(a/ppm) / b )

  return  (long)val*exp((log(pcurve[0]/ppm)/pcurve[1]));

}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin,float rl_value)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin),rl_value);
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
    } else if ( gas_id == GAS_C2H5OH ) {
     return MQGetPercentage(rs_ro_ratio,ro,C2H50H_Curve); //MQ135
    } else if ( gas_id == GAS_CH3 ) {
     return MQGetPercentage(rs_ro_ratio,ro,CH3Curve);     //MQ135
    } else if ( gas_id == GAS_CH3_2CO ) {
     return MQGetPercentage(rs_ro_ratio,ro,CH3_2COCurve); //MQ135
    } else if ( gas_id == GAS_CO_sec ) {
     return MQGetPercentage(rs_ro_ratio,ro,CO_secCurve);  //MQ135
    }
  } else if (sensor_id == MQ136_SENSOR ){
    if ( gas_id == GAS_SO2 ) {
     return MQGetPercentage(rs_ro_ratio,ro,SO2_Curve);     //MQ136
    } else if ( gas_id == GAS_CH4 ) {
     return MQGetPercentage(rs_ro_ratio,ro,CH4_secCurve);  //MQ136
    } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,ro,CO_terCurve);   //MQ136
    }
   } else if (sensor_id == MQ138_SENSOR ){
    if ( gas_id == GAS_C6H6 ) {
     return MQGetPercentage(rs_ro_ratio,ro,C6H6_Curve);    //MQ138
    } else if ( gas_id == GAS_CH4 ) {
     return MQGetPercentage(rs_ro_ratio,ro,CH4_terCurve);  //MQ138
    } else if ( gas_id == GAS_C3H8 ) {
     return MQGetPercentage(rs_ro_ratio,ro,C3H8_Curve); //MQ138
    } else if ( gas_id == GAS_NHEX ) {
     return MQGetPercentage(rs_ro_ratio,ro,NHEX_Curve); //MQ138
    }
  } else if (sensor_id == TGS2600_SENSOR ){
    if ( gas_id == GAS_C2H5OH ) {
      return MQGetPercentage(rs_ro_ratio,ro,C2H5OH_secCurve);  //TGS2600
    } else if ( gas_id == GAS_C4H10 ) {
       return MQGetPercentage(rs_ro_ratio,ro,C4H10Curve);   //TGS2600
    } else if ( gas_id == GAS_H2 ) {
       return MQGetPercentage(rs_ro_ratio,ro,H2_terCurve);  //TGS2600
    }    
   } else if (sensor_id == TGS2602_SENSOR ){
    if ( gas_id == GAS_C7H8 ) {
      return MQGetPercentage(rs_ro_ratio,ro,C7H8Curve);  //TGS2602
    } else if ( gas_id == GAS_H2S ) {
      return MQGetPercentage(rs_ro_ratio,ro,H2S_Curve);  //TGS2602
    } else if ( gas_id == GAS_NH3 ) {
      return MQGetPercentage(rs_ro_ratio,ro,NH3_Curve);  //TGS2602
    } else if ( gas_id == GAS_C2H5OH ) {
      return MQGetPercentage(rs_ro_ratio,ro,C2H5OH_quarCurve);  //TGS2602
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
  return (double)(pcurve[0] * pow(((double)rs_ro_ratio/ro), pcurve[1]));
}

/**********************************  sample  ***************************************
Input:   pressure 
Output:  an int containing the weather based on pressure
************************************************************************************/ 
int sample(float pressure) {
	// Algorithm found here
	// http://www.freescale.com/files/sensors/doc/app_note/AN3914.pdf
	if (minuteCount > 180)
		minuteCount = 6;

	pressureSamples[minuteCount] = pressure;
	minuteCount++;

	if (minuteCount == 5) {
		// Avg pressure in first 5 min, value averaged from 0 to 5 min.
		pressureAvg[0] = ((pressureSamples[1] + pressureSamples[2]
				+ pressureSamples[3] + pressureSamples[4] + pressureSamples[5])
				/ 5);
	} else if (minuteCount == 35) {
		// Avg pressure in 30 min, value averaged from 0 to 5 min.
		pressureAvg[1] = ((pressureSamples[30] + pressureSamples[31]
				+ pressureSamples[32] + pressureSamples[33]
				+ pressureSamples[34]) / 5);
		float change = (pressureAvg[1] - pressureAvg[0]);
		if (firstRound) // first time initial 3 hour
			dP_dt = ((65.0 / 1023.0) * 2 * change); // note this is for t = 0.5hour
		else
			dP_dt = (((65.0 / 1023.0) * change) / 1.5); // divide by 1.5 as this is the difference in time from 0 value.
	} else if (minuteCount == 60) {
		// Avg pressure at end of the hour, value averaged from 0 to 5 min.
		pressureAvg[2] = ((pressureSamples[55] + pressureSamples[56]
				+ pressureSamples[57] + pressureSamples[58]
				+ pressureSamples[59]) / 5);
		float change = (pressureAvg[2] - pressureAvg[0]);
		if (firstRound) //first time initial 3 hour
			dP_dt = ((65.0 / 1023.0) * change); //note this is for t = 1 hour
		else
			dP_dt = (((65.0 / 1023.0) * change) / 2); //divide by 2 as this is the difference in time from 0 value
	} else if (minuteCount == 95) {
		// Avg pressure at end of the hour, value averaged from 0 to 5 min.
		pressureAvg[3] = ((pressureSamples[90] + pressureSamples[91]
				+ pressureSamples[92] + pressureSamples[93]
				+ pressureSamples[94]) / 5);
		float change = (pressureAvg[3] - pressureAvg[0]);
		if (firstRound) // first time initial 3 hour
			dP_dt = (((65.0 / 1023.0) * change) / 1.5); // note this is for t = 1.5 hour
		else
			dP_dt = (((65.0 / 1023.0) * change) / 2.5); // divide by 2.5 as this is the difference in time from 0 value
	} else if (minuteCount == 120) {
		// Avg pressure at end of the hour, value averaged from 0 to 5 min.
		pressureAvg[4] = ((pressureSamples[115] + pressureSamples[116]
				+ pressureSamples[117] + pressureSamples[118]
				+ pressureSamples[119]) / 5);
		float change = (pressureAvg[4] - pressureAvg[0]);
		if (firstRound) // first time initial 3 hour
			dP_dt = (((65.0 / 1023.0) * change) / 2); // note this is for t = 2 hour
		else
			dP_dt = (((65.0 / 1023.0) * change) / 3); // divide by 3 as this is the difference in time from 0 value
	} else if (minuteCount == 155) {
		// Avg pressure at end of the hour, value averaged from 0 to 5 min.
		pressureAvg[5] = ((pressureSamples[150] + pressureSamples[151]
				+ pressureSamples[152] + pressureSamples[153]
				+ pressureSamples[154]) / 5);
		float change = (pressureAvg[5] - pressureAvg[0]);
		if (firstRound) // first time initial 3 hour
			dP_dt = (((65.0 / 1023.0) * change) / 2.5); // note this is for t = 2.5 hour
		else
			dP_dt = (((65.0 / 1023.0) * change) / 3.5); // divide by 3.5 as this is the difference in time from 0 value
	} else if (minuteCount == 180) {
		// Avg pressure at end of the hour, value averaged from 0 to 5 min.
		pressureAvg[6] = ((pressureSamples[175] + pressureSamples[176]
				+ pressureSamples[177] + pressureSamples[178]
				+ pressureSamples[179]) / 5);
		float change = (pressureAvg[6] - pressureAvg[0]);
		if (firstRound) // first time initial 3 hour
			dP_dt = (((65.0 / 1023.0) * change) / 3); // note this is for t = 3 hour
		else
			dP_dt = (((65.0 / 1023.0) * change) / 4); // divide by 4 as this is the difference in time from 0 value
		pressureAvg[0] = pressureAvg[5]; // Equating the pressure at 0 to the pressure at 2 hour after 3 hours have past.
		firstRound = false; // flag to let you know that this is on the past 3 hour mark. Initialized to 0 outside main loop.
	}

	if (minuteCount < 35 && firstRound) //if time is less than 35 min on the first 3 hour interval.
		return 5; // Unknown, more time needed
	else if (dP_dt < (-0.25))
		return 4; // Quickly falling LP, Thunderstorm, not stable
	else if (dP_dt > 0.25)
		return 3; // Quickly rising HP, not stable weather
	else if ((dP_dt > (-0.25)) && (dP_dt < (-0.05)))
		return 2; // Slowly falling Low Pressure System, stable rainy weather
	else if ((dP_dt > 0.05) && (dP_dt < 0.25))
		return 1; // Slowly rising HP stable good weather
	else if ((dP_dt > (-0.05)) && (dP_dt < 0.05))
		return 0; // Stable weather
	else
		return 5; // Unknown
}

void incomingMessage(const MyMessage &message) {
  if (message.type==V_VAR1) {  
    long int pulseCount = message.getLong();
    Serial.print("Received last pulse count from gw:");
    Serial.println(pulseCount);
    pcReceived = true;
  }
}
