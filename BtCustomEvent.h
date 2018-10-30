#ifndef BTCUSTOMEVENT_
#define BTCUSTOMEVENT_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtCustomEvent.h
 * Author: Johan Johanson
 * Date: February 26, 2005
 * Updated: February 26, 2005
 *
 * Description: A custom event struct used to notify application of a certain
 * 				event that requires application flow to halt
 *
*/

#include <PalmOS.h>

const UInt16 btCustomRadioStateEvent = 0;

struct BtCustomEvent
{
	// required event attributes
	eventsEnum eType;
	Boolean penDown;
	UInt8 tapCount;
	Int16 screenX;
	Int16 screenY;
	
	// event-specific data
	union
	{
		struct _GenericEventType generic;
		
		struct
		{
			UInt16 type;	// our event type
			UInt16 status;	// our event status code
		} eventData;
	
	} data;
};

#endif