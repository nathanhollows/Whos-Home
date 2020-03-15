#include "./esppl_functions.h"
#include <time.h>
#include "Settings.h"

uint8_t friendmac[DEVICEAMOUNT][ESPPL_MAC_LEN] = DEVICEMAC;
String friendname[DEVICEAMOUNT] = NAME;
unsigned long time_s;
unsigned long lastSeen[DEVICEAMOUNT];
const byte ledPinList[DEVICEAMOUNT] = PINS;
// Time to light up each LED in seconds
// Optimises over time
int delayTime[DEVICEAMOUNT] = {5, 5, 5, 5};

void setup() {
  delay(500);
  Serial.begin(115200);
  esppl_init(cb);
  for (int i = 0; i < DEVICEAMOUNT; i++){
    lastSeen[i] = 0;
    pinMode(ledPinList[i], OUTPUT);
  }
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
  if (lastSeen[person] != 0 && difference < 600000 && difference > delayTime[person]) {
    delayTime[person] += (difference - delayTime[person]) * 1.25;
  }

  lastSeen[person] = time_s / 1000;
  digitalWrite(ledPinList[person], HIGH);

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
    for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
      esppl_set_channel(i);
      esppl_process_frames();
    }
    for (int i = 0; i < DEVICEAMOUNT; i++){
      time_s = millis() / 1000;
      if (time_s - lastSeen[i] > delayTime[i]){
        digitalWrite(ledPinList[i], LOW);
      }
    }
  }
}
