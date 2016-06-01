/**********************************************************
Sketch pour contrôler un potentiomètre numérique
MCP41100 au moyen d'un Arduino Uno.
http://electroniqueamateur.blogspot.ca/2013/04/faire-varier-une-resistance-au-moyen-de.html
**********************************************************/


#include <SPI.h>
 
const int slaveSelectPin = 53;
 
void setup()
{
  Serial.begin(115200);
  pinMode (slaveSelectPin, OUTPUT);
  SPI.begin();
  /*
  // adjust high and low resistance of potentiometer
  // adjust Highest Resistance .
  digitalPotWrite(0x00);
  delay(1000);
  
  // adjust  wiper in the  Mid point  .
  digitalPotWrite(0x80);
  delay(1000);
  
  // adjust Lowest Resistance .
  digitalPotWrite(0xFF);
  delay(1000);
  
  digitalPotWrite(0xFF);*/
}
 
void loop()
{
  /*
  digitalPotWrite(0);  // valeur minimale
  delay(1000); 
  
  digitalPotWrite(255);  // valeur maximale  
  delay(500); 
  
  digitalPotWrite(128);  // valeur médiane
  delay(1000);*/
  /** declanche bien la voiture mais seulement autour de 250 - 255*/
  /* 
  for (int i=0; i <= 255/5; i++){
    digitalPotWrite(i *5);
    delay(150);
  }
  */
  /*digitalPotWrite(5);  // valeur maximal*/

  /*
  digitalPotWrite(200);
  delay(100);
  */

  for (int i=245; i <= 255; i++){
    digitalPotWrite(i);
    delay(100);
  }
}
 
int digitalPotWrite(int value)
{
  Serial.print("digitalPotWrite ");Serial.print(value);Serial.print(" at ");Serial.println(millis());
  analogWrite(slaveSelectPin, LOW);
  SPI.transfer(0x11); 
  SPI.transfer(value);
  digitalWrite(slaveSelectPin, HIGH);
}
