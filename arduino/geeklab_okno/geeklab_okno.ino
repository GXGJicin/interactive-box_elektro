// Komunikacni protokol prevzaty z http://forum.arduino.cc/index.php?topic=225329.msg1810764#msg1810764

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_PIN 6
#define INTERVAL 10
//#define DEBUG
#define INFO

uint32_t timer;
unsigned long curMillis;
unsigned long prevReplyToPCmillis = 0;
unsigned long replyToPCinterval = 1000;
unsigned long previousMillis = 0;

unsigned long previousMillisZhasinani = 0;

const byte buffSize = 50;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;
char messageFromPC[buffSize] = {0};

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

const byte dev_id = 100;
const byte fw_ver = 200;

byte rgb[7];
boolean pismeno[8];

//============

void setup () {
  Serial.begin(9600);  
  sensors.begin(); // OneWire
  sensors.setResolution(11); // presnost mereni DS18B20 9=0.5°C  10=0.25°C  11=0.125°C  12=0.0625°C
    
  #ifdef INFO
    Serial.println(F("\nStart!"));
    Serial.print(F("ID zarizeni: "));
    Serial.println(dev_id);    
    Serial.print(F("Verze firmware: "));
    Serial.println(fw_ver);
    Serial.println();    
  #endif
}

//============

void loop() {
  curMillis = millis();
  getDataFromPC();
  replyToPC();
  if(curMillis - previousMillis >= INTERVAL*1) {
    previousMillis = curMillis;
    analogWrite(9, rgb[0]);//Geek R
    analogWrite(6, rgb[1]);//Geek G
    analogWrite(5, rgb[2]);//Geek B
    analogWrite(3, rgb[3]);//Lab R
    analogWrite(10, rgb[4]);//Lab G
    analogWrite(11, rgb[5]);//Lab B
    
    digitalWrite(7,  pismeno[0]);//G
    digitalWrite(A3, pismeno[1]);//E
    digitalWrite(A2, pismeno[2]);//E
    digitalWrite(A1, pismeno[3]);//K
    digitalWrite(A0, pismeno[4]);//L
    digitalWrite(13, pismeno[5]);//A
    digitalWrite(12, pismeno[6]);//B
  }
  if(curMillis - previousMillisZhasinani >= 30000) {
    previousMillisZhasinani = curMillis;
    for (byte i=0; i <=6; i++) {
      pismeno[i] = 0;
    }
  }
}

//=============

void getDataFromPC() {
  // receive data from PC and save it into inputBuffer
    
  if (Serial.available() > 0) {

    char x = Serial.read();

      // the order of these IF clauses is significant
      
    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      parseData();
    }
    
    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) { 
      bytesRecvd = 0; 
      readInProgress = true;
    }
  }
}

//=============

void replyToPC() {
  if (newDataFromPC) {
    newDataFromPC = false;
    #ifdef DEBUG
      Serial.print(F("< POZADAVEK: "));
      Serial.print(messageFromPC);
      Serial.println(">");
    #endif
  }
}

//============

void parseData() {  //           pouzivane prikazy:       <RGB,128,128,128,128,128,128>  <PISMENO,1,1,1,1,1,1,1> <TEPLOTA> <INFO>
  // split the data into its parts
   
  char * strtokIndx; // this is used by strtok() as an index
  
  strtokIndx = strtok(inputBuffer, ",");      // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC
  
  if( strstr("RGB", messageFromPC) != NULL ) {
     Serial.println("RGB");
     byte i=0;
      while(strtokIndx != NULL) {
        strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
        int value = atoi(strtokIndx);     // convert this part to an integer
        rgb[i] = byte(value);
        i++;
      }
  } else if( strstr("PISMENO", messageFromPC) != NULL ) { // <RGB>
     Serial.println("PISMENO");
     byte i=0;
      while(strtokIndx != NULL) {
        strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
        int value = atoi(strtokIndx);     // convert this part to an integer
        pismeno[i] = byte(value);
        previousMillisZhasinani = curMillis;
        i++;
      }
  } else if( strstr("TEPLOTA", messageFromPC) != NULL ) { // <DUMP>
    teplota();
  } else if( strstr("INFO", messageFromPC) != NULL ) { // <INFO>
    info();    
  } else {
    Serial.println("NEPLATNA_DATA");
  }
}

//=============

void teplota () {
    byte count = sensors.getDeviceCount();
    DeviceAddress da;
    sensors.requestTemperatures();
    if (!count == 0) {
    #ifdef DEBUG
      Serial.print(F("Celkem_DS18B20="));
      Serial.println(count);  
    #endif
    for (byte i=0; i<count; i++) {
      byte id[8];
      for (byte e=0; e<8; e++) {        
        sensors.getAddress(da, i);   
        id[e] = da[e];  
        Serial.print(da[e], HEX);
        if (e != 7) Serial.print(F(":"));
      }
        Serial.print(F("="));
        Serial.println(sensors.getTempCByIndex(i) );
      }
    }    
}

//============

static void info() {
  Serial.print(F("ID_zarizeni="));
  Serial.println(dev_id);    
  
  Serial.print(F("Verze_firmware="));
  Serial.println(fw_ver);
  
  Serial.print(F("RGB="));
  for (int i=0; i<6; i++) {
    Serial.print(rgb[i]);
    if (i != 5) Serial.print(F(","));
  }
  Serial.println();
  
  Serial.print(F("PISMENA="));
  for (int i=0; i<7; i++) {
    Serial.print(pismeno[i]);
    if (i != 6) Serial.print(F(","));
  }  
  Serial.println();
  
}

//============

