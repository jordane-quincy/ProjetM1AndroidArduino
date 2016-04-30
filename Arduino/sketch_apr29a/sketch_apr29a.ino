#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>

#include <TinkerKit.h>

#define MSG_LENGTH 8 * 4 // sizeof(float) == 4 octets
#define INPUT_PIN I3

AndroidAccessory acc("Manufacturer",
                     "Project03",
                     "Description",
                     "Version",
                     "URI",
                     "Serial");

byte msg[MSG_LENGTH];
int lastButtonState;
int currentButtonState;

union FloatToByteArray
{
  float    f;
  uint32_t byteArray[4];
};

void setup() {
  Serial.begin(115200);
  acc.powerOn();
}

void loop() {
  if (acc.isConnected()) {
    float voltage = getVoltage(INPUT_PIN);

    Serial.print(voltage);Serial.println("V ");

    FloatToByteArray vConverter;
    vConverter.f = voltage;

    memcpy(msg, vConverter.byteArray, 4); //float sur 4 byte

    //Serial.println(msg);
    acc.write(msg, MSG_LENGTH);

    delay(1000);
  } else {
    Serial.print("AndroidAccessory not connected at ");Serial.println(millis());

    delay(2000);
  }
}


float getVoltage(int pin)
{
  return (analogRead(pin) * 0.004882814);
  // This equation converts the 0 to 1023 value that analogRead()
  // returns, into a 0.0 to 5.0 value that is the true voltage
  // being read at that pin.
}

