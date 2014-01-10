int inputPin = 0;
int impulsePin = 7;
int errorPin = 8;
int powerPin = 6;

//Sensor State
int lightValue = 0;
int lightThreshold = 10;
boolean isBright = true;

boolean initialized = false;

//Time
unsigned long now = 0;

//Transmission
int transmissionIndicator = 0;
unsigned long transmissionIdx = 0;

//Transmission interval
unsigned long lastTransmission = 0;
unsigned long rolloverInterval = 10000;

//Data accumulation before transmitting it
int dataLength = 0;
int maxDataLength = 5;
unsigned long data[5];

void transmit(){
  transmissionIdx++;
  
  Serial.print("[");
  Serial.print(transmissionIdx);
  Serial.print(",");
  Serial.print(lastTransmission);
  Serial.print(",[");
  
  for ( int i=0; i<dataLength; i++ ) {
    if ( i > 0 ) Serial.print(",");
    
    Serial.print(data[i]-lastTransmission);
  }
  
  Serial.println("]]");
}

void impulseOn(){
  if ( isBright == true ) {  
    return;
  }
  isBright = true;
  
  digitalWrite(impulsePin,HIGH);
  
  data[dataLength] = now;
  dataLength++;
}

void impulseOff(){
  if ( isBright == false ) return;
  isBright = false;
  
  digitalWrite(impulsePin,LOW);
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

void blinkTransmissionIndicator(){
  if ( transmissionIndicator <= 0 ) return;
  
  transmissionIndicator--;
  
  if ( transmissionIndicator <= 0 ) {
    digitalWrite(errorPin,LOW);
  } else {
    if ( transmissionIndicator % 10 > 5 ) digitalWrite(errorPin,LOW);
    else digitalWrite(errorPin,HIGH);
  }
}

void measure() {
  now = millis();
  
  lightValue = analogRead(inputPin);
  
  if ( lightValue > lightThreshold ) impulseOn();
  else impulseOff();
  
  blinkTransmissionIndicator();
  
  rollover();
  
  delay(10);
}
/*
int minLightValue = -1;
int minLightValueHigh = -1;

int maxLightValue = 0;
int maxLightValueLow = 0;

int middleLightValue = 0;

void tuneIn() {
  lightValue = analogRead(inputPin);
  
  if ( minLightValue < 0 ) {
    minLightValue = lightValue;
    maxLightValue = lightValue;
    minLightValueHigh = lightValue;
    maxLightValueLow = lightValue;
  } else {
    minLightValue = min(minLightValue,lightValue);
    maxLightValue = max(maxLightValue,lightValue);
    
    int newMiddleLightValue = round((maxLightValue - minLightValue) / 2);
    if ( newMiddleLightValue != middleLightValue ) {
      middleLightValue = newMiddleLightValue;
      minLightValueHigh = 0;
      maxLightValueLow = lightValue;
    } else {
      if ( lightValue < middleLightValue) minLightValueHigh = max(minLightValueHigh,lightValue);
      else maxLightValueLow = min(maxLightValueLow,lightValue);
    }
  }
  
  if( millis() > 3000 ) {
    initialized = true;
    //lightThreshold = middleLightValue;
  }
  
  delay(10);
}
*/

void setup() {
  pinMode(errorPin,OUTPUT);
  pinMode(impulsePin,OUTPUT);
  pinMode(powerPin,OUTPUT);
  
  digitalWrite(errorPin,LOW);
  digitalWrite(impulsePin,LOW);
  digitalWrite(powerPin,HIGH);
  
  Serial.begin(9600);
}

void loop() {
  measure();
  /*if ( initialized ) measure();
  else tuneIn();
  */
}
