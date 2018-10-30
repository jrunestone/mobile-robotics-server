/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtServerBase.cpp
 * Author: Johan Johanson
 * Date: January 24, 2005
 * Updated: February 26, 2005
 *
 * Description: See BtServerBase.h
 *
*/

#include "BtServerBase.h"

BtServerBase::BtServerBase() : BtBase(), connections(NULL), sdpHandle(0), maxNumClients(0), 
							   numConnectedClients(0), activeListener(0)
{
	MemSet(serviceName, MaxLenService, 0);
	MemSet((void **)&sdpUuid, sizeof(BtLibSdpUuidType), 0);
}

BtServerBase::~BtServerBase()
{

}

Boolean BtServerBase::init(Char *serviceName, UInt16 maxNumClients)
{
	if (connections != NULL || serviceName == NULL || !maxNumClients)
		return false;
	
	if (!BtBase::init())
		return false;
	
	StrNCopy(this->serviceName, serviceName, MaxLenService);
	this->maxNumClients = maxNumClients;
	
	connections = (BtSocketPair *)MemPtrNew(sizeof(BtSocketPair) * maxNumClients);

	if (connections == NULL)
		return false;
		
	MemSet(connections, sizeof(BtSocketPair) * maxNumClients, 0);

	for (UInt16 i = 0; i < maxNumClients; i++)
		connections[i].local = connections[i].remote.socket = InvalidSocket;
	
	return true;
}

void BtServerBase::disconnect()
{
	// stop advertising service
	BtLibSdpServiceRecordStopAdvertising(btRef, sdpHandle);
	BtLibSdpServiceRecordDestroy(btRef, sdpHandle);
	sdpHandle = 0;
	
	// disconnect all clients
	for (UInt16 i = 0; i < maxNumClients; i++)
		disconnect(i);
	
	if (connections != NULL)
	{
		MemPtrFree(connections);
		connections = NULL;
	}

	MemSet(serviceName, MaxLenService, 0);	
	maxNumClients = numConnectedClients = activeListener = 0;
}

Boolean BtServerBase::disconnect(UInt16 client)
{	
	if (connections == NULL || client >= maxNumClients || !numConnectedClients)
		return false;

	Err error = 0;

	// only try to disconnect an active connection
	if (connections[client].local != InvalidSocket || connections[client].remote.socket != InvalidSocket)
	{
		if ((error = BtLibSocketClose(btRef, connections[client].remote.socket)) != btLibErrNoError)
			debugPrint(btErrToStr(error));
		
		if (client == activeListener)
		{
			BtLibSdpServiceRecordStopAdvertising(btRef, sdpHandle);
			BtLibSdpServiceRecordDestroy(btRef, sdpHandle);
			sdpHandle = 0;
		}
		
	    if ((error = BtLibSocketClose(btRef, connections[client].local)) != btLibErrNoError)
	    	debugPrint(btErrToStr(error));
	    
	   	MemSet(&connections[client], sizeof(BtSocketPair), 0);
	   	connections[client].local = connections[client].remote.socket = InvalidSocket;
		numConnectedClients--;
	}
	
	// connection inactive or already disconnected
	return true;
}
	
Boolean BtServerBase::sendPacket(BtDataPacket *packet, UInt16 receiver)
{
	if (packet == NULL || receiver >= maxNumClients)
		return false;

	Err error = 0;

	// send packet through client socket	
	if ((error = BtLibSocketSend(btRef, connections[receiver].remote.socket, (UInt8 *)packet, 
								 sizeof(BtDataPacket) + packet->length)) 
		!= btLibErrPending)
	{
		debugPrint(btErrToStr(error));
		return false;
	}
	
	// send is asynchronous, result is pending
	return true;
}
	
void BtServerBase::authenticate(UInt16 link)
{
	Err error = 0;
	
	if (connections == NULL || link < 0 || link >= maxNumClients)
		return;
		
	if ((error = BtLibLinkSetState(btRef, (BtLibDeviceAddressTypePtr)&connections[link].remote.address,
								   btLibLinkPref_Authenticated, NULL, 0)) != btLibErrNoError)
		debugPrint(btErrToStr(error));
}		
	
void BtServerBase::setAccessibility(UInt32 mode)
{
	Err error = 0;
	
	if ((error = BtLibSetGeneralPreference(btRef, btLibPref_CurrentAccessible, &mode, sizeof(UInt32))) != btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		return;
	}

	localDevice.accessibility = mode;
}

void BtServerBase::setEncryption(Boolean encryption, UInt16 link)
{
	Err error = 0;
	
	if (connections == NULL || link < 0 || link >= maxNumClients)
		return;
		
	if ((error = BtLibLinkSetState(btRef, (BtLibDeviceAddressTypePtr)&connections[link].remote.address, 
								   btLibLinkPref_Encrypted, (void *)&encryption, sizeof(Boolean))) != btLibErrNoError)
		debugPrint(btErrToStr(error));
}		

UInt16 BtServerBase::getMaxNumClients()
{
	return maxNumClients;
}

UInt16 BtServerBase::getNumConnectedClients()
{
	return numConnectedClients;
}

UInt16 BtServerBase::getActiveListener()
{
	return activeListener;
}

Boolean BtServerBase::getAuthenticated(UInt16 link)
{
	Err error = 0;
	Boolean authenticated = false;
	
	if (connections == NULL || link < 0 || link >= maxNumClients)
		return false;
		
	if ((error = BtLibLinkGetState(btRef, (BtLibDeviceAddressTypePtr)&connections[link].remote.address,
							       btLibLinkPref_Authenticated, (void *)&authenticated, sizeof(Boolean))) 
		!= btLibErrNoError)
		debugPrint(btErrToStr(error));
		
	return authenticated;
}

Boolean BtServerBase::getEncryption(UInt16 link)
{
	Err error = 0;
	Boolean encryption = false;
	
	if (connections == NULL || link < 0 || link >= maxNumClients)
		return false;
		
	if ((error = BtLibLinkGetState(btRef, (BtLibDeviceAddressTypePtr)&connections[link].remote.address,
							       btLibLinkPref_Encrypted, (void *)&encryption, sizeof(Boolean))) != btLibErrNoError)
		debugPrint(btErrToStr(error));
		
	return encryption;
}

BtDevice *BtServerBase::getClientDevice(UInt16 client)
{
	if (connections == NULL || client < 0 || client >= maxNumClients)
		return NULL;
		
	return &connections[client].remote;
}

Boolean BtServerBase::onAuthenticationComplete(BtLibManagementEventType *event)
{
	if (event->status == btLibErrNoError)
		debugPrint("Link auth. successful");
	else if (event->status == btLibMeStatusAuthenticateFailure)
		debugPrint("Link auth. failed");
	
	return true;
}		