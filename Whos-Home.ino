/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <Pinger.h>
extern "C"
{
  #include <lwip/icmp.h> // needed for icmp packet definitions
}

#include "Settings.h"

const char* ssid     = STASSID;
const char* password = STAPSK;
bool isHome = false;

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

  pinger.OnReceive([](const PingerResponse& response)
  {
    if (response.ReceivedResponse)
    {
      Serial.printf(
        "Reply from %s: bytes=%d time=%lums TTL=%d\r\r\n",
        response.DestIPAddress.toString().c_str(),
        response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
        response.ResponseTime,
        response.TimeToLive);
        digitalWrite(LED_BUILTIN, LOW);
        isHome = true;
    }
    else
    {
      Serial.printf("Request timed out.\r\n");
        digitalWrite(LED_BUILTIN, HIGH);
      isHome = false;
    }
    return true;
  });
  
  pinger.OnEnd([](const PingerResponse& response)
  {
    // Evaluate lost packet percentage
    float loss = 100;
    if(response.TotalReceivedResponses > 0)
    {
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
    if(response.TotalReceivedResponses > 0)
    {
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
    if(response.DestMacAddress != nullptr)
    {
      Serial.printf(
        "    MAC address: " MACSTR "\r\n",
        MAC2STR(response.DestMacAddress->addr));
    }
    if(response.DestHostname != "")
    {
      Serial.printf(
        "    DNS name: %s\r\n",
        response.DestHostname.c_str());
    }

    return true;
  });

}

void loop() {  
  
  Serial.printf("\r\n\r\nPinging '192.168.178.56'\r\n");
  pinger.Ping("192.168.178.56");
  
  if(!isHome)
  {
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

  delay(300000);
}
