int inputPin = 0;

//Sensor State
int lightValue = 0;
int lightThreshold = 10;
boolean isBright = true;

boolean initialized = false;

//Time
unsigned long now = 0;

//Transmission
unsigned long transmissionIdx = 0;

//Transmission interval
unsigned long lastTransmission = 0;
unsigned long rolloverInterval = 10000;

//Data accumulation before transmitting it
int dataLength = 0;
int maxDataLength = 5;
unsigned long data[5];

void setup() {
  Serial.begin(9600);
}

unsigned long lastImpulse = 0;

void loop() {
  now = millis();
 
 if ( now - lastImpulse > 100 ) {
   lightValue = lightThreshold + 1;
   lastImpulse = now;
 } else {
   lightValue = 0;
 }
 
 //lightValue = analogRead(inputPin);
  
  if ( lightValue > lightThreshold ) impulseOn();
  else impulseOff();
  
  rollover();
  
  delay(10);
}

/**
 * A package should consist of less than 56 bytes,
 * which should always fit into the transmitter buffer
 **/
void transmit(){
  transmissionIdx++;
  
  Serial.flush(); //if the previous package was not fully sent, this waits for it to be sent out.
  
  Serial.print("[");
  Serial.print(transmissionIdx); //max 10 chars (4bytes)
  Serial.print(",");
  Serial.print(lastTransmission); //max 10 chars (4bytes)
  Serial.print(",[");
  
  for ( int i=0; i<dataLength; i++ ) {
    if ( i > 0 ) Serial.print(",");
    
    Serial.print(data[i]-lastTransmission);//max 5 chars
  }
  
  Serial.println("]]");
}

void impulseOn(){
  if ( isBright == true ) return;
  isBright = true;
  
  data[dataLength] = now;
  dataLength++;
}

void impulseOff(){
  if ( isBright == false ) return;
  isBright = false;
}

void rollover() {
  if ( dataLength <= 0 ) return;
  
  if (
    now > rolloverInterval + lastTransmission
    || maxDataLength <= dataLength
   ) {
     transmit();
     
     lastTransmission = now;
     dataLength=0;
   }
}
