#include "NewRemoteReceiver.h"
#include "NewRemoteTransmitter.h"
#include "System.h"
#include "Plug.h"
#include "arduino_secrets.h"
#include "CircularBuffer.h"
#include "Sensor.h"
#include "F007th.h"
#include "SerializerJson.h"
#include "SoftwareSerial.h"

static const int SENSORS_COUNT = 10;

static const short NOT_STARTED = 0;
static const short STARTING = 1;
static const short STARTED = 2;

int status = NOT_STARTED;
System ArduinoSystem;
SoftwareSerial raspberrySerial(2, 3);

Sensor* Sensors[SENSORS_COUNT];

CircBufferMacro(CirBuffer, 32);

void setup()
{
  status = STARTING;
  
  // initialize serial communication
  Serial.begin(115200);    
  raspberrySerial.begin(9600);

  // set the LED pin mode
  pinMode(System::RED, OUTPUT);      
  pinMode(System::YELLOW, OUTPUT);
  pinMode(System::GREEN, OUTPUT);

  NewRemoteReceiver::init(4, 2, ReceivePlugStatus);
  
  ArduinoSystem.Initialize(Sensors);
    
  F007th::Get()->Initialize();
}

void loop()
{ 
  delay(100);

  String out_data;  

  if (status == STARTING)
  {
    ArduinoSystem.SendSystemStatus(raspberrySerial, System::SystemStartedOrder);
    status = STARTED;
  }

  ArduinoSystem.CheckReboot();
  F007th::Get()->ReadindProcess();

  //Send the Plug status to the Webserver (Interuption)
  if(circ_bbuf_pop(&CirBuffer, &out_data) == 0)
  {
    ArduinoSystem.SendPlugStatus(raspberrySerial, out_data.c_str());
  }

  //Read the Sensor data periodically from the sensors and send to the WebServer
  ArduinoSystem.ReadSensorsData(raspberrySerial);

  //Catch the Sensor event from the sensors and send to the WebServer
  ArduinoSystem.ReadSensorsEvent(raspberrySerial);

  ArduinoSystem.ReadFromWebServer(raspberrySerial);   
}

//Interruption when we receive 433Mhz
void ReceivePlugStatus(NewRemoteCode receivedCode)
{
  String in_data = Serializer::SzPlugStatus(System::PlugStatusMsg, receivedCode);
  if (in_data.length() > 0)
  {
    circ_bbuf_push(&CirBuffer, in_data);
  }
}
