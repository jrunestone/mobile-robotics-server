#ifndef BTDATAPACKET_
#define BTDATAPACKET_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtDataPacket.h
 * Author: Johan Johanson
 * Date: January 26, 2005
 * Updated: March 01, 2005
 *
 * Description: Defines an abstract data packet used to encapsulate
 * 				the data one wishes to send
 *
*/

#include <PalmOS.h>

class BtDataPacket
{
public:
	BtDataPacket();
		
public:
	UInt8 *data;
	UInt16 length;
};

#endif