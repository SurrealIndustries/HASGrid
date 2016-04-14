/*
**  Start code for RF24
**  Authors: Brandon Marshall
**  Date: 3/24/15
**	Should only have to change CE and CS pins
*/

#ifndef __HasgridRF24_H__
#define __HasgridRF24_H__

class HasgridRF24
{
public:
	int nodeTop;
	HasgridRF24();
	~HasgridRF24();
	bool start(uint8_t nodeID, uint8_t channel);
	void update();
	bool send(int node, String msg);
	bool send(String msg);
	String receive();
	String getNodeAddress();
	uint8_t getNodeID(int i);
	String getAllNodes();
private:
	bool checkAddress();
	bool send(int node, void *msg, char type, size_t msgSize);
	int msgSize;
	bool isBase;
	long previousMillis;
	long interval;
};

#endif /* __HasgridRF24_H__ */
