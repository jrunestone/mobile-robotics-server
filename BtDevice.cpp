/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtDevice.cpp
 * Author: Johan Johanson
 * Date: January 20, 2005
 * Updated: January 20, 2005
 *
 * Description: See BtDevice.h
 *
*/

#include "BtBase.h"
#include "BtDevice.h"

BtDevice::BtDevice() : socket(BtBase::InvalidSocket), accessibility(0), classOfDevice(0)
{
	MemSet((void *)&address, sizeof(BtLibDeviceAddressType), 0);
	MemSet((void *)&friendlyName, sizeof(BtLibFriendlyNameType), 0);
}

BtDevice::~BtDevice()
{
	socket = BtBase::InvalidSocket;
	
	accessibility = 0;
	classOfDevice = 0;
	
	if (friendlyName.name != NULL)
		MemPtrFree(friendlyName.name);
	
	MemSet((void *)&address, sizeof(BtLibDeviceAddressType), 0);
	MemSet((void *)&friendlyName, sizeof(BtLibFriendlyNameType), 0);
}

BtLibSocketRef BtDevice::getSocket()
{
	return socket;
}

BtLibDeviceAddressType *BtDevice::getAddress()
{
	return &address;
}

BtLibFriendlyNameType *BtDevice::getFriendlyName()
{
	return &friendlyName;
}

UInt32 BtDevice::getAccessibility()
{
	return accessibility;
}

UInt32 BtDevice::getClassOfDevice()
{
	return classOfDevice;
}