//#include <Keyboard_jp.h>
//#include <Keyboard.h>
#include <KeyboardPico.h>
#include <KeyboardLayout_ja_JP.h>

#include "98_key_map.h"

#define TX  3  // TXD = !RST
#define RX  1
#define TX_DUMMY 16
#define I_RDY 4 // inverted
#define RX_DUMMY 6
// !RTY is alwasy high means no retry

// Softwaare Serial does not support UART with parity!!!

void setup() {
  Keyboard.begin(KeyboardLayout_ja_JP);
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(TX, OUTPUT);
  pinMode(I_RDY, OUTPUT);
  pinMode(TX_DUMMY, OUTPUT);
  pinMode(RX,INPUT);

  //Serial.begin(1200);

  Serial1.setRX(1); //1 13 17
  Serial1.setTX(TX_DUMMY); // 0 12 16
  Serial1.begin(19200,SERIAL_8O1);
  //Serial2 TX 4 8
  //Serial2 RX 5 9
  
  //////////////////////////////
  // Setup
  ///////////////////////////
  //Reset
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(I_RDY, HIGH);
  digitalWrite(TX,LOW);
  delayMicroseconds(15);
  digitalWrite(TX,HIGH);

  digitalWrite(I_RDY, LOW);

}

void loop() {
  
  
  digitalWrite(I_RDY, HIGH);
  delayMicroseconds(40);
  digitalWrite(I_RDY, LOW);
  

  if(Serial1.available() > 0){
    
    digitalWrite(LED_BUILTIN, LOW);
    signed char data = Serial1.read();
    //Serial.print(data);
    if(data < 0) {    // break
      data += 128;
      //Keyboard.release(PC98_to_ascii[data]);
      Keyboard.releaseID(PC98_to_UsageID[data]);
      
      /*
      Serial.print(data,HEX);
      Serial.println("released");
      */
    }
    else {  // Make 
      //Keyboard.press(PC98_to_ascii[data]);
      Keyboard.pressID(PC98_to_UsageID[data]);
     
     
      /*
      Serial.print(data,HEX);
      Serial.println("pressed");
      */
    }
    // Kana and CapsLock dosen't send break code
    if(data == 114){ // Kana
      Keyboard.writeID(53);  /* Grave Accent and Tilde `~ Hankaku Zenkaku */
    }  
    if(data == 113){ // CapsLock
      Keyboard.writeID(57);
      //Keyboard.release(PC98_to_ascii[data]);
    }  
  

  }
  else{
    digitalWrite(LED_BUILTIN, HIGH);
  }
  

}

