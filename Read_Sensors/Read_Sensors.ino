#include <MuxShield.h>

//Initialize the Mux Shield
MuxShield muxShield;

//Defining pins for thermistors - Row 1 on Mux
#define thermT1 0 //Mux Pin 1
#define thermT2 1 //Mux Pin 2
#define thermT3 2 //Mux Pin 3
#define thermT4 3 //Mux Pin 4
#define thermT5 4 //Mux Pin 5
#define thermT6 5 //Mux Pin 6
#define thermT7 6 //Mux Pin 7
#define thermT8 7 //Mux Pin 8
#define thermT9 8 //Mux Pin 9
#define thermT10 9 //Mux Pin 10
#define thermT11 10 //Mux Pin 11
#define thermT12 11 //Mux Pin 12
#define thermT13 12 //Mux Pin 13
#define thermT14 13 //Mux Pin 14
#define thermT16 14 //Mux Pin 15   thermT15 supposedly missing
#define numThermistors 15

#define FlowF1 1 // Flow meter numbering
#define FlowF2 2
#define FlowF3 3
#define FlowF4 4
#define numFlowmeters 4

#define numCTs 15 // Number of Current Transducers

unsigned int numRisingEdges1 = 0, numRisingEdges2 = 0, numRisingEdges3 = 0, numRisingEdges4 = 0;
unsigned long time_stamp = 0;
boolean useSameLoop_forflowmeters = false;

void setup() {
    //Set I/O 1, I/O 2, and I/O 3 as analog inputs
    muxShield.setMode(1,ANALOG_IN);
    time_stamp = millis();  
    attachInterrupt(2, flowmeter1_ISR, RISING); // Interrupt 2 on mega pin no 21 Due pin 0
    attachInterrupt(3, flowmeter2_ISR, RISING); // Interrupt 2 on mega pin no 21 Due pin 0
    attachInterrupt(4, flowmeter3_ISR, RISING); // Interrupt 2 on mega pin no 21 Due pin 0
    attachInterrupt(5, flowmeter4_ISR, RISING); // Interrupt 2 on mega pin no 21 Due pin 0
    Serial3.begin(9600);
    if (numThermistors >= numFlowmeters) useSameLoop_forflowmeters = true;    
}

float tempArray[numThermistors];
unsigned int flowmeterIter;
float flowmeterArray[numFlowmeters];
float CTArray[numCTs];

void loop() {
  flowmeterIter = 1;
  for (int thermIter=0; thermIter < numThermistors; thermIter++){
    tempArray[thermIter] = readTherm(thermIter);
    if (useSameLoop_forflowmeters)
      if (flowmeterIter++ <= numFlowmeters)
        flowmeterArray[flowmeterIter-1] = readFlowmeter(flowmeterIter);
  }
  if (!useSameLoop_forflowmeters)
    for (flowmeterIter=0; flowmeterIter < numFlowmeters; flowmeterIter++)
      flowmeterArray[flowmeterIter] = readFlowmeter(flowmeterIter);
  
  for(int CTIter = 0; CTIter < numCTs; CTIter++){
    if(Serial3.available()){
      CTArray[CTIter] = Serial.parseFloat();
    }
  }
}

float readTherm(int thermPin){
   float thermVolt;
   float temp;
   
   thermVolt = (muxShield.analogReadMS(1,thermPin)) * (5/1023) * 0.94; //0.94 empirical correction
   temp = (1.8443 * pow(thermVolt,4)) - (14.248 * pow(thermVolt,3)) + (31.071 * pow(thermVolt,2)) + (6.5131 * thermVolt) - 38.282;
   return(temp);
}

float readFlowmeter(int flowmeterNum){
  float flowmeterFreq = 0;
  
  if(millis() - time_stamp > 500)  
  {  
    switch(flowmeterNum){
      case FlowF1:
      detachInterrupt(2); // disable interrupts
      flowmeterFreq = numRisingEdges1 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second
      delay(40);
      numRisingEdges1 = 0;
      time_stamp = millis();  
      attachInterrupt(2, flowmeter1_ISR, RISING); // we have done with this mesasurenment,enable interrups for next cycle
      break;
      
      case FlowF2:
      detachInterrupt(3);
      flowmeterFreq = numRisingEdges2 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second
      delay(40);
      numRisingEdges2 = 0;
      time_stamp = millis();  
      attachInterrupt(3, flowmeter2_ISR, RISING); // we have done with this mesasurenment,enable interrups for next cycle
      break;
      
      case FlowF3:
      detachInterrupt(4);
      flowmeterFreq = numRisingEdges3 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second
      delay(40);
      numRisingEdges3 = 0;
      time_stamp = millis();  
      attachInterrupt(4, flowmeter3_ISR, RISING); // we have done with this mesasurenment,enable interrups for next cycle
      break;
      
      case FlowF4:
      detachInterrupt(5);
      flowmeterFreq = numRisingEdges4 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second
      delay(40);
      numRisingEdges4 = 0;
      time_stamp = millis();  
      attachInterrupt(5, flowmeter4_ISR, RISING); // we have done with this mesasurenment,enable interrups for next cycle
      break;
    }
  }
  return(flowmeterFreq);
}

void flowmeter1_ISR(){
  numRisingEdges1++;
}

void flowmeter2_ISR(){
  numRisingEdges2++;
}

void flowmeter3_ISR(){
  numRisingEdges3++;
}

void flowmeter4_ISR(){
  numRisingEdges4++;
}

