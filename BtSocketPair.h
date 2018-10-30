#ifndef BTSOCKETPAIR_
#define BTSOCKETPAIR_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtSocketPair.h
 * Author: Johan Johanson
 * Date: January 29, 2005
 * Updated: January 29, 2005
 *
 * Description: Encapsulates two communicating sockets
 *
*/

#include "BtDevice.h"

class BtSocketPair
{
public:
	BtSocketPair();
	
public:
	BtLibSocketRef local;
	BtDevice remote;
};

#endif