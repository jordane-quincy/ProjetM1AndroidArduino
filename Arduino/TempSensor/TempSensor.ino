#include <TinkerKit.h>

#define PIN_TEMP_SENSOR I1 // thermistor connected on I1 (TinkerKit shield)
TKThermistor thermistor(PIN_TEMP_SENSOR); //create the helper class and set the pinmode (INPUT)
float celsius;

void setup() {
  Serial.begin(115200);
}

void loop() {
  celsius = thermistor.readCelsius(); // Reading the value from the thermistor

  Serial.print("Temperature : ");
  Serial.print(celsius);Serial.print((char)176);Serial.print("C \r\n"); // (char)176 == ascii code of ° (degree sign)

  delay(1000); // wait for a second
}
