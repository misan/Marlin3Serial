#include "MarlinBTSerial.h"

char* sp_change_speed=":010203030BB8";
char* sp_stop=":01030108";
char* sp_start=":01030101";
char* sp_start_rev=":01030111";

void spindleOn();
void spindleOnRev();
void spindleSpeed(float speed);
void spindleOff();


#include <Arduino.h>




void send(String msg) {
  // activate driver
  digitalWrite(21,HIGH); // digitalWrite(9,HIGH);
  delay(50);
  MYSERIAL2.print(msg);
  delay(1);
  digitalWrite(21,LOW); // digitalWrite(9,LOW);
}

char hexa(char byte) { // return hexa value of that ASCII char
  if(byte<='9') return byte & 0x0f;
  if(byte<='F') return (byte & 0x0f) + 9; // A=0x41 --> 0x0a
}
  
char toHexa(int i) {
 if(i<10) return 0x30 | i; 
 return 0x41 + i - 10; 
}

char crc(char* buffer) {
 int l=strlen(buffer);
 int i=1;
 int chk=0;
 while(i<l) { //Serial.print(hexa(buffer[i])<<4|hexa(buffer[i+1]),HEX); 
           chk+= ( hexa(buffer[i])<<4|hexa(buffer[i+1]) ); i+=2; 
           //Serial.print(" ");
         }
// Serial.print(":"); Serial.println(chk,HEX);
 // Serial.println(0x100-chk,HEX);
 return (0x100-chk) & 0xff; 
}
  
void query(char* cmd) {
  char lrc = crc(cmd);
  String msg = cmd;
  msg+=toHexa((lrc&0xf0)>>4);
  msg+=toHexa(lrc&0x0f);
  msg+="\r\n";
  //Serial.print(msg);
  send(msg);
}
  
  void setSpeed(int v) {
  // Serial.print("Set new speed "); Serial.println(v,DEC);
  char* n=sp_change_speed;
  v*=1.666; // convert from Hz/100 tp RPM
  n[9]=toHexa((v/256)>>4);
  n[10]=toHexa((v/256)&0xf);
  n[11]=toHexa((v&0xff)>>4);
  n[12]=toHexa((v&0xff)&0xf);
  //Serial.println(n);
  query(n); // set new speed
}


void spindleOn() {query(sp_start); }
void spindleOnRev() {query(sp_start_rev); }
void spindleSpeed(float speed) {setSpeed(speed); }
void spindleOff() {query(sp_stop); }

