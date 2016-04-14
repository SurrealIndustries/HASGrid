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
#define RF_NODE_ID	4
#define RF_CHANNEL	80

#define HW_SWITCH_PIN   3
#define AC_pin          6
#define LIGHT_PIN       5
#define TEMP_SENSE      A5

HasgridRF24 rf;

int freqStep = 65;
volatile int i = 0;             // Variable to use as a counter
int dim = 0;                    // Dimming level (0-128)  0 = on, 128 = 0ff
volatile boolean zero_cross = 0; // Boolean to store a "switch" to tell us if we have crossed zero
volatile boolean lightState = false;
volatile int tempState = 0;
boolean previousLightState = false;
volatile boolean hwSwitchState = false;
boolean rfSwitchState = false;

////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.println(F("Serial Started!"));
  // start node first so we can get the light switch going
  pinMode(HW_SWITCH_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, lightState);
  attachInterrupt(0, zero_cross_detect, RISING);   // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  // attach function in case rf24 issue
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(hwSwitchUpdate, freqStep);
  Serial.println(F("Setting up RF24 Radio"));
  Serial.print(F("CHANNEL = "));
  Serial.print(String(RF_CHANNEL));
  Serial.print(F(" NODE ID = "));
  Serial.println(String(RF_NODE_ID));
  Serial.println("STARTING...");
  if (rf.start(RF_NODE_ID, RF_CHANNEL)) {
    Serial.println(F("RF24 Started!"));
    Serial.print(F("Address: "));
    Serial.println(rf.getNodeAddress());
  }
  else {
    Serial.println(F("RF24 ERROR!"));
  }
  Serial.println(F("Node Started!"));
}

void loop() {
  // handle serial input
  menu(serialRead());
  if (previousLightState != lightState) {
    previousLightState = lightState;
    Serial.println("Light: " + (String)(lightState ? "ON" : "OFF" ));
    Serial.println("Sent: " + (String)(sendState() ? "Success" : "Error" ));
  }
  updateRF24();
}
////////////////////////////////////////////////////////////////////////
//  Interupts No IO
void hwSwitchUpdate()
{
  tempState = analogRead(TEMP_SENSE);
  hwSwitchState = digitalRead(HW_SWITCH_PIN);
  lightState = (hwSwitchState != rfSwitchState);
  digitalWrite(LIGHT_PIN, lightState);
  if (zero_cross == true) {
    if (i >= dim) {
      digitalWrite(AC_pin, HIGH); // turn on light
      i = 0; // reset time step counter
      zero_cross = false; //reset zero cross detection
    }
    else {
      i++; // increment time step counter
    }
  }
}

void zero_cross_detect() {
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i = 0;
  digitalWrite(AC_pin, LOW);       // turn off TRIAC (and AC)
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
      rf.send(input.substring(3));
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
    if (msg == "ON") {
      rfSwitchState = !hwSwitchState;// light ex. T != F ==> T
    }
    else if (msg == "OFF") {
      rfSwitchState = hwSwitchState; // light ex. T != T ==> F
    }
    else if (msg == "SWITCH") {
      rfSwitchState = !rfSwitchState;
    }
    else if (msg == "STATUS") {
      sendState();
    }
    else {
      sendError();
    }
  }
}

bool sendState() {
  if (lightState) {
    return sendRF("Status - ON");
  }
  else {
    return sendRF("Status - OFF");
  }
}

//////////////////////////////////////////////////////////////////////

bool sendRF(String s) {
  rf.send("NODE: " + (String)RF_NODE_ID + " - " + s);
}

void sendError() {
  sendRF("COULD NOT RUN COMMAND");
}
