//import des librairies
#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>
#include <SPI.h>
 
//definition des constantes
#define INT_SIZE 4 // un int est stocke sur 4 byte
#define FLOAT_SIZE 4 // un float est stocke sur 4 byte
#define IR1_SIZE FLOAT_SIZE // taille (en byte) prevu pour mettre les donnees du capteur infra-rouge 1 : un float
#define MSG_LENGTH_TO_SEND IR1_SIZE // taille (en byte) prevu pour le message qui transitera de l'arduino a l'android = taille des donnes des deux capteurs

#define MSG_LENGTH_TO_RECEIVED 8 //taille (en byte) prevu pour le message qui transitera de l'android a l'arduino

#define INPUT_PIN_IR1 A9 //le capteur infra-rouge 1 est branche sur le pin A9

#define SLAVE_SELECT_PIN 53 //sur l'arduino mega adk il est sur le pin 53

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

//Pointeur vers une zone memooire afin de la lire en tant que float ou en tant que tableau de byte
union FloatToByteArray
{
  float    f;
  uint32_t byteArray[FLOAT_SIZE];
};
FloatToByteArray ir1Converter;

//Pointeur vers une zone memooire afin de la lire en tant que int ou en tant que tableau de byte
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
  pinMode(INPUT_PIN_IR1, INPUT); //on configure le pin du capteur infraèrouge 1 comme etant une entree
  pinMode (SLAVE_SELECT_PIN, OUTPUT);
  SPI.begin();
  digitalPotWrite(0); // init a la resistance maximale
}

//apres setup(), loop est lancee en boucle indefiniment (equivalent d'un while(true){} )
void loop() {
  if (acc.isConnected()) {
    //Partie reception donnees depuis android
    //len va contenir la longeur du message recu de l'android
    len = acc.read(msgReceived, MSG_LENGTH_TO_RECEIVED, 1);
    //si on a bien reçu des donnes de l'android
    if(len > 0){
      memcpy(intConverter.byteArray, msgReceived, INT_SIZE); //on utilise le tableau de byte du converter pour y mettre les donnees du telephone
      int vitesse = intConverter.i;
      Serial.print("Recu : ");Serial.println(vitesse);

        digitalPotWrite(0);
        delay(40);        
        digitalPotWrite(vitesse); // on modifie la resistance du MCP41100 en consequence
        delay(40);
    }
    
    //partie IR
    float IR1_Voltage = getVoltage(INPUT_PIN_IR1); //obtention du voltage

    //Serial.print("IR1 :");Serial.print(IR1_Voltage);Serial.print("V \n");

    ir1Converter.f = IR1_Voltage; // on set le voltage du capteur infra-rouge 1 dans le converter

    // on recupere les donnees du capteur infra-rouge 1 sous forme de tableau de byte
    memcpy(msgToSend, ir1Converter.byteArray, FLOAT_SIZE); // copie de FLOAT_SIZE octet (byte) du tableau de byte dans le message a envoyer a l'android

    //Partie envoi a android
    acc.write(msgToSend, MSG_LENGTH_TO_SEND);
    
    if(len = 0){
      //rien recu d'android
      delay(100); // attendre 100ms
    }
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


int digitalPotWrite(int value)
{
  Serial.print("digitalPotWrite ");Serial.print(value);Serial.print(" at ");Serial.println(millis());
  digitalWrite(SLAVE_SELECT_PIN, LOW);
  SPI.transfer(0x11); 
  SPI.transfer(value);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
}



