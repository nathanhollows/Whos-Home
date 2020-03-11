
#include "./esppl_functions.h"
#include <time.h>

/*  Define you friend's list size here
 How many MAC addresses are you tracking?
 */
#define LIST_SIZE 4
/*
 * This is your friend's MAC address list
 Format it by taking the mac address aa:bb:cc:dd:ee:ff
 and converting it to 0xaa,0xbb,0xcc,0xdd,0xee,0xff
 */
uint8_t friendmac[LIST_SIZE][ESPPL_MAC_LEN] = {};
/*
 * This is your friend's name list
 * put them in the same order as the MAC addresses
 */
String friendname[LIST_SIZE] = {
   "Billy",
   "Bob",
   "Danny",
   "Frank"
  };

/*
 * Initiating start time in milli-seconds
 */
 unsigned long time_ms;

// start variables package - Skickar 2018 hardware LED for NodeMCU on mini breadboard //
void setup() {
  delay(500);
  Serial.begin(115200);
  esppl_init(cb);
}

bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

void cb(esppl_frame_info *info) {
  for (int i=0; i<LIST_SIZE; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
      Serial.printf("%s is here!\n", friendname[i].c_str());
      time_ms = millis() / 1000;
      Serial.println(time_ms);
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
