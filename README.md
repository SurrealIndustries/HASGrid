# HASGrid #
Home Automation System Grid

This project uses RF24, RF24Mesh, and RF24Network libraries from TMRh20 to send messages to and from nodes to a base station.
Examples messages include sending light state and temperature.

## Examples ##

#### NodeSwitch_v2 ####

Detects light state and changes state based off messages from RF_Base.

#### NodeSwitch_v3 ####

Adds code for dimming and zero cross detection.

#### RF_Base_v4 ####

Reads and sends messages from Nodes.
Pulls status from nodes and formats it into json for web use.
