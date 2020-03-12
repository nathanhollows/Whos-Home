
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
int time_until_update = 100; //How many seconds until the person can be considered present

// start variables package - Skickar 2018 hardware LED for NodeMCU on mini breadboard //
void setup() {
  delay(500);
  Serial.begin(115200);
  esppl_init(cb);
  for (int i = 0; i < DEVICEAMOUNT; i++){
    Present[i] = 0;
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
  time_s = millis() / 1000;
  if (((Present[person] + time_until_update) < time_s) || (Present[person] == 0)){
    Present[person] = time_s;
    Serial.printf("%s is here!\n", friendname[person].c_str());
    Serial.println(time_s);
  }
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
  }
}
