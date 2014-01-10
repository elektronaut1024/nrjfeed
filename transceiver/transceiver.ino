#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xCD, 0xAE, 0x0F, 0xFE, 0xED };

EthernetClient client;
IPAddress forwardHostIP(80,74,143,90);

int serialBufferLength = 0;
int serialBufferLimit = 1000;
char serialBuffer[1001] = "";

boolean startedSerial = false;
  
void setup() {
  if (Ethernet.begin(mac) == 0) {
    for(;;) ; //failed to retrieve IP from DHCP
  }
  
  Serial.begin(9600);
  Serial1.begin(9600);
  
  if ( Serial ) Serial.println("started");
}

void readSerialData() {
  if ( !Serial1 ) return;
  if ( Serial1.available() <= 0 ) return;
 
  if ( Serial ) {
    Serial.print(Serial1.available());
    Serial.println(" data available.");
  }
  
  while ( Serial1.available() > 0 && serialBufferLength < serialBufferLimit ) {
    serialBuffer[serialBufferLength++] = Serial1.read();
    serialBuffer[serialBufferLength] = 0;
  }
  
  if ( Serial ) {
    Serial.print("Content-Length: ");
    Serial.println(serialBufferLength);
    Serial.print("data=");
    Serial.println(serialBuffer);
    Serial.println();
  }
  
  if ( forwardSerialData() ) {
    serialBufferLength = 0;
    if ( Serial ) {
      Serial.println();
      Serial.println("finished successfully");
    }
  } else {
     if ( Serial ) {
       Serial.println("failed to transmit data");
     }
  }
}

boolean forwardSerialData() {
  if ( !client.connect(forwardHostIP, 80) ) {
    return false;
  }

  client.println("POST /nrjfeed/ HTTP/1.1");
  client.println("Host: www.elektronaut.ch");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(serialBufferLength+5);
  client.println();
  client.print("feed=");
  client.println(serialBuffer);
  
  while ( client.connected() ) {
    if ( client.available() ) {
      char c = client.read();
      if ( Serial ) Serial.write(c);
    }
  }
  
  client.stop();
  
  return true;
}

void loop() {
  readSerialData();
  delay(1000);
}
