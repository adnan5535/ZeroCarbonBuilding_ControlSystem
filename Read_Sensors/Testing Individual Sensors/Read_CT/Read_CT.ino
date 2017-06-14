#include <MuxShield.h>
#include "EmonLib.h"                   // Include Emon Library

EnergyMonitor emon1;                   // Create an instance
MuxShield muxShield;                   //Initialize the Mux Shield

#define CT1 0 //Mux Pin 0 (Row 3)

void setup() {
  //Set I/O 1, I/O 2, and I/O 3 as analog inputs
    muxShield.setMode(3,ANALOG_IN);
    emon1.current(A3, 246.3);             // Current: input pin, calibration.
    Serial.begin(57600);
}

double Irms;

void loop() {
  Irms = emon1.calcIrms(1480);  // Calculate Irms only. Argument is number of samples to read
  
  Serial.print(" ");
  Serial.println(Irms);          // Irms
}
