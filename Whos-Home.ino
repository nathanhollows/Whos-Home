/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <Pinger.h>
#include <time.h>
#include "./esppl_functions.h"

extern "C"
{
  #include <lwip/icmp.h> // needed for icmp packet definitions
}

#include "Settings.h"

const char*   ssid = STASSID;
const char*   password = STAPSK;
String        names[DEVICEAMOUNT] = NAME;
const char*   devices[] = DEVICEIP;
const int     devicenum = DEVICEAMOUNT;
uint8_t       friendmac[DEVICEAMOUNT][ESPPL_MAC_LEN] = DEVICEMAC;
bool          isHome = false;
unsigned long time_ms;
int           devicechecknum = 0;

Pinger pinger;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinger.OnReceive([](const PingerResponse& response){
    if (response.ReceivedResponse){
      Serial.printf(
        "Reply from %s: bytes=%d time=%lums TTL=%d\r\r\n",
        response.DestIPAddress.toString().c_str(),
        response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
        response.ResponseTime,
        response.TimeToLive);
        digitalWrite(LED_BUILTIN, LOW);
        isHome = true;
    } else{
      Serial.printf("Request timed out.\r\n");
        digitalWrite(LED_BUILTIN, HIGH);
      isHome = false;
    }
    return true;
  });
  
  pinger.OnEnd([](const PingerResponse& response) {
    // Evaluate lost packet percentage
    float loss = 100;
    if(response.TotalReceivedResponses > 0){
      loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
    }
    
    // Print packet trip data
    Serial.printf(
      "Ping statistics for %s:\r\n",
      response.DestIPAddress.toString().c_str());
    Serial.printf(
      "    Packets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\r\n",
      response.TotalSentRequests,
      response.TotalReceivedResponses,
      response.TotalSentRequests - response.TotalReceivedResponses,
      loss);

    // Print time information
    if(response.TotalReceivedResponses > 0) {
      Serial.printf("Approximate round trip times in milli-seconds:\r\n");
      Serial.printf(
        "    Minimum = %lums, Maximum = %lums, Average = %.2fms\r\n",
        response.MinResponseTime,
        response.MaxResponseTime,
        response.AvgResponseTime);
    }
    
    // Print host data
    Serial.printf("Destination host data:\r\n");
    Serial.printf(
      "    IP address: %s\r\n",
      response.DestIPAddress.toString().c_str());
    if(response.DestMacAddress != nullptr) {
      Serial.printf(
        "    MAC address: " MACSTR "\r\n",
        MAC2STR(response.DestMacAddress->addr));
    }
    if(response.DestHostname != "") {
      Serial.printf(
        "    DNS name: %s\r\n",
        response.DestHostname.c_str());
    }
    return true;
  });
  esppl_init(cb);
}


/*
 * The two functions maccmp and cb check the packets
 */
bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

void cb(esppl_frame_info *info) {
  for (int i = 0; i < 4; i++){
    if (maccmp(info->sourceaddr, friendmac[devicechecknum]) || maccmp(info->receiveraddr, friendmac[devicechecknum])) {
      Serial.printf("%s is here!\n", names[devicechecknum].c_str());
      time_ms = millis() / 1000;
      Serial.println(time_ms);
    }
  }
}

void loop() {  
  //Cycle through names and devices and pings them all
  Serial.printf("Checking who is connected...\n");
  
  for (devicechecknum = 0; devicechecknum < devicenum; devicechecknum++){
    Serial.printf("Name: %s\n", names[devicechecknum].c_str());

    Serial.printf("Pinging device on %s\n", devices[devicechecknum]);
    pinger.Ping(devices[devicechecknum]);
    delay(10000);

    // If ping doesn't work, sniff packets -- NEED TO WORK ON AND FIGURE OUT TIMES
    // Remove the while loops maybe?
    if(!isHome){
      Serial.printf("Checking by packets to see if %s is here\n", names[devicechecknum].c_str());
      for (int j = 0; j < 4; j++){
        esppl_sniffing_start();
        for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
          esppl_set_channel(i);
          while (esppl_process_frames()) {
            //
          }
        }
        delay(5000);
        esppl_sniffing_stop();
        Serial.printf("J:%d\n", j);
      }
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
      }
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Finished checking, starting delay...");
  devicechecknum = 0;
  delay(300000);
}
