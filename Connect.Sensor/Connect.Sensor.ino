#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

#include "arduino_secrets.h"
#include "System.h"
#include "Sensor.h"
#include "UdpConnect.h"

static const int SENSORS_COUNT = 8;

int status = WL_IDLE_STATUS;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

Sensor* Sensors[SENSORS_COUNT];

ESP8266WebServer server(80);  

System Esp8266System;

UdpConnect UdpSensorConfig;
UdpConnect UdpSensorData;
 
void setup() 
{
  String ipAddress;
  
  Serial.begin(9600);
  delay(100);
  
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //Counter for the connection try
  int cnt=30;
  
  //connect to your local wi-fi network
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  //check wi-fi is connected to wi-fi network
  while ((WiFi.status() != WL_CONNECTED) && (cnt > 0))
  {
    delay(1000);
    Serial.print(".");

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("");
      Serial.println("WiFi connected..!");

      ipAddress = toString(WiFi.localIP());
      
      Serial.print("Got IP: ");  Serial.println(WiFi.localIP().toString());
    }

    cnt--;
  }

  if ((WiFi.status() != WL_CONNECTED)&&(cnt == 0))
  {
    Esp8266System.ResetEsp8266();
  }

  UdpSensorData.Init(5002);
  UdpSensorConfig.Init(5003);

  Esp8266System.Initialize(ipAddress, Sensors);

  // Call the 'handleRoot' function when a client requests URI "/"  
  server.on("/", handle_OnConnect);

  // When a client requests an unknown URI (i.e. something other than "/"), call
  server.onNotFound(handle_NotFound);

  server.begin();
  
  Serial.println("HTTP server started");
}

void loop() 
{
  //Listen for HTTP requests from clients
  server.handleClient();

  Esp8266System.CheckReboot();

  //Read the Sensor data periodically from the sensors and send to the WebServer
  Esp8266System.ReadSensorsData(UdpSensorData);

  //Read the Sensor configuration from the WebServer
  Esp8266System.ReadSensorConfig(UdpSensorConfig);

  delay(10000);
}

void handle_OnConnect() 
{
  if(Sensors[0] != nullptr)
  { 
    server.send(200, "text/html", SendHTML(Sensors[0]->Temperature,Sensors[0]->Humidity,Sensors[0]->Pressure)); 
  }
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature,float humidity,float pressure)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Houilles</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP8266 Houilles Weather Station</h1>\n";
  ptr +="<p>Temperature: ";
  ptr +=temperature;
  ptr +="&deg;C</p>";
  ptr +="<p>Humidity: ";
  ptr +=humidity;
  ptr +="%</p>";
  ptr +="<p>Pressure: ";
  ptr +=pressure;
  ptr +="hPa</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String toString(const IPAddress& address)
{
  return String() + address[0] + "." + address[1] + "." + address[2] + "." + address[3];
}
