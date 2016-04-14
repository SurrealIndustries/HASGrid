/*
**  Node Switch Test for RF24
**  Authors: Brandon Marshall, Tyler Hill
**  Date: 3/24/15
*/
#include <SPI.h>
#include <EEPROM.h>
#include "HasgridRF24.h"

// RF24 Mesh Configuration
#define RF_NODE_ID	0
#define RF_CHANNEL	85

////////////////////////////////////////////////////////////////////////

HasgridRF24 rf;
String rfMsg = "";

long previousMillis = 0;
long interval = 10000;
long timeout = 1000;

////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  Serial.println(F("Serial Started!"));
  Serial.println(F("Setting up RF24 Radio"));
  if(startRF24(RF_NODE_ID, RF_CHANNEL)){
    Serial.println(F("RF24 Started!"));
  }
  Serial.println(F("Base Started!"));
}

void loop() {
  menu(serialRead());
  updateRF24();
  // status
  if(millis() - previousMillis > interval) {
    ////TODO: connect to server
    String json = ("{\"Base\":0,\"Nodes\":");
    json += rf.nodeTop;
    json += (",\"Status:\"[");
    for (int i=1; i<=rf.nodeTop; i++){
      previousMillis = millis();
      if(sendStrRF(rf.getNodeID(i-1), "STATUS")){
        while(millis() - previousMillis < interval){
          rf.update();
          rfMsg = rf.receive();
          if(rfMsg != ""){
            json += (rfMsg);
            if(i!=rf.nodeTop){
              json += ",";
            }
            break;
          }
        }
      }
    }
    json += ("]}");
    Serial.println(json);
    previousMillis = millis();
  }
}

////////////////////////////////////////////////////////////////////////

String serialRead() {
  //Serial Commands
  String input = "";
  //get input
  while (Serial.available() > 0) {
    input += (char)Serial.read();
    delay(5);
  }
  input.toUpperCase();
  return input;
}

//Add menu to control network all input in uppercase
void menu(String input) {
  if (input != "") {
    Serial.print(F("INPUT: "));
    Serial.println(input);
    if (input[0] == '0') {
      printAllNodes();
    }
    else if (input.startsWith(F("RF"))) {
      sendStrRF(input.substring(3, 4).toInt(), input.substring(5));
    }
    else {
      Serial.println(F("Unknown Command"));
    }
  }
}

////////////////////////////////////////////////////////////////////////

bool startRF24(uint8_t rfID, uint8_t channel) {
  return rf.start(rfID, channel);
}

////////////////////////////////////////////////////////////////////////

void updateRF24() {
  rf.update();
  rfMsg = rf.receive();
  if (rfMsg != "") {
    Serial.print(F("RF Received: "));
    Serial.println(rfMsg);
  }
}

////////////////////////////////////////////////////////////////////////

bool sendStrRF(int nodeID, String msg) {
  Serial.print(F("RF Sending: "));
  Serial.print(msg);
  Serial.print(F("\tTo Node: "));
  Serial.println(nodeID);
  if (!rf.send(nodeID, msg)) {
    Serial.print(F("Error sending message to node: "));
    Serial.println( nodeID );
    return false;
  }
  else{
    return true;
  }
}

////////////////////////////////////////////////////////////////////////

void printAllNodes() {
  Serial.println(rf.getAllNodes());
}

void printNodeAddress() {
  Serial.print(F("Address: "));
  Serial.println(rf.getNodeAddress());
}
