#include <LWiFi.h>
#include <LWiFiServer.h>
#include <LWiFiClient.h>
#include "rgb_lcd.h"
#include <LBattery.h>
#include <LBT.h> 
#include <LBTServer.h>
#include <LStorage.h>
#include <LFlash.h>

#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP according to your WiFi AP configuration
#define Drv LFlash

void printWifiStatus();
void bridgeConnection();
void bluetoothConnection();
char* parseJsonCharValue(char* json, char* key);
int parseJsonIntValue(char* json, char* key);

LWiFiServer server(5401);
rgb_lcd lcd;
String config;
LFile configFile;
char* wifi_password;
char* wifi_ap;
int wifi_auth;

void setup()
{
  Drv.begin();
  LBTServer.begin((uint8_t*)"WiFiTcpServer");
  LWiFi.begin();
  Serial1.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setRGB(255, 0, 0);
  if (Drv.exists("wifi.txt")) 
    {
      configFile = Drv.open("wifi.txt");
      if(configFile)
      {
        configFile.seek(0); 
        char wifiConfigJson[configFile.size()+1];
        int i =0;               
        while (configFile.available()) 
        {  
          wifiConfigJson[i]=configFile.read();
          i++;
        }
        wifiConfigJson[i]= 0;
        Serial.println(wifiConfigJson);
        wifi_auth = parseJsonIntValue(wifiConfigJson, "wifi_auth");        
        wifi_ap= parseJsonCharValue(wifiConfigJson, "wifi_ap");
        wifi_password= parseJsonCharValue(wifiConfigJson, "wifi_password");        
      } 
    }
  if(wifi_ap && wifi_password && wifi_auth>-1)
  {
    LWiFiEncryption lWiFiEncryption = LWIFI_OPEN;
    if(wifi_auth==1)lWiFiEncryption = LWIFI_WPA;
    else if(wifi_auth==1)lWiFiEncryption = LWIFI_WEP;
    // keep retrying until connected to AP
    Serial.println("Connecting to AP");
    while (0 == LWiFi.connect(wifi_ap, LWiFiLoginInfo(lWiFiEncryption, wifi_password)))
    {
     delay(1000);
    }
    printWifiStatus();
    Serial.println("Start Server");
    server.begin();
    Serial.println("Server Started");
  }
  else
  {
    lcd.print("INVALID AUTHENTICATION");
  }
}

void loop()
{
  bridgeConnection();
  bluetoothConnection();
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

void bridgeConnection()
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

void bluetoothConnection()
{
  if(LBTServer.connected())
  {
    config = LBTServer.readString();
    if (Drv.exists("wifi.txt")) 
    {
      Drv.remove("wifi.txt"); 
    }
    else 
    {
      configFile = Drv.open("wifi.txt", FILE_WRITE);
      if (configFile) 
      {
        configFile.println(config);
        configFile.close();
      }      
    }    
    LBTServer.write("acknowledged");
  }
  else
  {
    LBTServer.accept(5);
  }  
}

char* parseJsonCharValue(char* json, char* key)
{
  size_t jsonLength = strlen(json);
  size_t keyLength = strlen(key);
  char* workJson = (char*)calloc(jsonLength, sizeof(char));
  strcpy(workJson, json);
  for (size_t i = 0; i < jsonLength; i++)
  {
    if (strncmp(workJson, key, keyLength) == 0)
    {
      workJson = workJson + keyLength + 5;           
      size_t valueLength = 0;
      for (size_t k = 0; k < strlen(workJson); k++)
      {
        if (strncmp(workJson, "\"", 1) == 0)
        {
          valueLength = k;
          break;
        }
        workJson++;
      }
      workJson = workJson - valueLength;
      char* returnValue = (char*)calloc(valueLength+1, sizeof(char));
      strncpy(returnValue, workJson, valueLength-1);      
      return returnValue;
    }
    workJson++;
  }
  return NULL;
}

int parseJsonIntValue(char* json, char* key)
{
  size_t jsonLength = strlen(json);
  size_t keyLength = strlen(key);
  char* workJson = (char*)calloc(jsonLength, sizeof(char));
  strcpy(workJson, json);
  for (size_t i = 0; i < jsonLength; i++)
  {
    if (strncmp(workJson, key, keyLength) == 0)
    {
      workJson = workJson + keyLength + 2;
      size_t valueLength = 0;
      for (size_t k = 0; k < strlen(workJson); k++)
      {
        if (strncmp(workJson, "}", 1) == 0 || strncmp(workJson, ",", 1) == 0)
        {
          valueLength = k;
          break;
        }
        workJson++;
      }
      valueLength++;
      workJson = workJson - valueLength;
      char* returnValue = (char*)calloc(valueLength + 1, sizeof(char));
      strncpy(returnValue, workJson, valueLength);
      return strtol(returnValue, &returnValue, valueLength - 1);
    }
    workJson++;
  }
  return -1;
}
