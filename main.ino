//#include <Keyboard_jp.h>
//#include <Keyboard.h>
#include <KeyboardPico.h>
#include <KeyboardLayout_ja_JP.h>
#include <SoftwareSerial.h>

#include "98_key_map.h"

#define TX  3  // TXD = !RST
#define RX  1
#define TX_DUMMY 16
#define I_RDY 4 // inverted
#define RX_DUMMY 6
// !RTY is alwasy high means no retry

// Softwaare Serial does not support UART with parity!!!

volatile unsigned char Kbd_command = 0b01110000;
volatile unsigned char prev_Kbd_command = 0b01110000;
#define LED_COMMAND 0b10011101

SoftwareSerial KbdSend(RX_DUMMY, TX, false);
bool NewType = false;

void myled(bool numlock, bool capslock, bool scrolllock, bool compose, bool kana, void *cbData) {
    unsigned char C = 0;
    unsigned char K = 0;
    if(capslock){
      C = 1;
    }
    if(kana) {
      K = 1;
    }
    Kbd_command = 112 + 8*K + 4*C;
}

void setup() {
  Keyboard.begin(KeyboardLayout_ja_JP);
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(TX, OUTPUT);
  pinMode(I_RDY, OUTPUT);
  pinMode(TX_DUMMY, OUTPUT);
  pinMode(RX,INPUT);

  Serial1.setRX(RX); //1 13 17
  Serial1.setTX(TX_DUMMY); // 0 12 16
  Serial1.begin(19200,SERIAL_8O1);
 
  Serial.begin(1200);

  KbdSend.begin(19200);
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
  /*
      second byte:
      0111 xxxx
          |||`- NumLock
          ||`-- ?
          |`--- CapsLock
          `---- Kana
  */

// for newer keyboard

  Keyboard.onLED(myled);
  
  /*
  // check keyboard type
  KbdSend.write(0b10011111); // keyboard type command, if newer type, it resposes FA, A0, 80, 
  unsigned char data = Serial1.read();
  NewType = true;
  if(data != 0xFA){
    NewType = false;
  }else 
  {
    data = Serial1.read();
  if(data != 0xA0){
    NewType = false;
  }else{
    data = Serial1.read();
    if(data != 0x80)
  {
  NewType = false;
  }
  }
  }
  */

 // unsigned char inhibitRepeating[2] = {0x9C, 0x70};

 // KbdSend.write(inhibitRepeating,2);


}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(I_RDY, LOW);

/*
if(NewType){
  if(prev_Kbd_command != Kbd_command){
  KbdSend.write(LED_COMMAND);
  KbdSend.write(Kbd_command);
  prev_Kbd_command = Kbd_command;
    }
  }
*/

  if(Serial1.available() > 0){
    
    digitalWrite(LED_BUILTIN, LOW);
    signed char data = Serial1.read();
    digitalWrite(I_RDY, HIGH);
    delayMicroseconds(40);
    
    if(data < 0) {    // break
      data += 128;
      Keyboard.releaseID(PC98_to_UsageID[data]);
    }
    else {  // Make 
      //Keyboard.press(PC98_to_ascii[data]);
      Keyboard.pressID(PC98_to_UsageID[data]);
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
    digitalWrite(I_RDY, HIGH);
    delayMicroseconds(40);
  }
  

}
