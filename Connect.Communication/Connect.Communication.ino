#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>

#include "NewRemoteReceiver.h"
#include "NewRemoteTransmitter.h"
#include "System.h"
#include "Plug.h"
#include "arduino_secrets.h"
#include "CircularBuffer.h"
#include "Sensor.h"
#include "F007th.h"
#include "UdpConnect.h"

static const int SENSORS_COUNT = 8;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

System ArduinoSystem;

Sensor* Sensors[SENSORS_COUNT];

UdpConnect UdpSensorConfig;
UdpConnect UdpPlugCommand;
UdpConnect UdpPlugStatus;
UdpConnect UdpSensorData;

CircBufferMacro(CirBuffer, 32);

void setup()
{
  String ipAddress;
  
  // initialize serial communication
  Serial.begin(9600);    

  Serial.println("setup");

  // set the LED pin mode
  pinMode(System::RED, OUTPUT);      
  pinMode(System::YELLOW, OUTPUT);
  pinMode(System::GREEN, OUTPUT);

  NewRemoteReceiver::init(2, 2, ReceivePlugStatus);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();

  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

  	if (status == WL_CONNECTED)
  	{
  		digitalWrite(System::GREEN, HIGH);
  	}
  
    // wait 1 seconds for connection:
    delay(1000);
  
  	if (status == WL_CONNECTED)
  	{
  		digitalWrite(System::GREEN, LOW);
  		digitalWrite(System::RED, LOW);

      Serial.println("");
      Serial.println("WiFi connected..!");

      ipAddress = toString(WiFi.localIP());
      
      Serial.print("Got IP: ");  Serial.println(ipAddress);
  	}
  	else
  	{
  		digitalWrite(System::RED, HIGH);
  
  		ArduinoSystem.ResetArduinoWifiRev2();
  	}
  }

  //connect to your local wi-fi network
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }

  UdpPlugStatus.Init(5001);
  UdpSensorData.Init(5002);
  UdpSensorConfig.Init(5003);
  UdpPlugCommand.Init(5000);

  ArduinoSystem.Initialize(ipAddress, Sensors);

  F007th::Get()->Initialize();

  PrintWifiStatus();                        // you're connected now, so print out the status
}

void loop()
{ 
  String out_data;  

  ArduinoSystem.CheckReboot();

  F007th::Get()->ReadindProcess();

  //Send the Plug status to the Webserver (Interuption)
  if(circ_bbuf_pop(&CirBuffer, &out_data) == 0)
  {
    ArduinoSystem.SendPlugStatus(UdpPlugStatus, out_data.c_str());
    
    Serial.println("Status send : " + out_data);
  }

  //Read the Sensor data periodically from the sensors and send to the WebServer
	ArduinoSystem.ReadSensorsData(UdpSensorData);

  //Read the Sensor configuration from the WebServer
  ArduinoSystem.ReadSensorConfig(UdpSensorConfig);

  //Read the Plug command from the WebServer and send to the plug
  ArduinoSystem.ReceivePlugCommand(UdpPlugCommand);
}

//Interruption when we receive 433Mhz
void ReceivePlugStatus(NewRemoteCode receivedCode)
{
	Plug  plug;

	String in_data = plug.SerializeStatus(receivedCode);

  if (in_data.length() > 0)
  {
    circ_bbuf_push(&CirBuffer, in_data);
  }
}

void PrintWifiStatus() 
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

String toString(const IPAddress& address)
{
  return String() + address[0] + "." + address[1] + "." + address[2] + "." + address[3];
}
