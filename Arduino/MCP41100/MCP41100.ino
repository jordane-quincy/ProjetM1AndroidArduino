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
}
 
void loop()
{
/** declenche bien la voiture sur le circuit aussi (mais faut la lancer manuellement mais après c'est bon)*/
   for (int i=252; i <= 255; i++){
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
