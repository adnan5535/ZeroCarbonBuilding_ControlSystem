unsigned long time_stamp;  
unsigned int count0 = 0;  
  
  
unsigned int output0 = 0;   
  
  
void setup()  
{  
  Serial.begin(57600);  
  time_stamp = millis();  
  attachInterrupt(2, sensor0, RISING); // Interrupt 2 on mega pin no 21 Due pin 0
}  
  
  
void loop()  
{  
  if(millis() - time_stamp > 500)  
  {  
    detachInterrupt(0); // disable interrups    
    output0 = count0 * 2;  
    Serial.print("Sensor0 Reading = ");  
    Serial.println(output0);  
    delay(40);  
    count0 = 0;
    time_stamp = millis();  
    attachInterrupt(0, sensor0, RISING); // we have done with this mesasurenment,enable interrups for next cycle  
  }  
}  
  
  
void sensor0()  
{  
  count0++;  
}
