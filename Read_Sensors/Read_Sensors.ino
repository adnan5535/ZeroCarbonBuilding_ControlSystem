#include <MuxShield.h>

//Initialize the Mux Shield
MuxShield muxShield;

#define therm1 0 //Mux Pin 0

void setup() {
  //Set I/O 1, I/O 2, and I/O 3 as analog inputs
    muxShield.setMode(1,ANALOG_IN);
//    muxShield.setMode(2,ANALOG_IN);
//    muxShield.setMode(3,ANALOG_IN);
    
    Serial.begin(57600);

}

//Arrays to store analog values after recieving them
//int IO1AnalogVals[16];
//int IO2AnalogVals[16];
//int IO3AnalogVals[16];
float temp;
float thermVolt;

void loop() {
//  for (int i=0; i<16; i++)
//  {
//    //Analog read on all 16 inputs on IO1, IO2, and IO3
//    IO1AnalogVals[i] = muxShield.analogReadMS(1,i);
//    IO2AnalogVals[i] = muxShield.analogReadMS(2,i);
//    IO3AnalogVals[i] = muxShield.analogReadMS(3,i);
//  }
  
  temp = readTherm(therm1);
  
  Serial.print(temp);
  Serial.println();

}

float readTherm(int thermPin){
   thermVolt = muxShield.analogReadMS(1,thermPin);
   thermVolt = (thermVolt * 5/1023);
   temp = (1.8443 * pow(thermVolt,4)) - (14.248 * pow(thermVolt,3)) + (31.071 * pow(thermVolt,2)) + (6.5131 * thermVolt) - 38.282;
   return(thermVolt);
}

