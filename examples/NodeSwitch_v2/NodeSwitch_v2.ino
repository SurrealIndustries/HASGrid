/*
**  Node Switch Test for RF24
**  Authors: Brandon Marshall, Tyler Hill
**  Date: 3/24/15
*/
#include <SPI.h>
#include <EEPROM.h>

#define NANO_NODE

#include "TimerOne.h"
#include "HasgridRF24.h"

// RF24 Mesh Configuration
#define RF_NODE_ID	2
#define RF_CHANNEL	85

#define HW_SWITCH_PIN   3
#define LIGHT_PIN       5
#define TEMP_SENSE      A5

HasgridRF24 rf;

volatile boolean lightState = false;
volatile int tempState = 0;
boolean previousLightState = false;
volatile boolean hwSwitchState = false;
boolean rfSwitchState = false;
int temp = 77;
int current = 900;

////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600); 
  Serial.println(F("Serial Started!"));
  // start node first so we can get the light switch going
  pinMode(HW_SWITCH_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, lightState);
  // attach function in case rf24 issue
  Timer1.attachInterrupt(hwSwitchUpdate, 5000000);  // attaches callback() as a timer overflow interrupt
  Serial.println(F("Setting up RF24 Radio"));
  if (rf.start(RF_NODE_ID, RF_CHANNEL)) {
    Serial.println(F("RF24 Started!"));
  }
  else {
    Serial.print(F("RF24 ERROR: "));
    Serial.println(F("Node not connected to base."));
  }
  Serial.println(F("Node Started!"));
}

void loop() {
  // handle serial input
  menu(serialRead());
  if(previousLightState != lightState){
    previousLightState = lightState;
    Serial.println("Light = " + (String)(lightState ? "ON" : "OFF" ));
    Serial.println("Sent = " + (String)(sendState() ? "Success" : "Error" ));
  }
  updateRF24();
}

// No IO
void hwSwitchUpdate()
{
  tempState = analogRead(TEMP_SENSE);
  hwSwitchState = digitalRead(HW_SWITCH_PIN);
  lightState = (hwSwitchState != rfSwitchState);
  digitalWrite(LIGHT_PIN, lightState);
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
  return input;
}

//TODO: Add menu to control network
void menu(String input) {
  if (input != "") {
    input.toUpperCase();
    if (input.startsWith("RF")) {
      sendStrRF(input.substring(3, 4).toInt(), input.substring(5));
    }
    else {
      Serial.println(F("Unknown Command"));
    }
  }
}

void updateRF24() {
  rf.update();
  String msg = rf.receive();
  if (msg != "") {
    if (msg.startsWith("ON")) {
      rfSwitchState = !hwSwitchState;// light ex. T != F ==> T
    }
    else if (msg.startsWith("OFF")) {
      rfSwitchState = hwSwitchState; // light ex. T != T ==> F
    }
    else if (msg.startsWith("SWITCH")) {
      rfSwitchState = !rfSwitchState;
    }
    else if (msg.startsWith("STATUS")) {
      sendState();
    }
    else {
      sendError(msg);
    }
  }
}

bool sendState() {
  String state = "{";
  if (lightState) {
    state += ("\"LIGHT\":\"ON\"");
  }
  else {
    state += ("\"LIGHT\":\"OFF\"");
  }
  state += ",\"TEMP\":";
  state += temp;
  state += ",\"CURRENT\":";
  state += current;
  state += "}";
  return sendRF(state);
}

//////////////////////////////////////////////////////////////////////
bool sendStrRF(int nodeID, String msg) {
  String json = jsonString(msg);
  Serial.println("Sent: " + json + "to Node: " + nodeID);
  return rf.send(json);
}

bool sendRF(String msg){
  String json = jsonString(msg);
  Serial.println("Sent: " + json);
  return rf.send(json);
}

void sendError(String s){
  String json = jsonString("\"COULD NOT RUN COMMAND" + s + "\"");
  Serial.println("Sent: " + json);
  sendRF(json);
}

String jsonString(String msg){
  return ("{\"NODE\":" + (String)RF_NODE_ID + ",\"MSG\":" + msg + "}");
}
