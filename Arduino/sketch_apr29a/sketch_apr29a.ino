#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>

#include <TinkerKit.h>

#define FLOAT_SIZE 4 // sizeof(float) == 4 byte
#define IR_SIZE FLOAT_SIZE
#define MANETTE1_SIZE FLOAT_SIZE
#define MSG_LENGTH IR_SIZE + MANETTE1_SIZE

#define INPUT_PIN_IR I0
#define INPUT_PIN_LDR I1
#define INPUT_PIN_MANETTE1 I2

#define LED0_INTEGRATED 13

#define OUTPUT_PIN_LED O1

TKPotentiometer manette1(INPUT_PIN_MANETTE1);
int manette1Val = 0;

AndroidAccessory acc("Manufacturer",
                     "Project03",
                     "Description",
                     "Version",
                     "URI",
                     "Serial");

byte msg[MSG_LENGTH];

byte msgReceived[MSG_LENGTH];
int len;

union FloatToByteArray
{
  float    f;
  uint32_t byteArray[4];
};

union ByteArrayToInt
{
  uint32_t byteArray[4];
  int    i;
};
ByteArrayToInt intConverter;

void setup() {
  Serial.begin(115200);
  acc.powerOn();
  
  pinMode(INPUT_PIN_LDR, INPUT);
  pinMode(OUTPUT_PIN_LED, OUTPUT);
  digitalWrite(OUTPUT_PIN_LED, HIGH);

  pinMode(LED0_INTEGRATED, OUTPUT);
}

void loop() {
  if (acc.isConnected()) {
    //partie IR
    float IR_Voltage = getVoltage(INPUT_PIN_IR);

    //Serial.print("IR :");Serial.print(IR_Voltage);Serial.print("V ");

    FloatToByteArray irConverter;
    irConverter.f = IR_Voltage;

    memcpy(msg, irConverter.byteArray, 4); //float sur 4 byte
/*
    //partie LDR
    float LDR_Voltage = getVoltage(INPUT_PIN_LDR);

    Serial.print("LDR :");Serial.print(LDR_Voltage);Serial.print("V ");

    FloatToByteArray ldrConverter;
    ldrConverter.f = LDR_Voltage;

    memcpy(msg, irConverter.byteArray, 4); //float sur 4 byte
*/
    //partie manette1
    manette1Val = manette1.read();
   // Serial.println(manette1Val);

    //Partie reception donnees depuis android
    len = acc.read(msgReceived, MSG_LENGTH, 1);
    if(len > 0){
      memcpy(intConverter.byteArray, msgReceived, 4); //int sur 4 byte
      int nbBlink = intConverter.i;
      Serial.print("Recu : ");Serial.println(nbBlink);
      
      blinkLedIntegratedXtimes(nbBlink);
    }
    
    //Partie envoi a android
    acc.write(msg, MSG_LENGTH);

    delay(100);
  } else {
    Serial.print("AndroidAccessory not connected at ");Serial.println(millis());

    delay(1000);
  }
}


float getVoltage(int pin)
{
  return (analogRead(pin) * 0.004882814);
  // This equation converts the 0 to 1023 value that analogRead()
  // returns, into a 0.0 to 5.0 value that is the true voltage
  // being read at that pin.
}

void blinkLedIntegratedXtimes(int x){
  for (int i=0; i < x; i++){
    digitalWrite(LED0_INTEGRATED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);              // wait for a second
    digitalWrite(LED0_INTEGRATED, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);              // wait for a second
  }
}



