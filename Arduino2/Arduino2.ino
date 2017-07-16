#include "EmonLib.h"


#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include <Ethernet.h>
#include <EthernetUdp.h>



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


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xCD };
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
const int timeZone = 0;     // GMT because daylight saving really throws a wrench in calcs
EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
unsigned long time_stamp_1 = 0;
unsigned long time_stamp_2 = 0;
unsigned long time_stamp_3 = 0;
unsigned long time_stamp_4 = 0;

const int SD_chipSelect = 4;
int ethInitErrcode;


void setup() {
  Serial.begin(9600);
    emon0.current(A0,206.896551);
   emon1.current(A1,206.896551);
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

 




float CTArray[numCTs];
String dataString_CTs;
String dateString, timeString, timeString_millis, comboDataString;
String fileExtension = ".csv";
int fileNameModifier = 0;
boolean fileCreated = false;

void loop() {
     /* Possible future improvement Create an array of pointers for iterating over the emon objects. This will
     *  allow for easier expandability.
     *  Will also need to create a const char* array for iterating over the microcontroller pins in the setup()
     *  function. 
     */

     
// for getting date 
     time_t t = now(); // Store the current time in struct object t.
  dateString = String(String(day()) + String("-") + String(month()) + String("-") + String(year()));
  timeString = String(String(hour(t)) + String("h") + String(minute(t)) + String("m") + String(second(t)));
  //timeString_millis = millis(); // needs to be updated to reflect milliseconds since last minute
  comboDataString = dateString + "," + timeString + "," + dataString_CTs; // C-C-C-C-C-C COMBO BREAKER
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
  else{
    Serial.println("Error opening data logging file");}
    
    
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString_CTs);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString_CTs);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
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



