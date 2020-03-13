#include "./esppl_functions.h"
#include <time.h>
#include "Settings.h"

/*
 * This is your friend's MAC address list
 Format it by taking the mac address aa:bb:cc:dd:ee:ff
 and converting it to 0xaa,0xbb,0xcc,0xdd,0xee,0xff
 */
uint8_t friendmac[DEVICEAMOUNT][ESPPL_MAC_LEN] = DEVICEMAC;
/*
 * This is your friend's name list
 * put them in the same order as the MAC addresses
 */
String friendname[DEVICEAMOUNT] = NAME;
unsigned long time_s;
unsigned long Present[DEVICEAMOUNT];
int delayTime[DEVICEAMOUNT] = {100, 100, 100, 100}; //How many seconds until the person can be considered present

/*
 * Here you define what pins you will be using
 * This has to be done manually for how many people you want on the circuit
 * Make sure that the LED's are in order for correct representation
 */
const byte ledPinList[DEVICEAMOUNT] = {D2, D1, D0, D3};

// start variables package - Skickar 2018 hardware LED for NodeMCU on mini breadboard //
void setup() {
  delay(500);
  Serial.begin(115200);
  esppl_init(cb);
  for (int i = 0; i < DEVICEAMOUNT; i++){
    Present[i] = 0;
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
  Present[person] = time_s;
  Serial.printf("%d %s\n", time_s, friendname[person].c_str());
  digitalWrite(ledPinList[person], HIGH);
}

void cb(esppl_frame_info *info) {
  for (int i = 0; i < DEVICEAMOUNT; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
      printWhosHere(i);
    }
  }
}

void loop() { // I didn't write this part but it sure looks fancy.
  esppl_sniffing_start();
  while (true) {
    for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
      esppl_set_channel(i);
      while (esppl_process_frames()) {
        //
      }
    }
    for (int i = 0; i < DEVICEAMOUNT; i++){
      time_s = millis() / 1000;
      if ((Present[i] + delayTime[i]) < time_s){
        digitalWrite(ledPinList[i], LOW);
      }
    }
  }
}
