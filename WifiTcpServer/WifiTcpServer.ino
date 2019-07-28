#include <LWiFi.h>
#include <LWiFiServer.h>
#include <LWiFiClient.h>
#include "rgb_lcd.h"
#include <LBattery.h>
#define WIFI_AP "Toronto Pearson Wi-Fi"
#define WIFI_PASSWORD "$unWing#2012" 
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP according to your WiFi AP configuration

LWiFiServer server(5401);
rgb_lcd lcd;
void setup()
{
  LWiFi.begin();
  Serial1.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setRGB(255, 0, 0);
  // keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }

  printWifiStatus();
  Serial.println("Start Server");
  server.begin();
  Serial.println("Server Started");
}

void loop()
{
  delay(500);
  LWiFiClient client = server.available();
  if (client)
  {    
    while (client.connected())
    {
      while (client.available())
      {
        Serial1.write(client.read());                        
      }
      while (Serial1.available()) 
      {
        client.write(Serial1.read());
      }      
    }
  }
}

void printWifiStatus()
{
  lcd.clear();
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(LWiFi.SSID());  
  lcd.print(LWiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = LWiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  lcd.setCursor(0, 1);
  lcd.print(ip);

  Serial.print("subnet mask: ");
  Serial.println(LWiFi.subnetMask());

  Serial.print("gateway IP: ");
  Serial.println(LWiFi.gatewayIP());

  // print the received signal strength:
  long rssi = LWiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("Battery Level: ");
  Serial.print(LBattery.level());
  Serial.println("%");;
}
