#include <time.h>
#include <EasyButton.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "./esppl_functions.h"

/* configuration  wifi */
const char *ssid = "Donkey 2 Electric Boogaloo";
const int DEVICEAMOUNT = 4;

ESP8266WebServer server(80);

uint8_t friendmac[DEVICEAMOUNT][ESPPL_MAC_LEN];
unsigned long time_s;
unsigned long lastSeen[DEVICEAMOUNT];
const byte ledPinList[DEVICEAMOUNT] = {D2, D1, D0, D5};
// Time to light up each LED in seconds
// Optimises over time
int delayTime[DEVICEAMOUNT] = {5, 5, 5, 5};
int LEDState[DEVICEAMOUNT] = {0, 0, 0, 0};

// Used for selection when NodeMCU is an access point
EasyButton AccessButton(0);
EasyButton DeviceButton(0);
bool ACMode = false;
int deviceSelect = 0;

// Sets up the NodeMCU ready for the program
void setup() {
  delay(500);
  Serial.begin(115200);
  esppl_init(cb);
  for (int i = 0; i < DEVICEAMOUNT; i++){
    pinMode(ledPinList[i], OUTPUT);
    lastSeen[i] = 0;
  }
  AccessButton.begin();
  DeviceButton.begin();
  AccessButton.onPressed(accessPointMode);
  DeviceButton.onPressed(deviceReset);

  // Gets the MAC addresses from the EEPROM storage
  EEPROM.begin(24);
  int addr = 0;
  for (int device = 0; device < DEVICEAMOUNT; device++){
    Serial.println("\n");
    for (int macByte = 0; macByte < 6; macByte++){
      EEPROM.get(addr, friendmac[device][macByte]);
      addr++;
    }
  }
}

// Turns the NodeMCU into an access point for devices to connect to
void accessPointMode(){
  Serial.println("Access Mode");
  ACMode = true;
  esppl_sniffing_stop();
  blinkLED(true);
  delay(100);
  blinkLED(true);
  for (int i = 0; i < DEVICEAMOUNT; i++){
    digitalWrite(ledPinList[i], LOW);
  }
  digitalWrite(ledPinList[deviceSelect], HIGH);
  wifi_promiscuous_enable(false);
  delay(200);
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

// Blinks the LED's and lets the user know they can set their devices
void blinkLED(bool accessMode){
  if (accessMode){
    for (int i = 0; i < DEVICEAMOUNT; i++){
      digitalWrite(ledPinList[i], HIGH);
      delay(100);
      digitalWrite(ledPinList[i], LOW);
    }
  } else{
    for (int i = DEVICEAMOUNT - 1; i > -1; i--){
      digitalWrite(ledPinList[i], HIGH);
      delay(100);
      digitalWrite(ledPinList[i], LOW);
    }
  }
}

// Sets the NodeMCU back into sniffing mode and shuts the access point
void resetState(){
  Serial.println("Resetting");
  wifi_promiscuous_enable(true);
  esppl_init(cb);
  delay(200);
  esppl_sniffing_start();
  for (int i = 0; i < DEVICEAMOUNT; i++){
    if (LEDState[i] == 1){
      digitalWrite(ledPinList[i], HIGH);
      lastSeen[i] = millis() / 1000;
    } else{
      digitalWrite(ledPinList[i], LOW);
    }
  }
}

// Let's the user see what device they are setting when in Access mode
void deviceReset(){
  Serial.println("Device Select");
  digitalWrite(ledPinList[deviceSelect], LOW);
  deviceSelect += 1;
  if (deviceSelect >= DEVICEAMOUNT){
    deviceSelect = 0;
  }
  digitalWrite(ledPinList[deviceSelect], HIGH);
}

// Checks the client connected to the Access point and stores their MAC at the selected LED
bool client_status() {
  struct station_info *stat_info;
  stat_info = wifi_softap_get_station_info();
  
  if (stat_info != NULL) {
    digitalWrite(ledPinList[deviceSelect], LOW);
    delay(200);
    digitalWrite(ledPinList[deviceSelect], HIGH);
    delay(200);
    digitalWrite(ledPinList[deviceSelect], LOW);
    delay(200);
    digitalWrite(ledPinList[deviceSelect], HIGH);
    int storeAddr = deviceSelect * 6;
    for (int i = 0; i < 6; i++){
      friendmac[deviceSelect][i] = stat_info->bssid[i];
      EEPROM.put((storeAddr + i), friendmac[deviceSelect][i]);
      EEPROM.commit(); 
    }
    stat_info = STAILQ_NEXT(stat_info, next);
    delay(300);
    return true;
  }
  return false;
}

// Part of MAC address checking
bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++){
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

// Prints information to Serial about what devices are being detected
void printWhosHere(int person){
  time_s = millis();
  int difference = time_s / 1000 - lastSeen[person];

  // Each phone broadcasts at different intervals
  // Increase delay time if LED is not staying on for long enough
  // Ignores diffferences greater than 10 minutes 
  if (lastSeen[person] != 0 && difference < 600 && difference > delayTime[person]) {
    delayTime[person] += (difference - delayTime[person]) * 1.25;
  }

  lastSeen[person] = time_s / 1000;
  digitalWrite(ledPinList[person], HIGH);
  LEDState[person] = 1;
}

// Checks the packets in the air against the MAC addresses stored
void cb(esppl_frame_info *info) {
  for (int i = 0; i < DEVICEAMOUNT; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
      printWhosHere(i);
    }
  }
}

// Main program loop that checks if people are here and if the FLASH button is pressed
void loop() {
  esppl_sniffing_start();
  while (true) {
    while (ACMode == false){
      for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
        esppl_set_channel(i);
        esppl_process_frames();
      }
      for (int i = 0; i < DEVICEAMOUNT; i++){
        time_s = millis() / 1000;
        if (time_s - lastSeen[i] > delayTime[i]){
          digitalWrite(ledPinList[i], LOW);
          LEDState[i] = 0;
        }
      }
      AccessButton.read();
    }
    unsigned int AccessTime = millis() / 1000;
    while (AccessTime + 30 > (millis() / 1000)){
      DeviceButton.read();
      server.handleClient();
      if (client_status()){
        break;
      }
      delay(100);
    }
    if (ACMode == true){
      blinkLED(false);
      delay(100);
      blinkLED(false);
      resetState();
      ACMode = false;
    }
  }
}
