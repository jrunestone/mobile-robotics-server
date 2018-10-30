#ifndef BTBASE_
#define BTBASE_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtBase.h
 * Author: Johan Johanson
 * Date: January 16, 2005
 * Updated: February 26, 2005
 *
 * Description: Defines an abstract Bluetooth socket class
 * 				Acts as a base for server and client socket implementations
 *
*/

#include <PalmOS.h>
#include <BtLib.h>

#include "BtDevice.h"

class BtBase
{
public:
	BtBase();
	virtual ~BtBase();

	static Boolean initSystem();
	static void shutdownSystem();

	virtual Boolean init();
	virtual void shutdown();

	virtual void disconnect() = 0;

	static void setCallbackRef(BtBase *ref);
	virtual void setAccessibility(UInt32 mode) = 0;

	Boolean getRadioInitialized();
	BtDevice *getLocalDevice();

	static void btAddrToStr(BtLibDeviceAddressType *address, Char *buffer);
	static Char *btErrToStr(Err btErr);

protected:
	virtual void debugPrint(Char *str);

	static void managementCallback(BtLibManagementEventType *event, UInt32 context);
	static void socketCallback(BtLibSocketEventType *event, UInt32 context);

	virtual Boolean onAccessibilityChange(BtLibManagementEventType *event);
	virtual Boolean onACLConnectInbound(BtLibManagementEventType *event);
	virtual Boolean onACLConnectOutbound(BtLibManagementEventType *event);
	virtual Boolean onACLDisconnect(BtLibManagementEventType *event);
	virtual Boolean onAuthenticationComplete(BtLibManagementEventType *event);
	virtual Boolean onEncryptionChange(BtLibManagementEventType *event);
	virtual Boolean onInquiryCanceled(BtLibManagementEventType *event);
	virtual Boolean onInquiryComplete(BtLibManagementEventType *event);
	virtual Boolean onInquiryResult(BtLibManagementEventType *event);
	virtual Boolean onLocalNameChange(BtLibManagementEventType *event);
	virtual Boolean onModeChange(BtLibManagementEventType *event);
	virtual Boolean onNameResult(BtLibManagementEventType *event);
	virtual Boolean onPairingComplete(BtLibManagementEventType *event);
	virtual Boolean onPasskeyRequest(BtLibManagementEventType *event);
	virtual Boolean onPasskeyRequestComplete(BtLibManagementEventType *event);
	virtual Boolean onPiconetCreated(BtLibManagementEventType *event);
	virtual Boolean onPiconetDestroyed(BtLibManagementEventType *event);
	virtual Boolean onRadioState(BtLibManagementEventType *event);
	virtual Boolean onRoleChange(BtLibManagementEventType *event);

	virtual Boolean onConnectedInbound(BtLibSocketEventType *event);
	virtual Boolean onConnectedOutbound(BtLibSocketEventType *event);
	virtual Boolean onConnectRequest(BtLibSocketEventType *event);
	virtual Boolean onData(BtLibSocketEventType *event);
	virtual Boolean onDisconnected(BtLibSocketEventType *event);
	virtual Boolean onSdpServiceRecordHandle(BtLibSocketEventType *event);
	virtual Boolean onSdpGetAttribute(BtLibSocketEventType *event);
	virtual Boolean onSdpGetStringLen(BtLibSocketEventType *event);
	virtual Boolean onSdpGetNumListEntries(BtLibSocketEventType *event);
	virtual Boolean onSdpGetNumLists(BtLibSocketEventType *event);
	virtual Boolean onSdpGetRawAttribute(BtLibSocketEventType *event);
	virtual Boolean onSdpGetRawAttributeSize(BtLibSocketEventType *event);
	virtual Boolean onSdpGetServerChannelByUuid(BtLibSocketEventType *event);
	virtual Boolean onSdpGetPsmByUuid(BtLibSocketEventType *event);
	virtual Boolean onSendComplete(BtLibSocketEventType *event);

public:
	static const Int16 InvalidSocket;

protected:
	static UInt16 btRef;
	static BtBase *callbackRef;

	Boolean radioInitialized;

	BtDevice localDevice;
};

#endif