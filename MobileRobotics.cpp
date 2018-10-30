/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: MobileRobotics.cpp
 * Author: Johan Johanson
 * Date: February 05, 2005
 * Updated: April 15, 2005
 *
 * Description: See MobileRobotics.cpp
 *
*/

#include "MobileRobotics.h"
#include "resource.h"

MobileRobotics::MobileRobotics() : BtRFCOMMServer(), busy(false)
{

}

MobileRobotics::~MobileRobotics()
{

}

void MobileRobotics::shutdown()
{
	rcxInterface.shutdown();
	BtRFCOMMServer::shutdown();
}

// called automatically from within class when the Bluetooth radio first is initialized
Boolean MobileRobotics::init(Char *serviceName, UInt16 maxNumClients, Boolean advanceCredit)
{
	if (!BtRFCOMMServer::init(serviceName, maxNumClients, advCredit))
		return false;

	// initialize the RCX IR interface
	if (!rcxInterface.init())
	{
		debugPrint("Unable to initialize RCX interface");
		return false;
	}

	// see if the RCX is responding - is online and in range
	UInt16 len = 7;
	UInt8 pingPacket[7];
	MemSet((void *)&pingPacket, sizeof(pingPacket), 0);

	rcxInterface.buildPacket("\x10", 1, pingPacket);

	// send query and receive echo within 1 second
	if (!rcxInterface.sendPacket(pingPacket, sizeof(pingPacket)))
	{
		debugPrint("IR error");
		return false;
	}
	
	// this method clearly doesn't work declared in an extra code segment
	/*if (!rcxInterface.recvPacket(pingPacket, &len, 20))
	{
		debugPrint("Unable to find the RCX");
		return false;
	}*/
	
	OrLibStartRcvData(rcxInterface.getLibRef(), rcxInterface.getBaudRate());
	OrLibRcvData(rcxInterface.getLibRef(), pingPacket, &len, rcxInterface.getRecvFrequency(), 
				 rcxInterface.getDataBits(), rcxInterface.getParity(), rcxInterface.isLsbFirst(), rcxInterface.isInvertData(), 10);
	OrLibEndRcvData(rcxInterface.getLibRef());

	if (len < 1)
	{
		debugPrint("RCX not found");
		return false;
	}
	
	debugPrint("RCX alive");
	WinDrawChars("ok", 2, 47 + 13, 141 + 13 / 2 - 5);

	return true;
}

void MobileRobotics::debugPrint(Char *str)
{
	// retrieve a ptr to debug field
	FieldPtr field = (FieldPtr)FrmGetObjectPtr(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
																					 MobileRoboticsDebugField));

	if (str != NULL && field != NULL)
	{
		// apply time stamp to message
		DateTimeType dateTime;
		Char date[timeStringLength + 3 + StrLen(str) + 1];

		TimSecondsToDateTime(TimGetSeconds(), &dateTime);
		TimeToAscii(dateTime.hour, dateTime.minute, tfColon24h, date);

		StrCat(date, " - ");
		StrNCat(date, str, timeStringLength + StrLen(str) + 1);

		FldSetInsPtPosition(field, FldGetTextLength(field));
		FldInsert(field, date, StrLen(date));
		FldInsert(field, "\n", 1);
	}
}

UInt16 MobileRobotics::getRCXBatteryLevel()
{
	busy = true;
	
    // send battery level request packet to the rcx
    UInt8 battPack[7];
    rcxInterface.buildPacket("\x30", 1, battPack);

    if (rcxInterface.sendPacket(battPack, sizeof(battPack)))
    {
		UInt8 replyPack[9];
		UInt16 len = 9;

		// expect a reply..
		OrLibStartRcvData(rcxInterface.getLibRef(), rcxInterface.getBaudRate());
		OrLibRcvData(rcxInterface.getLibRef(), replyPack, &len, rcxInterface.getRecvFrequency(), 
					 rcxInterface.getDataBits(), rcxInterface.getParity(), rcxInterface.isLsbFirst(), rcxInterface.isInvertData(), 10);
		OrLibEndRcvData(rcxInterface.getLibRef());
		
		if (len > 0)
		{
			// calculate millivolt percentage
			UInt16 mV = (replyPack[5] & 0xff) * 256;
			mV += (replyPack[3] & 0xff);
			
			busy = false;			
			return mV;
		}
    }
    
    busy = false;
    
    return 0;
}

void MobileRobotics::update()
{
	if (timer == 0)
		return;
		
	if (TimGetSeconds() - timer >= 3)
	{
		UInt8 stopPacket[9];
		rcxInterface.buildPacket("\x21\x44", 2, stopPacket);
		
		if (!rcxInterface.sendPacket(stopPacket, sizeof(stopPacket)))
			debugPrint("Unable to send command");
			
		timer = 0;
	}
}

Boolean MobileRobotics::onConnectedInbound(BtLibSocketEventType *event)
{
	if (!BtRFCOMMServer::onConnectedInbound(event))
		return false;

	// since we're only expecting one client, use link #0
	// begin with 2 credits in order to be able to send/receive data
	advanceCredit(2, 0);
	//authenticate(0);
	//setEncryption(true, 0);

	Char msg[80];
	Char name[connections[0].remote.getFriendlyName()->nameLength];
	StrNCopy(name, (Char *)connections[0].remote.getFriendlyName()->name, connections[0].remote.getFriendlyName()->nameLength);
	StrPrintF(msg, "Client connected: %s", name);

	debugPrint(msg);
	WinDrawChars("ok", 2, 47 + 13, 126 + 13 / 2 - 5);	// BT status

	return true;
}

Boolean MobileRobotics::onData(BtLibSocketEventType *event)
{
	if (event->eventData.data.data == NULL || busy)
		return false;

	// we send simple integer data
	// NOTE: you should use BtDataPacket and cast the
	// data pointer to that
	UInt8 cmd = *event->eventData.data.data;

	// REMEMBER TO ADVANCE CREDITS FOR THE ACTIVE LINK
	// OR ELSE YOU WON'T BE ABLE TO SEND OR RECEIVE ANY MORE DATA
	// (saves you 3 hours of work debugging something completely irrelevant)
	// (yes it happened to me)
	advanceCredit(1, 0);

	/*
		interpret the command and send it to the RCX

		0) power on (the car, not the rcx)
		1) power off (the car, not the rcx)
		2) forward
		3) backward
		4) left
		5) right
		6) stop (only forward/backward engine)
	*/

	switch (cmd)
	{
		case CMD_ON:
		{
			debugPrint("command: power on");
			
			UInt8 onPacket[9];
			UInt8 dirPacket[9];
			UInt8 beepPacket[9];
			
			rcxInterface.buildPacket("\x21\x81", 2, onPacket);
			rcxInterface.buildPacket("\xe1\x01", 2, dirPacket);
			rcxInterface.buildPacket("\x51\x03", 2, beepPacket);

			if (!rcxInterface.sendPacket(beepPacket, sizeof(beepPacket)))
				debugPrint("Unable to send command");
				
			SysTaskDelay(10);

			if (!rcxInterface.sendPacket(dirPacket, sizeof(dirPacket)))
				debugPrint("Unable to send command");
				
			SysTaskDelay(10);
			
			if (!rcxInterface.sendPacket(onPacket, sizeof(onPacket)))
				debugPrint("Unable to send command");			
				
			// must wait a couple of seconds and then power it off again
			UInt32 start = TimGetSeconds();
			while (TimGetSeconds() - start < 7);
			
			UInt8 stopPacket[9];
			rcxInterface.buildPacket("\x21\x41", 2, stopPacket);
			
			if (!rcxInterface.sendPacket(stopPacket, sizeof(stopPacket)))
				debugPrint("Unable to send command");
			
			break;
		}

		case CMD_OFF:
		{
			debugPrint("command: power off");

			UInt8 onPacket[9];
			UInt8 dirPacket[9];
			UInt8 beepPacket[9];
			
			rcxInterface.buildPacket("\x21\x81", 2, onPacket);
			rcxInterface.buildPacket("\xe1\x81", 2, dirPacket);
			rcxInterface.buildPacket("\x51\x02", 2, beepPacket);

			if (!rcxInterface.sendPacket(beepPacket, sizeof(beepPacket)))
				debugPrint("Unable to send command");

			SysTaskDelay(10);

			if (!rcxInterface.sendPacket(dirPacket, sizeof(dirPacket)))
				debugPrint("Unable to send command");
			
			SysTaskDelay(10);
			
			if (!rcxInterface.sendPacket(onPacket, sizeof(onPacket)))
				debugPrint("Unable to send command");			
				
			// must wait a couple of seconds and then power it off again
			UInt32 start = TimGetSeconds();
			while (TimGetSeconds() - start < 7);
			
			UInt8 stopPacket[9];
			rcxInterface.buildPacket("\x21\x41", 2, stopPacket);
			
			if (!rcxInterface.sendPacket(stopPacket, sizeof(stopPacket)))
				debugPrint("Unable to send command");

			break;
		}

		case CMD_FORWARD:
		{
			debugPrint("command: forward");
			
			UInt8 onPacket[9];
			UInt8 dirPacket[9];
			
			rcxInterface.buildPacket("\x21\x82", 2, onPacket);
			rcxInterface.buildPacket("\xe1\x02", 2, dirPacket);

			if (!rcxInterface.sendPacket(dirPacket, sizeof(dirPacket)))
				debugPrint("Unable to send command");
			
			SysTaskDelay(10);
			
			if (!rcxInterface.sendPacket(onPacket, sizeof(onPacket)))
				debugPrint("Unable to send command");
				
			break;
		}

		case CMD_BACKWARD:
		{
			debugPrint("command: backward");
			
			UInt8 onPacket[9];
			UInt8 dirPacket[9];
			
			rcxInterface.buildPacket("\x21\x82", 2, onPacket);
			rcxInterface.buildPacket("\xe1\x82", 2, dirPacket);

			if (!rcxInterface.sendPacket(dirPacket, sizeof(dirPacket)))
				debugPrint("Unable to send command");
			
			SysTaskDelay(10);
			
			if (!rcxInterface.sendPacket(onPacket, sizeof(onPacket)))
				debugPrint("Unable to send command");
			
			break;
		}

		case CMD_LEFT:
		{
			debugPrint("command: left");

			UInt8 onPacket[9];
			UInt8 dirPacket[9];
			
			rcxInterface.buildPacket("\x21\x84", 2, onPacket);
			rcxInterface.buildPacket("\xe1\x84", 2, dirPacket);

			if (!rcxInterface.sendPacket(dirPacket, sizeof(dirPacket)))
				debugPrint("Unable to send command");
			
			SysTaskDelay(10);
			
			if (!rcxInterface.sendPacket(onPacket, sizeof(onPacket)))
				debugPrint("Unable to send command");
				
			// must wait a couple of seconds and then power steering off
			timer = TimGetSeconds();

			break;
		}

		case CMD_RIGHT:
		{
			debugPrint("command: right");

			UInt8 onPacket[9];
			UInt8 dirPacket[9];
			
			rcxInterface.buildPacket("\x21\x84", 2, onPacket);
			rcxInterface.buildPacket("\xe1\x04", 2, dirPacket);

			if (!rcxInterface.sendPacket(dirPacket, sizeof(dirPacket)))
				debugPrint("Unable to send command");
			
			SysTaskDelay(10);
			
			if (!rcxInterface.sendPacket(onPacket, sizeof(onPacket)))
				debugPrint("Unable to send command");
				
			// must wait a couple of seconds and then power it off again
			timer = TimGetSeconds();

			break;
		}

		case CMD_STOP:
		{
			debugPrint("command: stop");
			
			UInt8 stopPacket[9];
			
			rcxInterface.buildPacket("\x21\x42", 2, stopPacket);
			
			if (!rcxInterface.sendPacket(stopPacket, sizeof(stopPacket)))
				debugPrint("Unable to send command");

			break;
		}
		
		default:
		{
			debugPrint("command: unknown");
			break;
		}
	}

	return true;
}

Boolean MobileRobotics::onDisconnected(BtLibSocketEventType *event)
{
	// since we're only accepting 1 client, disconnect #0
	disconnect(0);
	debugPrint("Client disconnected");

	UInt8 stopPacket[9];
	UInt8 beepPacket[9];
	
	rcxInterface.buildPacket("\x21\x42", 2, stopPacket);
	rcxInterface.buildPacket("\x51\x02", 2, beepPacket);

	if (!rcxInterface.sendPacket(beepPacket, sizeof(beepPacket)))
		debugPrint("Unable to send command");

	SysTaskDelay(10);

	// stop forward/backward engine
	if (!rcxInterface.sendPacket(stopPacket, sizeof(stopPacket)))
		debugPrint("Unable to send command");

	WinEraseChars("ok", 2, 47 + 13, 126 + 13 / 2 - 5);
	WinDrawChars("--", 2, 47 + 13, 126 + 13 / 2 - 5);

	// go back to listening
	if (!listen())
	{
		debugPrint("Unable to return to listening state");
		return false;
	}

	debugPrint("Server in listening state");

	return true;
}