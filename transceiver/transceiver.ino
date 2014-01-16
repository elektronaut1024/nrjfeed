#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xCD, 0xAE, 0x0F, 0xFE, 0xED };

boolean HTTP = true;
EthernetClient client;
IPAddress forwardHostIP(80,74,143,90);
int forwardingDelay = 15000;

int serialBufferLength = 0;
int serialBufferLimit = 1000;
char serialBuffer[1001] = "";

boolean startedSerial = false;

unsigned int waitInc = 1;

boolean connectHTTP(){
  if ( !client.connect(forwardHostIP, 80) ) {
    if ( Serial ) {
      Serial.print("failed to connect. waiting ");
      Serial.print(waitInc);
      Serial.println("ms and retry again");
    }
    delay(waitInc);
    if ( waitInc < 32000 ) waitInc = waitInc * 2;
    
    return false;
  }
  
  waitInc = 1;
  
  return true;
}

char packageEnd[] = "]]\r\n";
int packageEndPos = 0;

void readSerialData() {
  if ( !Serial1 ) return;
  if ( Serial1.available() <= 0 ) return;
  
  boolean forward = false;
   
  while ( Serial1.available() > 0 ) {
    if ( serialBufferLength >= serialBufferLimit ) {
      forward = true;
      Serial.println("buffer full");
      serialBufferLength = 0;
      break;
    }
    char c = Serial1.read();
    serialBuffer[serialBufferLength++] = c;
    serialBuffer[serialBufferLength] = 0;
    
    if ( packageEnd[packageEndPos] == c ) {
      packageEndPos++;
      if ( packageEnd[packageEndPos] == 0 ) {
        forward = true;
        break;
      }
    } else {
      packageEndPos = 0;
    }
  }
  
  if ( !forward ) return; //nothing worth forwarding
  if ( !forwardData(serialBuffer,serialBufferLength) ) return; //not forwarded for some reason

  serialBufferLength = 0;
}

boolean forwarding = false;
unsigned long lastForward = 0;

boolean forwardData(const char* data, const int length) {
  if ( forwarding ) return false; //already forwarding something
  
  unsigned long now = millis();
  if ( now - lastForward < forwardingDelay ) {
    return false; //skip update
  }
  
  if ( Serial ) Serial.print(data);
  
  if ( !HTTP ) return true;
  if ( !connectHTTP() ) return false;
  
  forwarding = true;
    
  client.println("POST /nrjfeed/ HTTP/1.1");
  client.println("Host: www.elektronaut.ch");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(length+5);
  client.println();
  client.print("feed=");
  client.println(data);
  
  lastForward = now;
  
  return true;
}

void finishForwarding(){
  if ( !forwarding ) return;
  
  while ( client.connected() && client.available() ) client.read();
  
  if ( !client.connected() ) {
    forwarding = false;
    client.stop();
  }
}

void setup() {
  Serial1.begin(9600);
  
  Serial.begin(9600);
  
  if ( HTTP ) {
    delay(1000);
    if ( Serial ) Serial.println("Trying for Ethernet");
    
    if (Ethernet.begin(mac) == 0) {
      if ( Serial ) Serial.println("No Ethernet");
      HTTP = false;
    }
  }
  
  if ( Serial ) Serial.println("started");
}

void loop() {
  readSerialData();
  finishForwarding();
}
