#ifndef RESOURCE_
#define RESOURCE_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: resource.h
 * Author: Johan Johanson
 * Date: November 26, 2005
 * Updated: April 15, 2005
 *
 * Description: Resource file for application
 *				Global resource definitions and application constants
 *
*/

// resource id:s
#define MobileRoboticsForm 1000
#define MobileRoboticsIcon 1000
#define MobileRoboticsDebugField 1100
#define MobileRoboticsDebugFieldScrollBar 1200

// RCX command id:s received from the client
const UInt8 CMD_ON = 0x00;
const UInt8 CMD_OFF = 0x01;
const UInt8 CMD_FORWARD = 0x02;
const UInt8 CMD_BACKWARD = 0x03;
const UInt8 CMD_LEFT = 0x04;
const UInt8 CMD_RIGHT = 0x05;
const UInt8 CMD_STOP = 0x06;

#endif