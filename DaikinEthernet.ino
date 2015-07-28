//
//  DaikinEthernet.ino
//  
//
//  Created by danny on 07/01/15.
//  Copyright (c) 2015 danny. All rights reserved.
//
//  https://github.com/danny-source/

#include <SPI.h>
#include <Ethernet.h>
#include <IRdaikin.h>
#include <DHT.h>

//feature define
#define DAIKIN_AC
#define SENSOR_TEMPERATURE
#define SENSOR_HUMIDITY

#define SENSOR_DHT
#ifdef SENSOR_DHT
#define DHTTYPE DHT11
#define DHTPIN 2     // what pin we're connected to
DHT dht(DHTPIN, DHTTYPE);
#endif

//Ethernet Declare
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 9, 59);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
EthernetClient client;

//Http parse declare
String requestStringBuffer;
long requestPackageLength;
bool isCommand;
bool isSerialCommand;
int commandMethod;//1:POST 2:GET
String commandAction;
String commandHttpAction;
boolean currentLineIsBlank = true;
//Schdule Timer
long startTime;
long endTime;
int timerCounter;

//Command declare
#define NUMBER_OF_COMMAND 5
char *command_cmd[NUMBER_OF_COMMAND] = {"/ft?","/ac?","/ac=","/tp?","/hm?"};
int command_len[NUMBER_OF_COMMAND] = {4,6,16,6,6};
#define FEATURE_READ 0
#define AC_READ 1
#define AC_WRITE 2
#define TEMPERATURE_READ 3
#define HUMIDITY_READ 4

//Daikin declare
int daikinCommandBuffer[1][6] = {{0,0,0,0,0,25}};
int daikinCommandTemp[6] = {0,0,0,0,0,0};
#define NUMBER_OF_DAIKIN 1
IRdaikin irdaikin;
//sensor
#define NUMBER_OF_TEMPERATURE 1
#define NUMBER_OF_HUMIDITY 1

float sensorTemperature[] = {0};
float sensorHumidity[] = {99};

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  //ethernet shield init
  Ethernet.begin(mac, ip);
  Serial.println(Ethernet.localIP());
  server.begin();  
  commandMethod = 0;
  commandAction = "";
  //timer init
  startTime = 0;
  //dht sensor init
#ifdef SENSOR_DHT  
  dht.begin();
#endif  
}


void loop() {
  
  //
 endTime = millis();
  if ( (endTime - startTime) > 1000) {
    startTime=endTime;
    timerCounter++;
    
    //every 1 second
    schdule1Second();
    //every 2 second
    if ((timerCounter%2) == 0) {
      schdule2Second();
    }
     //clear counter
    if (timerCounter >= 60) {
      timerCounter = 0;
    }    
  }
  //check http
  client = server.available();
  if (client) {
      requestPackageLength=0;
      requestStringBuffer = String ();
      isCommand= true;
      while (client.connected()) {
            if (client.available()) {
              if (parseHTTP_Method_Action()) {
                  
                  progressHTTP_Action();
                  break;
              }
            }
      }
  }else {
   //no avaiable
  }
}

//http
bool parseHTTP_Method_Action(){
        char c = client.read();
        //Serial.print(c);
        if (isCommand) {
           requestStringBuffer = requestStringBuffer + c;          
        }
        // check method
        if (requestStringBuffer.startsWith(F("GET ")) && isCommand) {
          commandMethod = 1;
          requestStringBuffer = "";
        }else if (requestStringBuffer.startsWith(F("POST ")) && isCommand) {
          commandMethod = 2;
          requestStringBuffer = "";          
        }
        //check method and action end of line.
        if (requestStringBuffer.endsWith(F(" HTTP/"))  && (isCommand)) {
          isCommand = false;          
          commandAction = requestStringBuffer.substring(0,requestStringBuffer.length() - 6);
          
        } 
        //request end of lin  
         if (c == '\n' && currentLineIsBlank) {
          return true;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        } 
        return false;
}

void progressHTTP_Action(){
          if (commandMethod>0) {
            //Serial.print(commandAction);
            //Serial.print(commandAction.length());
            //Serial.println();
            int commandCnt = 0;
            while (true) {
              //Serial.print(command_cmd[commandCnt]);
              if (commandAction.startsWith(command_cmd[commandCnt]) && (command_len[commandCnt] == commandAction.length())) {
                progressCommandAction(commandCnt);
                break;
              }
              
              //check command number,if no matched ,send error
              commandCnt = commandCnt + 1;
              if (commandCnt > (NUMBER_OF_COMMAND - 1)) {
                sendHTTPJSONCMD_ERROR();
                break;
              }
            }            
            //Serial.println("progress end");
            commandAction = "";
           }else {
             sendHTTPJSONERROR();
           }     
}

void progressCommandAction(int commandNumberOfAction) {
  switch (commandNumberOfAction) {
    case FEATURE_READ:
      getFeature();
      break;
    case AC_READ:
      getACStatus();
      break;
    case AC_WRITE:
      setACStatus();
      break;
    case TEMPERATURE_READ:
      getTemperature();
      break;
    case HUMIDITY_READ:
      getHumidity();
      break;    
    default:
      sendHTTPJSONERROR();
  }
}

void getFeature() {
   int idx = checkStartWithQuestionMark();
   if (idx == -1) {
     sendHTTPJSONCMD_ERROR();
     return;
   }   
   int acNumber = getNumberOfAC(idx);
   if (acNumber == -1) {
     sendHTTPJSONCMD_ERROR();
     return;
   }  
    HTTP_JSON_Header();
    client.print(F("{\"status\":\"ok\""));
    client.print(F(","));
    client.print(F("\"version\":\""));
    client.print(F("1"));
    client.print(F("\""));
    client.print(F(","));   
    client.print(F("\"ac\":\""));
#ifdef DAIKIN_AC    
    client.print(F("1"));
#else
    client.print(F("0"));
#endif
    client.print(F("\""));
    client.print(F(","));
    client.print(F("\"humidity\":\""));
#ifdef SENSOR_HUMIDITY    
    client.print(F("1"));
#else
    client.print(F("0"));
#endif
    client.print(F("\""));
    client.print(F(","));
    client.print(F("\"temperature\":\""));
#ifdef SENSOR_TEMPERATURE    
    client.print(F("1"));
#else
    client.print(F("0"));
#endif
    client.print(F("\""));
    client.println(F("}"));        
    client.flush();
    client.stop();   
}

void setACStatus() {
   int idx = checkStartWithEqual();
   if (idx == -1) {
     sendHTTPJSONCMD_ERROR();
     return;
   }   
   int acNumber = getNumberOfAC(idx);
   if (acNumber == -1) {
     sendHTTPJSONCMD_ERROR();
     return;
   }
   //Serial.println(acNumber);
   Serial.println("--");
   //check and store to temp
   for (int i = 0;i < 6; i++) {
     int startIdx = idx + (i*2);
     int intn = commandAction.substring(startIdx,startIdx + 2).toInt();
     //Serial.println(commandAction.substring(startIdx,startIdx + 2));
     //
      if (checkDaikinCommand(i ,intn) == -1) {
        sendHTTPJSONCMD_ERROR();
        return;
      }
     //
     daikinCommandTemp[i] = intn;
     Serial.println(daikinCommandTemp[i]);   
   }
   //
   daikinCommandBuffer[acNumber][0] = daikinCommandTemp[0];
   daikinCommandBuffer[acNumber][1] = daikinCommandTemp[1];
   daikinCommandBuffer[acNumber][2] = daikinCommandTemp[2];
   daikinCommandBuffer[acNumber][3] = daikinCommandTemp[3];
   daikinCommandBuffer[acNumber][4] = daikinCommandTemp[4];
   daikinCommandBuffer[acNumber][5] = daikinCommandTemp[5];  
   sendDaikinCommand(acNumber);
}

int checkDaikinCommand(int key,int value) {
  switch (key) {
    case 0:
      break;
    case 1:
      //power 0,1
      if ((value < 0) || (value > 1)) {
        return -1;
      }    
      break;
    case 2:
      //swing 0,1
      if ((value < 0) || (value > 1)) {
        return -1;
      }     
      break;
    case 3:
      //mode 0,1,2
      if ((value < 0) || (value > 2)) {
        return -1;
      }     
      break;
    case 4:
      //speed 0,1,2,3,4,5,6
      if ((value < 0) || (value > 6)) {
        return -1;
      }     
      break;
    case 5:
      //temperature 18 ~ 33
      if ((value < 18) || (value > 33)) {
        return -1;
      }     
      break;
    default:
      return -1;    
  }
  return 1;
}

void sendDaikinCommand(int  acNumber) {
    if (daikinCommandBuffer[acNumber][1] == 1) {
      irdaikin.daikin_on();
    }else {
      irdaikin.daikin_off();
    }
    if (daikinCommandBuffer[acNumber][2] == 1) {
      irdaikin.daikin_setSwing_on();
    }else {
      irdaikin.daikin_setSwing_off();
    }
    irdaikin.daikin_setMode(daikinCommandBuffer[acNumber][3]);
    irdaikin.daikin_setFan(daikinCommandBuffer[acNumber][4]);
    irdaikin.daikin_setTemp(daikinCommandBuffer[acNumber][5]);
    irdaikin.daikin_sendCommand();
    //irdaikin.dump();
    sendHTTPJSONOK();  
}

int checkStartWithEqual() {
  int idx = commandAction.indexOf('=');
  if (idx == -1) {
    return -1;
  }
  return idx + 1;
}
int checkStartWithQuestionMark() {
  int idx = commandAction.indexOf('?');
  if (idx == -1) {
    return -1;
  }
  return idx + 1;
}

int getNumberOfAC(int start) {
   int acNumber = commandAction.substring(start,start + 2).toInt();
   if (acNumber>=(NUMBER_OF_DAIKIN)) {
       return -1;
   }else {
     return  acNumber;
   } 
}

//command action response
void getACStatus() {
   int idx = checkStartWithQuestionMark();
   if (idx == -1) {
     sendHTTPJSONCMD_ERROR();
     return;
   }   
   int acNumber = getNumberOfAC(idx);
   if (acNumber == -1) {
     sendHTTPJSONCMD_ERROR();
     return;
   }  
    HTTP_JSON_Header();
    client.print(F("{\"status\":\"ok\","));
    client.print(F("\"number\":\""));
    client.print(daikinCommandBuffer[acNumber][0]);
    client.print(F("\","));         
    client.print(F("\"power\":\""));
    client.print(daikinCommandBuffer[acNumber][1]);
    client.print(F("\","));
    client.print(F("\"swing\":\""));
    client.print(daikinCommandBuffer[acNumber][2]);
    client.print(F("\","));
    client.print(F("\"mode\":\""));
    client.print(daikinCommandBuffer[acNumber][3]);
    client.print(F("\","));
    client.print(F("\"speed\":\""));
    client.print(daikinCommandBuffer[acNumber][4]);
    client.print(F("\","));
    client.print(F("\"temperature\":\""));
    client.print(daikinCommandBuffer[acNumber][5]);
    client.print(F("\""));
    client.println(F("}"));        
    client.flush();
    client.stop(); 
}
void getTemperature() {
#ifndef SENSOR_TEMPERATURE
  sendHTTPJSONCMD_ERROR();
  return;
#endif
   int idx = checkStartWithQuestionMark();
   if (idx == -1) {
   sendHTTPJSONCMD_ERROR();
   return;
   }   
   int acNumber = getNumberOfAC(idx);
   if ((acNumber == -1) || (acNumber>=NUMBER_OF_TEMPERATURE)) {
   sendHTTPJSONCMD_ERROR();
   return;
   }  
  HTTP_JSON_Header();
  client.print(F("{\"status\":\"ok\","));
  client.print(F("\"number\":\""));
  client.print(acNumber);
  client.print(F("\","));          
  client.print(F("\"temperature\":\""));
  client.print(sensorTemperature[acNumber]);
  client.print(F("\""));          
  client.println(F("}"));
  client.flush();
  client.stop();          
}
void getHumidity() {
#ifndef SENSOR_HUMIDITY
  sendHTTPJSONCMD_ERROR();
  return;
#endif  
   int idx = checkStartWithQuestionMark();
   if (idx == -1) {
   sendHTTPJSONCMD_ERROR();
   return;
   }   
   int acNumber = getNumberOfAC(idx);
   if ((acNumber == -1) || (acNumber>=NUMBER_OF_HUMIDITY)) {
   sendHTTPJSONCMD_ERROR();
   return;
   }   
  HTTP_JSON_Header();
  client.print(F("{\"status\":\"ok\","));
  client.print(F("\"number\":\""));
  client.print(acNumber);
  client.print(F("\","));          
  client.print(F("\"humidity\":\""));
  client.print(sensorHumidity[acNumber]);
  client.print(F("\""));          
  client.println(F("}"));
  client.flush();
  client.stop();          
}

void schdule1Second() {
  //Serial.println("schdule1Second");
}
void schdule2Second() {
  //Serial.println("schdule2Second");
#ifdef SENSOR_DHT  
    readDHT(0);
#endif    
}
// HTTP Response

static void HTTP_JSON_Header() {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Pragma: no-cache"));
  client.println(F("Connection: close"));  // the connection will be closed after completion of the response
  client.println();  
}

void sendHTTPOK() {
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Pragma: no-cache"));
          client.println(F("Connection: close"));  // the connection will be closed after completion of the response
          client.println();
          client.println(F("<!DOCTYPE HTML>"));
          client.println(F("<html>"));
          client.print(F("OK"));
          client.println(F("</html>"));
          client.flush();
          client.stop();          
}
void sendHTTPJSONOK() {
          HTTP_JSON_Header();
          client.println(F("{\"status\":\"ok\"}"));
          client.flush();
          client.stop();   
}
void sendHTTPJSONERROR() {
          HTTP_JSON_Header();
          client.println(F("{\"status\":\"error\"}"));
          client.flush();
          client.stop();   
}
void sendHTTPJSONCMD_ERROR() {
          HTTP_JSON_Header();
          client.println(F("{\"status\":\"command error\"}"));
          client.flush();
          client.stop();   
}
//
void readDHT(int hvacNumber) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius
  float temperatureC = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperatureC) ) {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }
  sensorHumidity[hvacNumber] = humidity;
  // Read temperature as Celsius
  sensorTemperature[hvacNumber] = temperatureC;
  
}

