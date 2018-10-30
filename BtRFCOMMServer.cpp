/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtRFCOMMServer.cpp
 * Author: Johan Johanson
 * Date: January 26, 2005
 * Updated: February 26, 2005
 *
 * Description: See BtRFCOMMServer.h
 *
*/

#include "BtRFCOMMServer.h"

BtRFCOMMServer::BtRFCOMMServer() : BtServerBase()
{

}

BtRFCOMMServer::~BtRFCOMMServer()
{

}

Boolean BtRFCOMMServer::init(Char *serviceName, UInt16 maxNumClients, Boolean advanceCredit)
{
	if (!BtServerBase::init(serviceName, maxNumClients))
		return false;

	advCredit = advanceCredit;

	// initialize service UUID
	// random generated
	// {716441D9-D196-41A9-80D8-DCC753ADEC59}
    BtLibSdpUuidInitialize(sdpUuid, "\x71\x64\x41\xD9\xD1\x96\x41\xA9\x80\xD8\xDC\xC7\x53\xAD\xEC\x59", btLibUuidSize128);

	return true;
}

Boolean BtRFCOMMServer::listen()
{
	// opens the next invalid socket in list for listening

	Err error = 0;
	UInt16 num = 0;

	for (num = 0; num < maxNumClients; )
		if (connections[num].local == InvalidSocket && connections[num].remote.socket == InvalidSocket)
			break;
		else
			num++;

	if (connections[num].local != InvalidSocket)
	{
		debugPrint("No free listen sockets");
		return false;
	}

	// create socket
	if ((error = BtLibSocketCreate(btRef, &connections[num].local, socketCallback, 0, btLibRfCommProtocol))
		!= btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		return false;
	}

	BtLibSocketListenInfoType info;
	MemSet(&info, sizeof(BtLibSocketListenInfoType), 0);

	info.data.RfComm.maxFrameSize = BT_RF_DEFAULT_FRAMESIZE;
	info.data.RfComm.advancedCredit = 0;

	// setup socket for listening
	if ((error = BtLibSocketListen(btRef, connections[num].local, &info)) != btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		return false;
	}

	BtLibSdpServiceRecordStopAdvertising(btRef, sdpHandle);
	BtLibSdpServiceRecordDestroy(btRef, sdpHandle);
	sdpHandle = 0;

	// create an SDP service record handle
	if ((error = BtLibSdpServiceRecordCreate(btRef, &sdpHandle)) != btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		disconnect(num);
		return false;
	}

	// set SDP attributes
	if ((error = BtLibSdpServiceRecordSetAttributesForSocket(btRef, connections[num].local, &sdpUuid, 1, serviceName, 
															 StrLen(serviceName), sdpHandle))
		!= btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		disconnect(num);
		return false;
	}

	// start advertising service
	if ((error = BtLibSdpServiceRecordStartAdvertising(btRef, sdpHandle)) != btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		disconnect(num);
		return false;
	}

	activeListener = num;

	return true;
}

Boolean BtRFCOMMServer::sendPacket(BtDataPacket *packet, UInt16 receiver)
{
	if (advCredit)
		advanceCredit(1, receiver);
		
	return BtServerBase::sendPacket(packet, receiver);
}

void BtRFCOMMServer::advanceCredit(UInt8 credit, UInt16 link)
{
	Err error = 0;
	
	if (connections == NULL || link < 0 || link >= maxNumClients)
		return;
	
	if ((error = BtLibSocketAdvanceCredit(btRef, connections[link].remote.socket, credit)) != btLibErrNoError)
		debugPrint(btErrToStr(error));
}

Boolean BtRFCOMMServer::onConnectedInbound(BtLibSocketEventType *event)
{
	if (event->status != btLibErrNoError)
	{
		debugPrint("Inbound connection denied");
		return false;
	}

	// make an alias to avoid costly computing
	BtDevice *client = &connections[activeListener].remote;

	if (client->socket != InvalidSocket)
	{
		debugPrint("Inbound socket already exists");
		BtLibSocketClose(btRef, event->eventData.newSocket);
		return false;
	}

	// save client socket with the listening socket
	client->socket = event->eventData.newSocket;

	// retrieve remote client information
	if (client->friendlyName.name != NULL)
		MemSet(client->friendlyName.name, 32, 0);
		
	client->friendlyName.name = (UInt8 *)MemPtrNew(249);
	client->friendlyName.nameLength = 249;
	
	BtLibGetRemoteDeviceName(btRef, (BtLibDeviceAddressTypePtr)&client->address, &client->friendlyName, 
							 btLibRemoteOnly);

	Char msg[32];
	StrPrintF(msg, "Inbound link socket #%d", activeListener);
	debugPrint(msg);

	// begin listen on a new socket
	if (++numConnectedClients < maxNumClients)
		listen();

	return true;
}

Boolean BtRFCOMMServer::onConnectRequest(BtLibSocketEventType *event)
{
	// accept/deny inbound connection
	// stop advertising on the corresponding listening socket
	if (numConnectedClients >= maxNumClients)
	{
		BtLibSocketRespondToConnection(btRef, event->socket, false);
		debugPrint("Inbound connection denied (max)");
		return true;
	}		
		
	BtLibSocketRespondToConnection(btRef, event->socket, true);
	BtLibSdpServiceRecordStopAdvertising(btRef, sdpHandle);
	
	// save address
	connections[activeListener].remote.address = event->eventData.requestingDevice;
	debugPrint("Connection inbound..");

	return true;
}

Boolean BtRFCOMMServer::onDisconnected(BtLibSocketEventType *event)
{	
	// find disconnecting socket	
	for (UInt16 i = 0; i < maxNumClients; )
		if (connections[i].local != InvalidSocket && connections[i].remote.socket == InvalidSocket)
		{
			disconnect(i);
			Char msg[80];
			StrPrintF(msg, "Socket #%d disconnected", i);
		}
		else
			i++;

	return true;
}