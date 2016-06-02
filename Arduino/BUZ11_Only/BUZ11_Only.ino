/**********************************************************
Sketch pour controler un mosfet de type N
a l'aide d'un potentiomètre numérique
MCP41100 au moyen d'un Arduino Mega ADK.
**********************************************************/

#define PIN 45 //sur l'arduino mega adk il est sur le pin 53

void setup()
{
  Serial.begin(115200);
  pinMode(PIN, OUTPUT);
  analogWrite(PIN, 0); // init
}
 
void loop()
{ /*
  //int value = 115; //au ralenti
  // int value = 120; // ça tourne
  // int value = 135; // ça tourne assez vite
  // int value = 160; // fast
  //int value = 175; //too fast (ca sort)
  int value = 130;
  Serial.print("analogWrite ");Serial.print(value);Serial.print(" at ");Serial.println(millis());
  analogWrite(PIN, value);
  */
    int value =random(120,160);
    Serial.print("analogWrite ");Serial.print(value);Serial.print(" at ");Serial.println(millis());
    analogWrite(PIN, value);
    delay(3000);
}
