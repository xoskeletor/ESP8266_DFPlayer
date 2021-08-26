/*
  DFPlayer_ESP8266_noLibrary_CMD
  DFPlayer mini controlled by the Wemos D1mini ESP8266 module using serial
  First play selected track, then play next tracks
  Check DFPlay status using BUSY pin
  Play each track until the end
*/

#include <SoftwareSerial.h>
#define BAUD_RATE 9600 //DFplayer default is 9600

//////// Pin names on the Wemos D1Mini
#define D8 (15)
#define D5 (14)
#define D7 (13)
#define D6 (12)
#define D4 (2)
#define D2 (4)
#define D1 (5)
#define D0 (16)
#define RX (3)

//////// Command Bytes for DFPlayer, static
#define Start_Byte 0x7E // $S
#define Version_Byte 0xFF // VER
#define Command_Length 0x06 // Len
#define End_Byte 0xEF // $O

SoftwareSerial serDF (D1, D2); // RX | TX

///////////////////////////////////////////////////// Initialize Parameters
uint8_t volume = 12; // 30 max
uint8_t trackNum = 1;

///////////////////////////////////////////////////// Set Up
void setup() {
  serDF.begin(BAUD_RATE);
  Serial.begin(115200);
  delay(3000);
  pinMode(D1, INPUT); // From DFplayer TX
  pinMode(D2, OUTPUT); // To DFplayer RX
  pinMode(D5, INPUT); //From BUSY pin on DFplayer
  initialize();
  delay(100);
  setVolume(volume);
  playTrack(trackNum);
  delay(3000); // First play duration
}

///////////////////////////////////////////////////// Main Loop
void loop() {
  if (serDF.available()) {
    int inByte = serDF.read();
    Serial.write(inByte);
  }
  while (busy() == true) {
    delay(100);
  }
  playNext();
}

///////////////////////////////////////////////////// DFPlayer Functions

//////// Initialize
void initialize() {
  execute_CMD(0x3F, 0, 0, 0); // Send initialization parameters
}

//////// Set Volume
void setVolume(uint8_t vlm) {
  execute_CMD(0x06, 0, 0, vlm); // Volume level 0x00~0x30
  delay(50);
}

//////// Play/Resume
void playBack() {
  execute_CMD(0x0D, 0, 0, 0);
  delay(50);
}

//////// Play Specified Track
void playTrack(uint8_t trkNum) {
  execute_CMD(0x03, 0, 0, trkNum); // Specify track number 0-2999
  delay(50);
}

//////// Play Next Track
void playNext() {
  execute_CMD(0x01, 0, 0, 0);
  delay(50);
}

//////// Monitor MP3 Play Status using BUSY pin on the DFplayer
bool busy() {
  uint8_t DFstat = digitalRead(D5);
  if (DFstat == LOW) {
    return true; // is playing, busy
  }
  else {
    return false; // is not playing
  }
}

/* This function builds the command structure
// In DFplayer functions, set 2nd parameter to [1: Query Feedback, 0: no Feedback]
*/
void execute_CMD(byte CMD, byte Feedback, byte Par1, byte Par2) {
  // Calculate the checksum (2 bytes)
  int16_t checksum = -(Version_Byte + Command_Length + CMD + Feedback + Par1 + Par2);

  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Feedback, Par1, Par2, checksum >> 8, checksum & 0xFF, End_Byte};

  // Send the command line to the module
  for (byte k = 0; k < 10; k++) {
    serDF.write(Command_line[k]);
  }
}
