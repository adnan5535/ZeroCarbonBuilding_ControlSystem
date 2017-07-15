/* reddit.com/r/startledcats
   Because all good things start with cats */

#include <MuxShield.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define numThermistors 16 // Total number of thermistors in system. Modify this number if adding/removing thermistors
const byte FlowF1 = 21; // Flow meter numbering, Mega Pin 21
const byte FlowF2 = 20; // Mega Pin 20
const byte FlowF3 = 19; // Mega Pin 19
const byte FlowF4 = 18; // Mega Pin 18
int FlowmeterArray[] = {FlowF1, FlowF2, FlowF3, FlowF4};
#define numFlowmeters 4 // Total number of flowmeters in system. Modify this number if adding/removing flowmeters
#define numCTs 15 // Number of Current Transducers. Modify this number if adding/removing current transducers

MuxShield muxShield; //Initialize the Mux Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
const int timeZone = 0;     // GMT because daylight saving really throws a wrench in calcs
EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
unsigned long time_stamp_1 = 0;
unsigned long time_stamp_2 = 0;
unsigned long time_stamp_3 = 0;
unsigned long time_stamp_4 = 0;

//boolean useSameLoop_forflowmeters = false;
const int SD_chipSelect = 4;
int ethInitErrcode;

void setup() {
  Serial.begin(9600); // Using 9600 baudrate because I decided that is as high as I want to go while maintaining serial read data integrity
  Serial3.begin(9600);
  muxShield.setMode(1, ANALOG_IN);
  time_stamp_1 = millis();

  /* --------------------------------------------------------------------------------------
     This block of code initialises interrupts used for Flowmeter frequency readings
    ----------------------------------------------------------------------------------------
  */
  attachInterrupt(digitalPinToInterrupt(FlowF1), FlowISR_1, RISING); // Interrupt 2 on mega pin no 21
  attachInterrupt(digitalPinToInterrupt(FlowF2), flowmeter2_ISR, RISING); // Interrupt 3 on mega pin no 20
  attachInterrupt(digitalPinToInterrupt(FlowF3), flowmeter3_ISR, RISING); // Interrupt 4 on mega pin no 19
  attachInterrupt(digitalPinToInterrupt(FlowF4), flowmeter4_ISR, RISING); // Interrupt 5 on mega pin no 18


  // if (numThermistors >= numFlowmeters) useSameLoop_forflowmeters = true; // Trying to minimise loop iterations for optimized code. This allows the use of the same loop
  // for reading thermistors and flowmeters
  /* --------------------------------------------------------------------------------------
     This block initialises SD card data logging system. Note, due to use of pin 10 by
    ----------------------------------------------------------------------------------------
  */
  pinMode(4, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);    // disable w5100 ethernet controller SPI chip select while setting up SD
  digitalWrite(4, LOW);
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_chipSelect))
    Serial.println("Card failed, or not present. Data is not being logged.");
  else
    Serial.println("Card initialized. Hurray!");

  /* --------------------------------------------------------------------------------------
     This block of code initialises internet connection
    ----------------------------------------------------------------------------------------
  */
  digitalWrite(10, LOW);
  digitalWrite(4, HIGH); // disable SD card pin select
  ethInitErrcode = Ethernet.begin(mac);
  if (ethInitErrcode == 0) // Program will wait here for some time and then proceed
    Serial.println("Uh oh. Failed to initialise internet connection.");
  delay(2000); // takes a second for the w5100 to get ready
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(Ethernet.localIP());
  Udp.begin(localPort);
  Serial.println("Waiting for sync");
  setSyncProvider(getNtpTime);

}

float tempArray[numThermistors];
unsigned int flowmeterIter;
float FlowmeterOutputArray[numFlowmeters];
float CTArray[numCTs];
String dataString_therms, dataString_flowmeters, dataString_CTs;
String dateString, timeString, timeString_millis, comboDataString;
String fileExtension = ".csv";
int fileNameModifier = 0;
boolean fileCreated = false;

void loop()
{
  dataString_therms = "";
  dataString_flowmeters = "";
  dataString_CTs = "";
  flowmeterIter = 0;

  for (int thermIter = 0; thermIter < numThermistors; thermIter++)
  {
    tempArray[thermIter] = readTherm(thermIter);
    if (thermIter == 0) dataString_therms += tempArray[thermIter]; // don't want comma before first value
    else dataString_therms += "," + String(tempArray[thermIter]);
  }

  for (flowmeterIter = 0; flowmeterIter < numFlowmeters; flowmeterIter++)
  {
    FlowmeterOutputArray[flowmeterIter] = readFlowmeter(FlowmeterArray[flowmeterIter]);
    if (flowmeterIter == 0) dataString_flowmeters += FlowmeterOutputArray[flowmeterIter]; // don't want comma before first value
    else dataString_flowmeters += "," + String(FlowmeterOutputArray[flowmeterIter]);
  }

  for (int CTIter = 0; CTIter < numCTs; CTIter++)
  { // Possible future improvement: add this logic to the same loop as thermistors for more optimized code
    if (Serial3.available())
    {
      CTArray[CTIter] = Serial3.parseFloat();
      if (CTIter == 0) dataString_CTs += CTArray[CTIter]; // don't want comma before first value
      else dataString_CTs += "," + String(CTArray[CTIter]);
    }
  }
  time_t t = now(); // Store the current time in struct object t.
  dateString = String(String(day()) + String("-") + String(month()) + String("-") + String(year()));
  timeString = String(String(hour(t)) + String("h") + String(minute(t)) + String("m") + String(second(t)));
  //timeString_millis = millis(); // needs to be updated to reflect milliseconds since last minute
  comboDataString = dateString + "," + timeString + "," + dataString_therms + "," + dataString_flowmeters + "," + dataString_CTs; // C-C-C-C-C-C COMBO BREAKER
  digitalWrite(10, HIGH); // disable ethernet controller SPI chip select
  digitalWrite(4, LOW); // enable SD card SPI chip select
  String fileName = String(fileNameModifier) + fileExtension;
  if (!fileCreated)
    while (SD.exists(fileName)) fileName = String(++fileNameModifier) + fileExtension;
  fileCreated = true;
  File fileHandle = SD.open(fileName, FILE_WRITE); // Create and open file for writing
  if (fileHandle) {
    fileHandle.println(comboDataString);
    fileHandle.close();
    Serial.println(comboDataString); // C-C-C-C-C-C COMBO BREAKER
    digitalWrite(10, LOW); // enable ethernet controller SPI chip select
    digitalWrite(4, HIGH); // disable SD card SPI chip select
  }
  else
    Serial.println("Error opening data logging file");
}

/* --------------------------------------------------------------------------------------
   This block converts thermistor + ADC raw data (0-1023) to a 0-5v signal, then applies
   the termperature voltage correlation supplied by manufacturer.
  ----------------------------------------------------------------------------------------
*/
float readTherm(int thermPin) {
  float thermVolt;
  float temp;

  thermVolt = (muxShield.analogReadMS(1, thermPin)) * 0.0048828; //=raw*5/1024
  temp = (1.8443 * pow(thermVolt, 4)) - (14.248 * pow(thermVolt, 3)) + (31.071 * pow(thermVolt, 2)) + (6.5131 * thermVolt) - 38.282; // Based on curve fit. Excel sheet on Dropbox
  //temp = 5;
  return (temp);
}

/* --------------------------------------------------------------------------------------
   This block uses interrupts to count the number of hall effect sensor events that occur in 500ms for each
   sensor. This number is then converted to a frequency, before the manufacturers
   supplied correlation is used to determine flowrate.
  ----------------------------------------------------------------------------------------
*/

float flowmeterFreq = 0; // declaring as global because if the first if statement in readFlowmeter doesn't get satisfied, returned frequency will be the previously measured frequency
unsigned int numRisingEdges1 = 0, numRisingEdges2 = 0, numRisingEdges3 = 0, numRisingEdges4 = 0;
float massFlowRate = 0;

float readFlowmeter(int flowmeterNum)
{
  {
    switch (flowmeterNum)
    {
      case FlowF1:
        if (millis() - time_stamp_1 > 500)
          detachInterrupt(digitalPinToInterrupt(FlowF1)); // disable interrupts
        flowmeterFreq = numRisingEdges1 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second
        delay(40);
        numRisingEdges1 = 0;
        time_stamp_1 = millis();
        attachInterrupt(digitalPinToInterrupt(FlowF1), FlowISR_1, RISING); // we have done with this mesasurenment,enable interrups for next cycle
        break;

      case FlowF2:
        if (millis() - time_stamp_2 > 500)
          detachInterrupt(digitalPinToInterrupt(FlowF2));
        flowmeterFreq = numRisingEdges2 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second
        // flowmeterFreq = 2;
        delay(40);
        numRisingEdges2 = 0;
        time_stamp_2 = millis();
        attachInterrupt(digitalPinToInterrupt(FlowF2), flowmeter2_ISR, RISING); // we have done with this mesasurenment,enable interrups for next cycle
        break;

      case FlowF3:
        if (millis() - time_stamp_3 > 500)
          detachInterrupt(digitalPinToInterrupt(FlowF3));
        flowmeterFreq = numRisingEdges3 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second
        // flowmeterFreq = 3;
        delay(40);
        numRisingEdges3 = 0;
        time_stamp_3 = millis();
        attachInterrupt(digitalPinToInterrupt(FlowF3), flowmeter3_ISR, RISING); // we have done with this mesasurenment,enable interrups for next cycle
        break;

      case FlowF4:
        if (millis() - time_stamp_4 > 500)
          detachInterrupt(digitalPinToInterrupt(FlowF4));
        flowmeterFreq = numRisingEdges4 * 2; // 500 ms window. Multiply by 2 to get number of rising edges in 1 second

        delay(40);
        numRisingEdges4 = 0;
        time_stamp_4 = millis();
        attachInterrupt(digitalPinToInterrupt(FlowF4), flowmeter4_ISR, RISING); // we have done with this mesasurenment,enable interrups for next cycle
        break;
    }
  }
  //massFlowRate = (0.1628*flowmeterFreq) + 1.2825;
  massFlowRate = flowmeterFreq;
  return (massFlowRate);
}


// Interrupt service routines for measuring flowmeter frequencies
void FlowISR_1() {
  numRisingEdges1++;
}
void flowmeter2_ISR() {
  numRisingEdges2++;
}
void flowmeter3_ISR() {
  numRisingEdges3++;
}
void flowmeter4_ISR() {
  numRisingEdges4++;
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
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
  Serial.println("No NTP Response :-(");
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

