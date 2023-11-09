#include "NewRemoteReceiver.h"
#include "NewRemoteTransmitter.h"
#include "CircularBuffer.h"

CircBufferMacro(CirBuffer, 32);

void setup()
 {
  Serial.begin(9600);    
  Serial.println("setup");

  NewRemoteReceiver::init(0, 2, ReceivePlugStatus);
}

void loop()
 {
  String out_data;  

  //Send the Plug status to the Webserver (Interuption)
  if(circ_bbuf_pop(&CirBuffer, &out_data) == 0)
  {

  }

}

//Interruption when we receive 433Mhz
void ReceivePlugStatus(NewRemoteCode receivedCode)
{
  String status;

	if (receivedCode.switchType == 0)
	{
		status = "OFF";
	}
	else if (receivedCode.switchType == 1)
	{
		status = "ON";
	}
  
	String in_data = "{\"Status\":\"" + status + "\", \"Address\":\"" + receivedCode.address + "\", \"Unit\":\"" + receivedCode.unit + "\"}";
  Serial.println(in_data);
  Serial.println(receivedCode.period);
  if (in_data.length() > 0)
  {
    circ_bbuf_push(&CirBuffer, in_data);
  }
}
