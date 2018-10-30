#ifndef BTDEVICE_
#define BTDEVICE_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtDevice.h
 * Author: Johan Johanson
 * Date: January 20, 2005
 * Updated: January 20, 2005
 *
 * Description: Encapsulates information about a Bluetooth device (local or remote)
 *
*/

#include <PalmOS.h>
#include <BtLib.h>

class BtDevice
{
	friend class BtBase;
	friend class BtServerBase;
	friend class BtRFCOMMServer;
	
public:
	BtDevice();
	~BtDevice();
	
public:
	BtLibSocketRef getSocket();
	
	BtLibDeviceAddressType *getAddress();
	BtLibFriendlyNameType *getFriendlyName();
	
	UInt32 getAccessibility();
	UInt32 getClassOfDevice();
		
private:
	BtLibSocketRef socket;
	
	BtLibDeviceAddressType address;
	BtLibFriendlyNameType friendlyName;
	
	UInt32 accessibility;
	UInt32 classOfDevice;
};

#endif