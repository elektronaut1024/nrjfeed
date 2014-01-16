#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xCD, 0xAE, 0x0F, 0xFE, 0xED };

boolean HTTP = true;
EthernetClient client;
IPAddress forwardHostIP(80,74,143,90);
int forwardingDelay = 2000;

int serialBufferLength = 0;
int serialBufferLimit = 1000;
char serialBuffer[1001] = "";

boolean startedSerial = false;

unsigned int waitInc = 1;

boolean isStarted = false;

boolean stopHTTP(){
  if ( !isStarted ) return false;
  
  client.stop();
  isStarted = false;
  
  return true;
}

boolean connectHTTP(){
  if ( client.connected() ) return true;
  stopHTTP();
  
  if ( !client.connect(forwardHostIP, 80) ) {
    if ( Serial ) {
      Serial.print("failed to connect. waiting ");
      Serial.print(waitInc);
      Serial.println("ms and retry again");
    }
    delay(waitInc);
    if ( waitInc < 32000 ) waitInc = waitInc * 2;
    
    return false; //discard any data because it is lost anyway
  }
  
  isStarted = true;
  
  if ( Serial ) Serial.println("Connected");
  
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
    if ( HTTP && !client.connected() && stopHTTP() ) {
      if ( Serial ) Serial.println("Server disconnected");
    }
    return false; //skip update
  }
  
  if ( Serial ) Serial.print(data);
  
  if ( !HTTP ) return true;
  if ( !connectHTTP() ) return false;
  
  forwarding = true;
    
  client.println("POST /nrjfeed/ HTTP/1.1");
  client.println("Host: www.elektronaut.ch");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(length+5);
  client.println();
  client.print("feed=");
  client.println(data);
  
  lastForward = now;
  
  return true;
}

int finishHeaderPos = 0;
char finishHeader[] = "\r\n";
boolean finishedHeader = false;

int connectionCloseHeaderPos = 0;
char connectionCloseHeader[] = "\r\nConnection: close";

void finishForwarding(){
  if ( !forwarding ) return;
  
  if ( !client.available() ) return;
  if ( !finishedForwarding() ) return;
  
  forwarding = false;
}

boolean finishedForwarding() {
  while ( client.connected() && client.available() ) {
    char c = client.read();
    
    if ( connectionCloseHeader[connectionCloseHeaderPos] == c ) {
      connectionCloseHeaderPos++;
      if ( connectionCloseHeader[connectionCloseHeaderPos] == 0 ) {
        if ( Serial ) Serial.println("Server will close connection");
        return true;
      }
    } else {
      connectionCloseHeaderPos = 0;
    }
 
    if ( finishHeader[finishHeaderPos] == c ) {
      finishHeaderPos++;
      if ( finishHeader[finishHeaderPos] == 0 ) {
        if ( finishedHeader ) { //if a header was finished before, the whole header section is now terminated
          if ( Serial ) Serial.println("Continue");
          return true;
        } else {
          finishHeaderPos = 0;
          finishedHeader = true;
        }
      }
    } else {
      finishHeaderPos = 0;
      finishedHeader = false;
    }
  }
  
  if ( Serial ) Serial.println("Not finished");
  
  return false;
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
