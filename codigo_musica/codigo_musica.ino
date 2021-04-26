#include <SD.h>  
#include <SPI.h>
#include <TMRpcm.h>  
#include <Wire.h>
#define pinSD 10

TMRpcm tmrpcm;   //Se crea un objeto de la librería TMRpcm
int x=0;
void setup(){
  Wire.begin(4);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600); 
  tmrpcm.speakerPin = 9; //Se define el pin en el que está conectada la bocina
  if (!SD.begin(pinSD)) {  // see if the card is present and can be initialized:
    return;   //No hacer nada si no se pudo leer la tarjeta
  }
}

void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c); 
  }
  x = Wire.read(); // receive byte as a character
  Serial.print(x);         // print the character
}

void loop() {
  switch(x){
    case 1:
      tmrpcm.play("inicio.wav");
      delay(100);
      break;
    case 2: 
      tmrpcm.play("waka.wav");
      delay(500);
      break;
    case 3:
      tmrpcm.play("muere.wav");
      delay(100);
      break;
    case 4:
      tmrpcm.play("inter.wav");
      delay(100);
      break;
    }
}
