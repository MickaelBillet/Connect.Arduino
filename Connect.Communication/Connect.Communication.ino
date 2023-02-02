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
UdpConnect UdpConnection;

CircBufferMacro(CirBuffer, 32);

void setup()
{
  String ipAddress = "";
  // initialize serial communication
  Serial.begin(9600);    
  Serial.println("setup");

  // set the LED pin mode
  pinMode(System::RED, OUTPUT);      
  pinMode(System::YELLOW, OUTPUT);
  pinMode(System::GREEN, OUTPUT);

  NewRemoteReceiver::init(2, 2, ReceivePlugStatus);
  ipAddress = ArduinoSystem.Launch(ssid, pass);
  if (ipAddress == "")
  {
    ArduinoSystem.ResetArduinoWifiRev2();
  }
  else
  {  
    UdpPlugCommand.Init(5001);  
    UdpPlugStatus.Init(5002);
    UdpSensorData.Init(5003);
    UdpSensorConfig.Init(5004);
    UdpConnection.Init(5005);
    ArduinoSystem.Initialize(ipAddress, Sensors);
    F007th::Get()->Initialize();
    PrintWifiStatus();              
  }
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
  ArduinoSystem.ReadSensorConfig(UdpSensorConfig, UdpConnection);

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
