/*
**  Start code for RF24
**  Authors: Brandon Marshall
**  Date: 3/24/15
*/

// Include required libraries
#include <avr/io.h>
//#include <arduino.h>

#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"

#include "HasgridRF24.h"

// Define RF24Radio chip pins
#if defined(__AVR_ATmega2560__)
#define RF24RADIO_CE	49
#define RF24RADIO_CS	53
#elif (__AVR_ATmega328P__)
#define RF24RADIO_CE	9
#define RF24RADIO_CS	10
#elif (__AVR_Mighty_OPTI__)
#define RF24RADIO_CE	13
#define RF24RADIO_CS	0
#else
#define RF24RADIO_CE	13
#define RF24RADIO_CS	0
#endif

// Create RF24Radio and mesh
RF24 radio(RF24RADIO_CE, RF24RADIO_CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

////////////////////////////////////////////////////////////////////////
//    Wait for other ICSP devices to be enabled before communicating to RF24!

HasgridRF24::HasgridRF24() {
	// Used for counting nodes
	nodeTop = 0;
	// Used to receive String messages
	msgSize = 0;
	isBase = false;
	// timer
	previousMillis = 0;
	interval = 60000;
}

HasgridRF24::~HasgridRF24() {/*nothing to destruct*/}

// TODO: return started status
bool HasgridRF24::start(uint8_t nodeID, uint8_t channel) {
	Serial.print(F("RF24RADIO_CE PIN = "));
	Serial.println(RF24RADIO_CE);
	Serial.print(F("RF24RADIO_CS PIN = "));
	Serial.println(RF24RADIO_CS);
	Serial.print(F("CHANNEL = "));
	Serial.print(String(channel));
	Serial.print(F(" NODE ID = "));
	Serial.println(String(nodeID));
	Serial.println(F("Starting RF..."));
	isBase = !(nodeID);
	mesh.setNodeID(nodeID);
	mesh.begin(channel);
	if (isBase){
		Serial.println(getAllNodes());
		return true;
	}
	else{
		return checkAddress();
	}
}

////////////////////////////////////////////////////////////////////////
//TODO: handle new nodes being added to the network
// convert to bool
void HasgridRF24::update() {
	// Call mesh.update to keep the network updated
	// TODO: check for connection and mesh.renewAddress();
	mesh.update();
	if (isBase){
		// Keep the 'DHCP service' running on the master node
		mesh.DHCP();
		//Note when node is added to mesh
		if (nodeTop != mesh.addrListTop) {
			Serial.println(getAllNodes());
			nodeTop = mesh.addrListTop;
		}
	}
	else{
		if(millis() - previousMillis > interval){
			Serial.print(F("Checking Connection: "));
			if(mesh.checkConnection()==0){
				Serial.println(F(" Not Connected."));
				Serial.print(F("Releasing Address: "));
				Serial.println((String)mesh.releaseAddress());
				Serial.println(F("Renewing Address..."));
				mesh.renewAddress();
				Serial.println("Connected:" + (String)checkAddress());
			}
			else{
				Serial.println(F(" Connected!"));
			}
			previousMillis = millis();
		}
	}
}

////////////////////////////////////////////////////////////////////////

bool HasgridRF24::checkAddress(){
	Serial.print(F("Address: "));
	Serial.println(getNodeAddress());
	if(mesh.mesh_address == MESH_DEFAULT_ADDRESS){
		if(mesh.checkConnection()){
			return true;
		}
		else{
			return false;
		}
	}
	return true;
}

String HasgridRF24::getAllNodes() {
	String status = "";
	// Display the currently assigned addresses and nodeIDs
	status += (F("========ADDRESSES========\n\r"));
	for (int i = 0; i < mesh.addrListTop; i++) {
		status += (F("NodeID: "));
		status += (String(mesh.addrList[i].nodeID));
		status += (F(" RF24Network Address: 0"));
		status += (String(mesh.addrList[i].address, OCT));
		status += (F("\n\r"));
	}
	status += (F("========================="));
	return status;
}

String HasgridRF24::getNodeAddress(){
	return String(mesh.mesh_address);
}

uint8_t HasgridRF24::getNodeID(int i){
	return mesh.addrList[i].nodeID;
}

String HasgridRF24::receive() {
	//RECEVING
	if (network.available()) {
		RF24NetworkHeader header;
		network.peek(header);
		if (header.type == 'H'){
			network.read(header, &msgSize, sizeof(msgSize));
			return "";
		}
		else if (header.type == 'M'){
			char buffer[msgSize];
			network.read(header, &buffer, sizeof(buffer));
			msgSize = 0;
			return String(buffer);
		}
		else{
			network.read(header, 0, 0);
			//mesh.getNodeId(header.from_node);
			String s = "Unknown Header Type: " + header.type;
			s += " ID " + header.id;
			s += " from " + header.from_node;
			return s;
		}
	}
	return "";
}

// Send to Node
bool HasgridRF24::send(int node, void *msg, char type, size_t msgSize) {
	uint16_t address;
	// not home base
	if (node > 0) {
		// Find address
		address = mesh.getAddress(node);
		if (address == 0) {
			// could not find node
			return false;
		}
	}
	else {
		// can not send message to self
		if(isBase){
			return false;
		}
		address = 0;
	}
	// Declare Header
	RF24NetworkHeader header(address, type);
	// Send Message
	if (network.write(header, msg, msgSize)) {
		return true;
	}
	else {
		if(!isBase){
			//Refresh the network address
			mesh.checkConnection();
			mesh.releaseAddress();
			mesh.renewAddress();
		}
		mesh.update();
		//Retry sending message
		if (network.write(header, msg, msgSize)) {
			return true;
		}
	}
	return false;
}

bool HasgridRF24::send(int node, String msg){
	int rfMsgSize = msg.length() + 1;
	char buffer[ rfMsgSize ];
	msg.toCharArray( buffer, sizeof( buffer ) );

	if (send(node, &rfMsgSize, 'H', sizeof(rfMsgSize))) {
		if (send(node, &buffer, 'M', sizeof(buffer))) {
			return true;
		}
	}
	return false;
}

bool HasgridRF24::send(String msg){
	return send(0, msg);
}
