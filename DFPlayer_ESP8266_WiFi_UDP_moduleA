/*  
  ESP_A
  DFPlayer mini controlled by the Wemos D1mini ESP8266 module using serial
  Play until end of track using BUSY pin to block
  Bi-directional communication via WiFi
  Send and receive UDP packets with client ESP
  Built upon "DFPlayer_ESP8266_noLibrary_CMD.ino" and 
  https://siytek.com/communication-between-two-esp8266/ and 
  https://www.instructables.com/ESP8266-Direct-Data-Communication/  
*/

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define BaudRate_DF 9600 // DFplayer default is 9600
#define BaudRate_esp 115200 // ESP8266 serial monitor

// Set Access Point credentials
#define AP_SSID "ESP_A"
#define AP_PASS "randomatic"

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

// UDP
WiFiUDP UDP;
#define UDP_PORT 4210 // receive from ESP_B

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

SoftwareSerial serDF (D1, D2); // in | out WeMos D1Mini

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
  Serial.begin(115200); 
  delay(100);
  // set pin modes
  pinMode(D1, INPUT); // From DFplayer TX
  pinMode(D2, OUTPUT); // To DFplayer RX
  pinMode(D5, INPUT); //From DFplayer BUSY pin
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
  
  // Set the WiFi to both access point & station mode
  WiFi.mode(WIFI_AP_STA);
  
  // Begin access point
  Serial.println("Starting access point...");
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println(WiFi.localIP());
    
  // Begin UDP port
  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port ");
  Serial.println(UDP_PORT);
  
  // Turn on LED indicate stand-by
  digitalWrite(D7, HIGH);

}

///////////////////////////////////////////////////// Main Loop
void loop() { 
  // Receive and parse UDP
  int packetSize = UDP.parsePacket();
  
  // check if packet received
  if (packetSize){
    digitalWrite(D7, LOW); // LED off to indicate busy
    Serial.printf("Received %d bytes from ", packetSize);
    Serial.print(UDP.remoteIP());
    delay(1500); 
            playTrack(3);
            // To ensure the mp3 is played from start to finish
      while (busy() == true) { 
        delay(100);
      }
    
    int n = UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.print("UDP packet contents: ");
    Serial.print(packetBuffer);
    packetBuffer[n] = 0;

    // LED on to indicate standby ready
    digitalWrite(D7, HIGH);
    

  } else { 
            // when nothing is received...
            // read button input
            buttonST = digitalRead(D4);
            if (buttonST < 1) {
                        Serial.print("Button pressed");
                        // To ensure the mp3 is played from start to finish
                        while (busy() == true) {
                            delay(100);
                        }
                        playTrack(2);
    
                        // send UDP packet
                        UDP.beginPacket(UDP.remoteIP(), UDP_PORT);
                        Serial.println("UDP start...");
                        UDP.write(ball);
                        UDP.endPacket();
                        Serial.println("Sending UDP to the other IP: ");
                        Serial.print(WiFi.localIP());
                        Serial.println("UDP content: ");
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
  if (DFstat < 1) {
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
