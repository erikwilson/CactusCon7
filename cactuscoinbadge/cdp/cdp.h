#include "Arduino.h"

#ifndef CSR_h
#define CSR_h

class CSR
{
    public:
	int getCSRID();
	int getBroadcastedID(int id);
	byte * getCSRSignature();
        void setCSRID(int id);
	void setBroadcastedID(int id);
	byte * toByteArray();
}

#endif

#ifndef CAD_h
#define CAD_h

class CAD
{
    public:
	int getCSRID();
	int getBroadcastedID(int id);
	byte * getCSRSignature();
	byte * getBroadcasterSignature();
        void setCSRID(int id);
	void setBroadcastedID(int id);
	byte * toByteArray();
}

#endif

#ifndef CDP_h
#define CDP_h

class CDP
{
    public:
        int getType();
	int getBadgeID();
	void decodePacket(byte *packet);
	void setType(int type);
	void setBadgeID(int badgeID);
        CSR getMessage();
	const int RUP = 1;
	const int GBP = 2;
	const int CSR = 3;
	const int CAD = 4;
}

#endif
