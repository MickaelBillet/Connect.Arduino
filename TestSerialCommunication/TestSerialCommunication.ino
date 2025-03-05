#include "TestCommunication.h"
#include "Plug.h"
#include "Sensor.h"
#include "SerializerJson.h"
#include "SoftwareSerial.h"

static const int SENSORS_COUNT = 10;

static const short NOT_STARTED = 0;
static const short STARTING = 1;
static const short STARTED = 2;

int status = NOT_STARTED;
TestCommunication Test;

void setup()
{
  status = STARTING;
 
  // initialize serial communication
  Serial.begin(115200);    
}

void loop()
{ 
  Test.SendSystemStatus(TestCommunication::SystemStartedOrder);

  Test.SendPlugStatus();

  delay(20); 

  Test.SendData();

  delay(20);

  Test.SendEvent();

  delay(20);
}
