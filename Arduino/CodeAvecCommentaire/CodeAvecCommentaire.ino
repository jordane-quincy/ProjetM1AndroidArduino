//import des librairies
#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>
#include <TinkerKit.h>

//definition des constantes
#define INT_SIZE 4 // un int est stocke sur 4 byte
#define FLOAT_SIZE 4 // un float est stocke sur 4 byte
#define IR1_SIZE FLOAT_SIZE // taille (en byte) prevu pour mettre les donnees du capteur infra-rouge 1 : un float
#define IR2_SIZE FLOAT_SIZE // taille (en byte) prevu pour mettre les donnees du capteur infra-rouge 2 : un float
#define MSG_LENGTH_TO_SEND IR1_SIZE + IR2_SIZE // taille (en byte) prevu pour le message qui transitera de l'arduino a l'android = taille des donnes des deux capteurs

#define MSG_LENGTH_TO_RECEIVED 8 //taille (en byte) prevu pour le message qui transitera de l'android a l'arduino

#define INPUT_PIN_IR1 A9 //le capteur infra-rouge 1 est branche sur le pin A9
#define INPUT_PIN_IR1 A10 //le capteur infra-rouge 2 est branche sur le pin A10

#define LED_INTEGRATED 13 //sur l'arduino mega adk, il y a une led integre sur la carte sur le pin 13

//creation de l'objet AndroidAccessory qui implemente le protocole Android Open Accessory (AOA)
AndroidAccessory acc("J-B DURIEZ et Jordane QUINCY",
                     "ProjectM1",
                     "Accesoire android du Projet de Master 1 TNSI",
                     "1.0",
                     "https://github.com/jordane-quincy/ProjetM1AndroidArduino",
                     "0123456789876543");

byte msgToSend[MSG_LENGTH_TO_SEND];

byte msgReceived[MSG_LENGTH_TO_RECEIVED];
int len;

//Convertisseur de float en tableau de byte (et inversement)
union FloatToByteArray
{
  float    f;
  uint32_t byteArray[FLOAT_SIZE];
};
FloatToByteArray ir1Converter;

//Convertisseur de tableau de byte en int (et inversement)
union ByteArrayToInt
{
  uint32_t byteArray[INT_SIZE];
  int    i;
};
ByteArrayToInt intConverter;

//premiere methode executee a l'allumage de la carte arduino, n'est lancee qu'une seule fois au demarrage
void setup() {
  Serial.begin(115200); //on configure la vitesse de la liaison serie avec le pc a 115200 bits/sec (la plus veloce)
  acc.powerOn(); //on alimente l'accesoire
  pinMode(LED_INTEGRATED, OUTPUT); //on configure le pin de la led integre comme etant une sortie
}

//apres setup(), loop est lancee en boucle indefiniment (equivalent d'un while(true){} )
void loop() {
  if (acc.isConnected()) {
    //partie IR1
    float IR1_Voltage = getVoltage(INPUT_PIN_IR1); //obtention du voltage

    Serial.print("IR1 :");Serial.print(IR1_Voltage);Serial.print("V \n");

    ir1Converter.f = IR1_Voltage; // on set le voltage du capteur infra-rouge 1 dans le converter

  // ir1Converter converti les donnees du capteur infra-rouge 1 sous forme de tableau de byte
    memcpy(msgToSend, ir1Converter.byteArray, FLOAT_SIZE); // copie de FLOAT_SIZE octet (byte) du tableau de byte dans le message a envoyer a l'android

    //Partie reception donnees depuis android
    //len va contenir la longeur du message recu de l'android
    len = acc.read(msgReceived, MSG_LENGTH_TO_RECEIVED, 1);
    //si on a bien reçu des donnes de l'android
    if(len > 0){
      memcpy(intConverter.byteArray, msgReceived, INT_SIZE); //on utilise le tableau de byte du converter pour y mettre les donnees du telephone
      int nbBlink = intConverter.i;
      Serial.print("Recu : ");Serial.println(nbBlink);

      blinkLedIntegratedXtimes(nbBlink); // on fait clignoter X fois la led
    }

    //Partie envoi a android
    acc.write(msgToSend, MSG_LENGTH_TO_SEND);

    delay(100); // attendre 100ms
  } else {
    Serial.print("AndroidAccessory not connected at ");Serial.println(millis());

    delay(1000); // attendre 1000ms
  }
}


float getVoltage(int pin)
{
  //analogRead retourne une valeur entre 0 et 1023 or on souhaite obtenir le voltage du pin (entre 0 et 5V)
  return (analogRead(pin) * 0.0048828125); // 0.0048828125 == (5 / 1024)
}

void blinkLedIntegratedXtimes(int nbBlink){
  for (int i=0; i < nbBlink; i++){
    digitalWrite(LED_INTEGRATED, HIGH); //on met le pin sur la position HAUTE pour allumer la led
    delay(1000); //attendre 1000ms
    digitalWrite(LED_INTEGRATED, LOW); //on met le pin sur la position BASSE pour eteindre la led
    delay(1000); //attendre 1000ms
  }
}



