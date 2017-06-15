/* reddit.com/r/startledcats
   Because all good things start with cats */

#include <MuxShield.h>
#include <SD.h>
#include <TimeLib.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

MuxShield muxShield; //Initialize the Mux Shield
const int SD_chipSelect = 4;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov

const int timeZone = 0;     // GMT because daylight saving really throws a wrench in calcs
EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

/* //Defining pins for thermistors - Row 1 on Mux
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
#define thermT16 14 //Mux Pin 15   thermT15 supposedly missing */
#define numThermistors 15 // Total number of thermistors in system. Modify this number if adding/removing thermistors

#define FlowF1 0 // Flow meter numbering, Mega Pin 21
#define FlowF2 1 // Mega Pin 20
#define FlowF3 2 // Mega Pin 19
#define FlowF4 3 // Mega Pin 18
#define numFlowmeters 4 // Total number of flowmeters in system. Modify this number if adding/removing flowmeters

#define numCTs 15 // Number of Current Transducers. Modify this number if adding/removing current transducers

boolean useSameLoop_forflowmeters = false;

void setup() {
    Serial3.begin(9600); // Using 9600 baudrate because I decided that is as high as I want to go while maintaining serial read data integrity
    muxShield.setMode(1,ANALOG_IN);
    time_stamp = millis();  
    attachInterrupt(2, flowmeter1_ISR, RISING); // Interrupt 2 on mega pin no 21
    attachInterrupt(3, flowmeter2_ISR, RISING); // Interrupt 3 on mega pin no 20
    attachInterrupt(4, flowmeter3_ISR, RISING); // Interrupt 4 on mega pin no 19
    attachInterrupt(5, flowmeter4_ISR, RISING); // Interrupt 5 on mega pin no 18
    
    if (numThermistors >= numFlowmeters) useSameLoop_forflowmeters = true; // Trying to minimise loop iterations for optimized code. This allows the use of the same loop
                                                                           // for reading thermistors and flowmeters

    Serial3.print("Initializing SD card...");
    if (!SD.begin(SD_chipSelect))
        Serial3.println("Card failed, or not present. Data is not being logged.");
    else 
        Serial3.println("card initialized. Hurray!");

    if (Ethernet.begin(mac) == 0)
        Serial3.println("Uh oh. Failed to initialise internet connection.");
    Serial3.print("IP number assigned by DHCP is ");
    Serial3.println(Ethernet.localIP());
    Udp.begin(localPort);
    Serial3.println("Waiting for sync");
    setSyncProvider(getNtpTime);
}

float tempArray[numThermistors];
unsigned int flowmeterIter;
float flowmeterArray[numFlowmeters];
float CTArray[numCTs];
String dataString_therms, dataString_flowmeters, dataString_CTs;
String dateString, timeString, timeString_millis, comboDataString;

void loop() {
    dataString_therms = "";
    dataString_flowmeters = "";
    dataString_CTs = "";
    flowmeterIter = 0;
    
    for (int thermIter=0; thermIter < numThermistors; thermIter++){
        tempArray[thermIter] = readTherm(thermIter);
        if (thermIter == 0) dataString_therms += tempArray[thermIter]; // don't want comma before first value
        else dataString_therms += "," + String(tempArray[thermIter]);
        if (useSameLoop_forflowmeters)  // Trying to use same loop for reading thermistors and flowmeters
            if (flowmeterIter < numFlowmeters){
                flowmeterArray[flowmeterIter] = readFlowmeter(flowmeterIter);
                if (flowmeterIter == 0) dataString_flowmeters += flowmeterArray[flowmeterIter]; // don't want comma before first value
                else dataString_flowmeters += "," + String(flowmeterArray[flowmeterIter]);
                flowmeterIter++;
            }
    }
    if (!useSameLoop_forflowmeters) // This runs if number of flowmeters > number of thermistors
        for (flowmeterIter=0; flowmeterIter < numFlowmeters; flowmeterIter++){
            flowmeterArray[flowmeterIter] = readFlowmeter(flowmeterIter);
            if (flowmeterIter == 0) dataString_flowmeters += flowmeterArray[flowmeterIter]; // don't want comma before first value
            else dataString_flowmeters += "," + String(flowmeterArray[flowmeterIter]);
        }
    for(int CTIter = 0; CTIter < numCTs; CTIter++){  // Possible future improvement: add this logic to the same loop as thermistors for more optimized code
        if(Serial3.available()){
            CTArray[CTIter] = Serial3.parseFloat();
            if (CTIter == 0) dataString_CTs += CTArray[CTIter]; // don't want comma before first value
            else dataString_CTs += "," + String(CTArray[CTIter]);
        }
    }
    
    time_t t = now(); // Store the current time in struct object t.
    dateString = String(day()) + "-" + String(month()) + "-" + String(year()) + "  ";
    timeString = String(hour(t)) + "h" + String(minute(t)) + "m" + String(second(t)) + "s";
    timeString_millis = millis();
    comboDataString = dateString + "," + timeString + "," + timeString_millis + "," + dataString_therms + "," + dataString_flowmeters + "," + dataString_CTs; // C-C-C-C-C-C COMBO BREAKER
    
    File fileHandle = SD.open(dateString + timeString + timeString_millis + ".csv", FILE_WRITE); // Create and open file for writing
    if (fileHandle) {
        fileHandle.println(comboDataString);
        fileHandle.close();
        Serial3.println(comboDataString); // C-C-C-C-C-C COMBO BREAKER
    }
    else
        Serial3.println("error opening data logging file");
}

float readTherm(int thermPin){
    float thermVolt;
    float temp;
    
    thermVolt = (muxShield.analogReadMS(1,thermPin)) * (5/1023) * 0.94; //0.94 empirical correction for error introduced by mux shield
    temp = (1.8443 * pow(thermVolt,4)) - (14.248 * pow(thermVolt,3)) + (31.071 * pow(thermVolt,2)) + (6.5131 * thermVolt) - 38.282; // Based on curve fit. Excel sheet on Dropbox
    return(temp);
}

float flowmeterFreq = 0; // declaring as global because if the first if in readFlowmeter doesn't get satisfied, returned frequency will be the previously measured frequency
unsigned int numRisingEdges1 = 0, numRisingEdges2 = 0, numRisingEdges3 = 0, numRisingEdges4 = 0;
unsigned long time_stamp = 0;

float readFlowmeter(int flowmeterNum){
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


// Interrupt service routines for measuring flowmeter frequencies
void flowmeter1_ISR() { numRisingEdges1++; }
void flowmeter2_ISR() { numRisingEdges2++; }
void flowmeter3_ISR() { numRisingEdges3++; }
void flowmeter4_ISR() { numRisingEdges4++; }


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    Serial3.println("Transmit NTP Request");
    sendNTPpacket(timeServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            Serial3.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR; // returns time
        }
    }
    Serial3.println("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:                 
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
