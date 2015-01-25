#include <SPI.h>
#include <MySensor.h>  

#define ANALOG_INPUT_LEAFWETNESS_SENSOR 0   // Digital input did you attach your soil sensor.  
#define CHILD_ID 0   // Id of the sensor child

MySensor gw;
unsigned long SLEEP_TIME = 30*1000;  // sleep time between reads (seconds * 1000 milliseconds)
MyMessage msg(CHILD_ID, V_TRIPPED);
int lastSoilValue = -1;

void setup()  
{ 
 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Leaf Wetness Sensor", "1.0");
  // Register all sensors to gw (they will be created as child devices)  
  gw.present(CHILD_ID, S_MOTION);
}
 
void loop()     
{     

   
  // Read digital soil value
  int soilValue = ((float)analogRead(ANALOG_INPUT_LEAFWETNESS_SENSOR)*100/1023);  
  if (soilValue != lastSoilValue) {
    Serial.println(soilValue);
    gw.send(msg.set(soilValue));  // Send the inverse to gw as tripped should be when no water in soil
    lastSoilValue = soilValue;
  }
  // Power down the radio
  gw.sleep(SLEEP_TIME);
}


