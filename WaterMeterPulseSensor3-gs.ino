// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"


/*
 Use this sensor to measure volume and flow of your house watermeter.
 You need to set the correct pulsefactor of your meter (pulses per m3).
 The sensor starts by fetching current volume reading from gateway (VAR 1).
 Reports both volume and flow back to gateway.
 Sensor on pin analog 0
 
  Particle Photon
  Capteur TRT5000
  Watermeter sensus Residia Jet
 
 */
 
#define ANALOG_DIGITAL_SENSOR 0              // The digital input you attached your sensor. 
#define PULSE_FACTOR 1                       // Number of blinks per m3 of your meter (One rotation/liter)
#define SLEEP_MODE false                     // flowvalue can only be reported when sleep mode is false.
#define MAX_FLOW 40                          // Max flow (l/min) value to report. This filetrs outliers.
unsigned long SEND_FREQUENCY = 1;//20000;    // Minimum time between send (in seconds). We don't want to spam the gateway.

double ppl = ((double)PULSE_FACTOR);         // Pulses per liter

volatile unsigned long pulseCount = 0;   
volatile unsigned long lastBlink = 0;
volatile double flow = 0;   
boolean pcReceived = false;
unsigned long oldPulseCount = 0;
unsigned long newBlink = 0;   
double oldflow = 0;
double volume;                     
double oldvolume;
unsigned long  lastSend;
unsigned long  lastPulse;
unsigned long  currentTime;
double val = 0;
double oldval = 0;
int led2 = D7;

String idx="594";
HttpClient http;
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL }
};

 http_request_t request;
 http_response_t response;
 http_request_t request2;
 http_response_t response2;

void setup() {
  Serial.begin(115200);
  pulseCount = oldPulseCount = 0;
  lastSend = millis();
  pinMode(13, OUTPUT);
  Particle.publish("Particle 2","started");
  pinMode(led2, OUTPUT);

}

void loop() {
    
  currentTime = millis();

  val=digitalRead(0);
  //Serial.println(val);
  if ((oldval!=val)&&(val==1)) {
    digitalWrite(led2, HIGH);
    Particle.publish("Sensor",String((long)val)+ " "+String((long)oldval));
    pulseCount++;
    oldval=val;
    delay(500);	
    digitalWrite(led2, LOW);
    //Serial.print('.');
  } else {
    oldval=val;
    delay(100);	
  }
  
    // Only send values at a maximum frequency or woken up from sleep
  bool sendTime;
  if ((currentTime - lastSend) > SEND_FREQUENCY) {sendTime=1;}else{sendTime=0;};

  
    // Pulse count has changed
    if (pulseCount != oldPulseCount) {
      double volume = ((double)pulseCount/((double)PULSE_FACTOR));     
      oldPulseCount = pulseCount;
      if (((volume != oldvolume)||(sendTime))&&(pcReceived)) {
        //SEND gw.send(volumeMsg.set(volume, 3));               // Send volume value to gw
        request2.hostname = "192.168.1.10"; //1.10
        request2.port = 8080;
        request2.path = "/json.htm?type=command&param=udevice&idx="+idx+"&svalue="+String((int)volume);
        // Get request
        http.get(request2, response2, headers);
        Particle.publish("url2",request2.hostname+":"+request2.port+" "+request2.path+ " "+response2.status);
        //Serial.print("V=");
        //Serial.println(volume);
        Particle.publish("Volume",String((double)volume,3));
        oldvolume = volume;
      } 
    }
    lastSend = currentTime;
    //Particle.publish("Status",String(oldvolume)+" "+String(volume)+" "+String(pulseCount)+" "+String(sendTime)+" "+String(pcReceived));

    /*Serial.print(oldvolume);
    Serial.print(" ");
    Serial.print(volume);
    Serial.print(" ");
    Serial.print(pulseCount);
    Serial.print(" ");
    Serial.print(sendTime);
    Serial.print(" ");
    Serial.println(pcReceived);*/
    
    if (! pcReceived) {
    // No count received. Try requesting it again
        request.hostname = "192.168.1.10"; //1.10
        request.port = 8080;
        request.path = "/json.htm?type=devices&rid="+idx;
        // Get request
        http.get(request, response, headers);
        String str(response.body);
        Particle.publish("index",request.path+ " "+response.status);
        if (response.status==200) {
            const char* stringArgs = str.c_str();
            char* myCopy = strtok(strdup(stringArgs), "\n");
            int i=0;
            double res;
    
            while (myCopy != NULL) {
                if (i==12) {
                    //Particle.publish(myCopy);
                    String str = myCopy;
                    String resp= str.substring(22, str.length()-4);
                    res=strtof(resp,NULL);
                    Particle.publish("Histo",String((double)res,3));
                }
                i++;
                myCopy = strtok(NULL, "\n");
            }  
            
            lastSend=currentTime;
            pulseCount = oldPulseCount = res*1000;
            //Serial.print("Received last pulse count from gw:");
            //Serial.println(res);
            //Spark.publish("PC"+pulseCount);
        
            pcReceived = true;delay(1000);
            Particle.publish("Status init",String(oldvolume)+" "+String(volume)+" "+String(pulseCount)+" "+String(sendTime)+" "+String(pcReceived));

        }
    }
  

}

