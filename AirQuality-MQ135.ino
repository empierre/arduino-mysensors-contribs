/*
  Arduino MQ135

  connect the sensor as follows for raw sensor :

  A H A   >>> 5V
  B       >>> A0
  H       >>> GND
  B       >>> 10K ohm >>> GND

  connect the sensor as follows if on board :

  Vcc     >>> 5V
  Gnd     >>> Gnd
  Aout    >>> A0 
 
  Contribution: epierre
  Based on David Gironi http://davidegironi.blogspot.fr/2014/01/cheap-co2-meter-using-mq135-sensor-with.html
  http://skylink.dl.sourceforge.net/project/davidegironi/avr-lib/avr_lib_mq135_01.zip
 
*/

#include <SPI.h>
#include <MySensor.h>  
#include <Wire.h> 

#define CHILD_ID_AIQ 0
#define AIQ_SENSOR_ANALOG_PIN 0

#define MQ135_DEFAULTPPM 399 //default ppm of CO2 for calibration
#define MQ135_DEFAULTRO 68550 //default Ro for MQ135_DEFAULTPPM ppm of CO2
#define MQ135_SCALINGFACTOR 116.6020682 //CO2 gas value
#define MQ135_EXPONENT -2.769034857 //CO2 gas value
#define MQ135_MAXRSRO 2.428 //for CO2
#define MQ135_MINRSRO 0.358 //for CO2

unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in seconds)
//VARIABLES
double mq135_ro = 10000;    // this has to be tuned 10K Ohm
double val = 0;                 // variable to store the value coming from the sensor
double valAIQ =0;
double lastAIQ =0;

MySensor gw;
MyMessage msg(CHILD_ID_AIQ, V_LEVEL);

void setup()  
{
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("AIQ Sensor MQ135", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_AIQ, S_AIR_QUALITY);  
   
}

/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
long mq135_getro(long resvalue, double ppm) {
return (long)(resvalue * exp( log(MQ135_SCALINGFACTOR/ppm) / MQ135_EXPONENT ));
}

/*
 * get the ppm concentration
 */
double mq135_getppm(long resvalue, long ro) {
double ret = 0;
double validinterval = 0;
validinterval = resvalue/(double)ro;
if(validinterval<MQ135_MAXRSRO && validinterval>MQ135_MINRSRO) {
ret = (double)((double)MQ135_SCALINGFACTOR * pow( ((double)resvalue/ro), MQ135_EXPONENT));
}
return ret;
}

void loop()      
{    
  double valr = analogRead(AIQ_SENSOR_ANALOG_PIN);// Get AIQ value
  Serial.println(val);
  double val =  ((float)22000*(1023-valr)/valr); 
  //during clean air calibration, read the Ro value and replace MQ135_DEFAULTRO value with it, you can even deactivate following function call.
  mq135_ro = mq135_getro(val, MQ135_DEFAULTPPM);
  //convert to ppm (using default ro)
  valAIQ = mq135_getppm(val, MQ135_DEFAULTRO);

  Serial.print ( "Val / Ro / value:");
  Serial.print ( val);
  Serial.print ( " / ");
  Serial.print ( mq135_ro);
  Serial.print ( " / ");
  Serial.print ( valAIQ);

 
  if (valAIQ != lastAIQ) {
      gw.send(msg.set(MQ135_DEFAULTPPM+(int)ceil(valAIQ)));
      lastAIQ = ceil(valAIQ);
  }
 
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  gw.sleep(SLEEP_TIME); //sleep for: sleepTime
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
