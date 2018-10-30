#ifndef BTRFCOMMSERVER_
#define BTRFCOMMSERVER_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtRFCOMMServer.h
 * Author: Johan Johanson
 * Date: January 26, 2005
 * Updated: February 10, 2005
 *
 * Description: Builds upon BtServerBase
 * 				Defines an RFCOMM based abstract class
 *
*/

#include "BtServerBase.h"
#include "sections.h"

class BtRFCOMMServer : public BtServerBase
{
public:
	BtRFCOMMServer() EXT_SEG;
	virtual ~BtRFCOMMServer() EXT_SEG;

	virtual Boolean init(Char *serviceName, UInt16 maxNumClients, Boolean advanceCredit) EXT_SEG;
	virtual Boolean listen() EXT_SEG;
	
	// packet must be persistent, non-local
	virtual Boolean sendPacket(BtDataPacket *packet, UInt16 receiver) EXT_SEG;
	
	void advanceCredit(UInt8 credit, UInt16 link) EXT_SEG;

protected:
	virtual Boolean onConnectedInbound(BtLibSocketEventType *event) EXT_SEG;
	virtual Boolean onConnectRequest(BtLibSocketEventType *event) EXT_SEG;
	virtual Boolean onDisconnected(BtLibSocketEventType *event) EXT_SEG;
	
protected:
	Boolean advCredit;
};

#endif