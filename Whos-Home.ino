#include "./esppl_functions.h"
#include <time.h>
#include <EasyButton.h>
#include "Settings.h"

uint8_t friendmac[DEVICEAMOUNT][ESPPL_MAC_LEN] = DEVICEMAC;
String friendname[DEVICEAMOUNT] = NAME;
unsigned long time_s;
unsigned long lastSeen[DEVICEAMOUNT];
const byte ledPinList[DEVICEAMOUNT] = PINS;
// Time to light up each LED in seconds
// Optimises over time
int delayTime[DEVICEAMOUNT] = {5, 5, 5, 5};
int LEDState[DEVICEAMOUNT] = {0, 0, 0, 0};

// Used for selection when NodeMCU is an access point
EasyButton AccessButton(0);
EasyButton DeviceButton(0);
bool ACMode = false;
int deviceSelect = 0;

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
}

void blinkLED(){
  for (int i = 0; i < DEVICEAMOUNT; i++){
    digitalWrite(ledPinList[i], HIGH);
    delay(100);
    digitalWrite(ledPinList[i], LOW);
  }
}

void resetState(){
  Serial.println("Resetting");
  esppl_sniffing_start();
  for (int i = 0; i < DEVICEAMOUNT; i++){
    Serial.printf("Name: %s, Check: %d\n", friendname[i].c_str(), LEDState[i]);
    if (LEDState[i] == 1){
      digitalWrite(ledPinList[i], HIGH);
      lastSeen[i] = millis() / 1000;
    } else{
      digitalWrite(ledPinList[i], LOW);
    }
  }
}

void deviceReset(){
  Serial.println("Device Select");
  digitalWrite(ledPinList[deviceSelect], LOW);
  deviceSelect += 1;
  if (deviceSelect >= DEVICEAMOUNT){
    deviceSelect = 0;
  }
  digitalWrite(ledPinList[deviceSelect], HIGH);
}

void accessPointMode(){
  Serial.println("Access Mode");
  ACMode = true;
  esppl_sniffing_stop();
  blinkLED();
  delay(100);
  blinkLED();
  for (int i = 0; i < DEVICEAMOUNT; i++){
    digitalWrite(ledPinList[i], LOW);
  }
  digitalWrite(ledPinList[deviceSelect], HIGH);
}

bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

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

  Serial.printf("%d\t%d\t%s\r\n", time_s, delayTime[person], friendname[person].c_str());
}

void cb(esppl_frame_info *info) {
  for (int i = 0; i < DEVICEAMOUNT; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
      printWhosHere(i);
    }
  }
}

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
      delay(100);
    }
    if (ACMode == true){
      resetState();
    }
  }
}
