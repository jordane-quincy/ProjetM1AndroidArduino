#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>

#include <TinkerKit.h>

#define COMMAND_BUTTON 0x1
#define TARGET_BUTTON 0x1
#define VALUE_ON 0x1
#define VALUE_OFF 0x0
#define INPUT_PIN I3

AndroidAccessory acc("Manufacturer",
                     "Project03",
                     "Description",
                     "Version",
                     "URI",
                     "Serial");

byte sntmsg[3];
int lastButtonState;
int currentButtonState;

void setup() {
  Serial.begin(115200);
  acc.powerOn();
  sntmsg[0] = COMMAND_BUTTON;
  sntmsg[1] = TARGET_BUTTON;
}

void loop() {
  if (acc.isConnected()) {
    currentButtonState = digitalRead(INPUT_PIN);

    Serial.print(getVoltage(INPUT_PIN));
    Serial.print("V :");
    Serial.println(currentButtonState);

    if (lastButtonState != currentButtonState) {
      if (currentButtonState == LOW) {
        sntmsg[2] = VALUE_ON;
      } else {
        sntmsg[2] = VALUE_OFF;
      }

      acc.write(sntmsg, 3);
      lastButtonState = currentButtonState;
    }

    delay(1000);
  } else {
    Serial.print("AndroidAccessory not connected at ");
    Serial.println(millis());
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

