#include "EmonLib.h"

#define numCTs 16
EnergyMonitor emon0;
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;
EnergyMonitor emon4;
EnergyMonitor emon5;
EnergyMonitor emon6;
EnergyMonitor emon7;
EnergyMonitor emon8;
EnergyMonitor emon9;
EnergyMonitor emon10;
EnergyMonitor emon11;
EnergyMonitor emon12;
EnergyMonitor emon13;
EnergyMonitor emon14;
EnergyMonitor emon15;
void setup() {
    Serial.begin(9600);
    emon0.current(A0,206.896551);
    emon1.current(A1,206.896551);//246.3
    emon2.current(A2,206.896551);
    emon3.current(A3,206.896551);
    emon4.current(A4,206.896551);
    emon5.current(A5,206.896551);
    emon6.current(A6,206.896551);
    emon7.current(A7,206.896551);
    emon8.current(A8,206.896551);
    emon9.current(A9,206.896551);
    emon10.current(A10,206.896551);
    emon11.current(A11,206.896551);
    emon12.current(A12,206.896551);
    emon13.current(A13,206.896551);
    emon14.current(A14,206.896551);
    emon15.current(A15,206.896551);
}


float CTArray[numCTs];
String dataString_CTs;

void loop() {
     /* Possible future improvement Create an array of pointers for iterating over the emon objects. This will
     *  allow for easier expandability.
     *  Will also need to create a const char* array for iterating over the microcontroller pins in the setup()
     *  function. 
     */
    CTArray[0] = emon1.calcIrms(1282);//1480
    CTArray[1] = emon2.calcIrms(1282);
    CTArray[2] = emon3.calcIrms(1282);
    CTArray[3] = emon4.calcIrms(1282);
    CTArray[4] = emon5.calcIrms(1282);
    CTArray[5] = emon6.calcIrms(1282);
    CTArray[6] = emon7.calcIrms(1282);
    CTArray[7] = emon8.calcIrms(1282);
    CTArray[8] = emon9.calcIrms(1282);
    CTArray[9] = emon10.calcIrms(1282);
    CTArray[10] = emon11.calcIrms(1282);
    CTArray[11] = emon12.calcIrms(1282);
    CTArray[12] = emon13.calcIrms(1282);
    CTArray[13] = emon14.calcIrms(1282);
    CTArray[14] = emon15.calcIrms(1282);
    dataString_CTs = 
    String(CTArray[0]) + "," + String(CTArray[1]) + "," + String(CTArray[2]) + "," + String(CTArray[3]) + "," + 
    String(CTArray[4]) + "," + String(CTArray[5]) + "," + String(CTArray[6]) + "," + String(CTArray[7]) + "," + 
    String(CTArray[8]) + "," + String(CTArray[9]) + "," + String(CTArray[10]) + "," + String(CTArray[11]) + "," + 
    String(CTArray[12]) + "," + String(CTArray[13]) + "," + String(CTArray[14]);
    Serial.println(dataString_CTs);
}
