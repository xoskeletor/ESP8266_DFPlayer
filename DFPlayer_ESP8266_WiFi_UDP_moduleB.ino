/*  
  ESP_B
  DFPlayer mini controlled by the Wemos D1mini ESP8266 module using serial
  Play until end of track using BUSY pin to block
  Bi-directional communication via WiFi
  Send and receive UDP packets with host ESP
  Built upon "DFPlayer_ESP8266_noLibrary_CMD.ino" and 
  https://siytek.com/communication-between-two-esp8266/ and 
  https://www.instructables.com/ESP8266-Direct-Data-Communication/  
*/

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "user_interface.h"

#define BaudRate_DF 9600 // DFplayer default is 9600
#define BaudRate_ESP 115200 // ESP8266 serial monitor

// Set WiFi credentials
#define WIFI_SSID "ESP_A"
#define WIFI_PASS "randomatic"
IPAddress remote_IP(192,168,4,1);

// UDP
WiFiUDP UDP;
#define UDP_PORT 4210 // send to ESP_A

// UDP Buffer
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

//////// Pinout on the Wemos D1Mini
#define D0 (16)
#define D1 (5)
#define D2 (4)
#define D3 (0)
#define D4 (2)
#define D5 (14)
#define D6 (12)
#define D7 (13)
#define D8 (15)
#define RX (3)
#define TX (1)

//////// Command Bytes for DFPlayer, static
#define Start_Byte 0x7E // $S
#define Version_Byte 0xFF // VER
#define Command_Length 0x06 // Len
#define End_Byte 0xEF // $O

SoftwareSerial serDF (D1, D2); // in | out WeMos

///////////////////////////////////////////////////// Initialize Parameters
uint8_t volume = 18; // 30 max
char ball = 255;
bool buttonST = 1;

///////////////////////////////////////////////////// Set Up
void setup() {
  // set up serial port with DFPlayer
  serDF.begin(BaudRate_DF);
  delay(100);
  // set up serial port USB;
  Serial.begin(BaudRate_ESP);
  delay(100);
  // set pin modes
  pinMode(D1, INPUT); // From DFplayer TX
  pinMode(D2, OUTPUT); // To DFplayer RX
  pinMode(D5, INPUT); // From DFplayer BUSY pin
  pinMode(D4, INPUT); // Button or LDR sensor
  pinMode(D7, OUTPUT); // Standby indicator LED
  
  // Initialize DFPlayer
  initialize();
  delay(500);
  setVolume(volume);
  delay(500);
  playTrack(1);
  delay(3000); // play duration
  
  // Stop active previous WIFI
  WiFi.disconnect();
  
  // begin WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.mode(WIFI_STA);
  //wifi_set_sleep_type(NONE_SLEEP_T); //LIGHT_SLEEP_T and MODE_SLEEP_T

  // connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  // loop continuously until time-out while WiFi is not connected
  uint8_t i = 0;
  Serial.println();
  while (WiFi.status() != WL_CONNECTED && i < 4) {
    delay(1000);
    Serial.print(++i);
    Serial.print("...");
  }
  
  // Connection to WiFi
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
            Serial.printf("Could not connect after %d seconds", i);
  }
  else {
            Serial.print("Connected! IP address: ");
            Serial.println(WiFi.localIP());
  }
  
  // begin UDP port
  UDP.begin(UDP_PORT);
  Serial.println();
  Serial.println("Starting UDP port ");
  Serial.println(UDP_PORT);

  // turn on standby indicator LED
  digitalWrite(D7, HIGH);
}
///////////////////////////////////////////////////// Main action
void loop() { 
  // Receive and read UDP
  int packetSize = UDP.parsePacket();
  
  // check if packet received
  if (packetSize) {
    digitalWrite(D7, LOW); // indicate busy
    Serial.printf("Received %d bytes from ", packetSize);
    Serial.print(UDP.remoteIP());
    while (busy()==true) {
            delay(100);
    }
    delay(1500);
    playTrack(3);
    
    int n = UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    
    Serial.print("UDP contents received: ");
    Serial.print(packetBuffer);

    // indicate standby ready
    digitalWrite(D7, HIGH);
    

  } else { 
            // when nothing is received...
            // read button input
            buttonST = digitalRead(D4);
            if (buttonST < 1) {
                        Serial.print("Button pressed");
                        while (busy()==true) {
                                    delay(100);
                        }
                        playTrack(2);

                        // send UDP packet
                        UDP.beginPacket(remote_IP, UDP_PORT);
                        Serial.println();
                        Serial.println("Sending UDP...");
                        UDP.write(ball);
                        UDP.endPacket();
                        Serial.println("UDP packet value: ");
                        Serial.print(ball);
                        delay(100);
            }
    }
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
