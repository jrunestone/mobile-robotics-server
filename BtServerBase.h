#ifndef BTSERVERBASE_
#define BTSERVERBASE_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtServerBase.h
 * Author: Johan Johanson
 * Date: January 20, 2005
 * Updated: February 10, 2005
 *
 * Description: Builds upon BtBase
 * 				Defines an abstract protocol independent Bluetooth socket server
 *
*/

#include "BtBase.h"
#include "BtDataPacket.h"
#include "BtSocketPair.h"

class BtServerBase : public BtBase
{
public:
	BtServerBase();
	virtual ~BtServerBase();
	
	virtual Boolean init(Char *serviceName, UInt16 maxNumClients);	
	virtual Boolean listen() = 0;
	
	virtual void disconnect();
	virtual Boolean disconnect(UInt16 client);

	// packet must be persistent, non-local
	virtual Boolean sendPacket(BtDataPacket *packet, UInt16 receiver);

	void authenticate(UInt16 link);

	void setAccessibility(UInt32 mode);
	void setEncryption(Boolean encryption, UInt16 link);
	
	UInt16 getMaxNumClients();
	UInt16 getNumConnectedClients();
	UInt16 getActiveListener();
	Boolean getAuthenticated(UInt16 link);
	Boolean getEncryption(UInt16 link);
	BtDevice *getClientDevice(UInt16 client);
	
protected:
	Boolean onAuthenticationComplete(BtLibManagementEventType *event);
	
public:
	enum
	{
		MaxNumClients = 7,
		MaxLenService = 16
	};		
	
protected:
	BtSocketPair *connections;
	
	BtLibSdpRecordHandle sdpHandle;
	BtLibSdpUuidType sdpUuid;
	
	Char serviceName[MaxLenService];
	
	UInt16 maxNumClients;
	UInt16 numConnectedClients;
	UInt16 activeListener;
};

#endif