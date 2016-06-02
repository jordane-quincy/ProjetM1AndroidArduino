/**********************************************************
Sketch pour controler un potentiomètre numérique
MCP41100 au moyen d'un Arduino Mega ADK.
**********************************************************/

#include <SPI.h>

#define SLAVE_SELECT_PIN 53 //sur l'arduino mega adk il est sur le pin 53
 
void setup()
{
  Serial.begin(115200);
  pinMode (SLAVE_SELECT_PIN, OUTPUT);
  SPI.begin();
}
 
void loop()
{
  /** declenche bien la voiture sur le circuit aussi (mais faut la lancer manuellement mais après c'est bon)*/
  /*for (int i=248; i <= 249; i++){
    digitalPotWrite(i);
    delay(100);
  }
  */
  /** declenche bien la voiture sur le circuit aussi (mais faut la lancer manuellement mais après c'est bon)*/
  for (int i=0; i <= 1000; i++){
    digitalPotWrite(250);
    digitalPotWrite(0);
  }
  
}
 
int digitalPotWrite(int value)
{
  Serial.print("digitalPotWrite ");Serial.print(value);Serial.print(" at ");Serial.println(millis());
  digitalWrite(SLAVE_SELECT_PIN, LOW);
  SPI.transfer(0x11);
  SPI.transfer(value);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
}
