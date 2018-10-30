#ifndef MOBILEROBOTICS_
#define MOBILEROBOTICS_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: MobileRobotics.h
 * Author: Johan Johanson
 * Date: February 02, 2005
 * Updated: April 15, 2005
 *
 * Description: Builds upon BtRFCOMMServer
 * 				Main application class, dependent implementation
 * 				Handles Bluetooth connections and IR management
 *
*/

#include <PalmOS.h>

#include "BtRFCOMMServer.h"
#include "RCXInterface.h"
#include "resource.h"
#include "sections.h"

class MobileRobotics : public BtRFCOMMServer
{
public:
	MobileRobotics() EXT_SEG;
	~MobileRobotics() EXT_SEG;

	Boolean init(Char *serviceName, UInt16 maxNumClients, Boolean advanceCredit);
	void shutdown();

	void debugPrint(Char *str) EXT_SEG;

	UInt16 getRCXBatteryLevel();

public:
	void update();

private:
	Boolean onConnectedInbound(BtLibSocketEventType *event) EXT_SEG;
	Boolean onData(BtLibSocketEventType *event) EXT_SEG;
	Boolean onDisconnected(BtLibSocketEventType *event) EXT_SEG;
	
private:
	RCXInterface rcxInterface;
	
	UInt32 timer;
	Boolean busy;
};

#endif