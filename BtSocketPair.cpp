/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtSocketPair.cpp
 * Author: Johan Johanson
 * Date: January 29, 2005
 * Updated: January 29, 2005
 *
 * Description: See BtSocketPair.h
 *
*/

#include "BtSocketPair.h"

BtSocketPair::BtSocketPair() : local(0)
{
	MemSet(&remote, sizeof(BtDevice), 0);
}