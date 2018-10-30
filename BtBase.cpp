/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: BtBase.cpp
 * Author: Johan Johanson
 * Date: January 16, 2005
 * Updated: February 26, 2005
 *
 * Description: See BtBase.h
 *
*/

#include "BtBase.h"
#include "BtCustomEvent.h"

const Int16 BtBase::InvalidSocket = 0xFFFF;
UInt16 BtBase::btRef = sysInvalidRefNum;
BtBase *BtBase::callbackRef = NULL;

BtBase::BtBase() : radioInitialized(false)
{

}

BtBase::~BtBase()
{

}

Boolean BtBase::initSystem()
{
	Err error = 0;
	UInt32 btVersion = 0;

	shutdownSystem();

	// look for BT compability and supported OS version
	if (FtrGet(btLibFeatureCreator, btLibFeatureVersion, &btVersion) != errNone)
		return false;

	// find the library, if not found, load it
	if (SysLibFind(btLibName, &btRef))
		if (SysLibLoad(sysFileTLibrary, sysFileCBtLib, &btRef) != errNone)
			return false;

	// open the library
	if ((error = BtLibOpen(btRef, false)) != btLibErrNoError)
	{
		btRef = sysInvalidRefNum;
		return false;
	}
	
	// register Bluetooth event callback method
	if ((error = BtLibRegisterManagementNotification(btRef, managementCallback, 0)) != btLibErrNoError)
		return false;
		
	// application must wait for radio to initialize
	// before performing bluetooth dependent tasks
	
	return true;
}

void BtBase::shutdownSystem()
{
	if (btRef != sysInvalidRefNum)
	{
		BtLibUnregisterManagementNotification(btRef, managementCallback);
		BtLibClose(btRef);
		btRef = sysInvalidRefNum;
	}
}

Boolean BtBase::init()
{
	Err error = 0;

	if (!radioInitialized || btRef == sysInvalidRefNum)
	{
		debugPrint("Bluetooth not initialized");
		return false;
	}

	// retrieve info about the local BT device
	if ((error = BtLibGetGeneralPreference(btRef, btLibPref_LocalClassOfDevice, (void *)&localDevice.classOfDevice, 
										   sizeof(localDevice.classOfDevice)))
		!= btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		return false;
	}

	if ((error = BtLibGetGeneralPreference(btRef, btLibPref_LocalDeviceAddress, (void *)&localDevice.address, 
										   sizeof(localDevice.address)))
		!= btLibErrNoError)
	{
		debugPrint(btErrToStr(error));
		return false;
	}

	return true;
}

void BtBase::shutdown()
{
	if (btRef != sysInvalidRefNum)
		disconnect();
}

void BtBase::setCallbackRef(BtBase *ref)
{
	callbackRef = ref;
}

Boolean BtBase::getRadioInitialized()
{
	return radioInitialized;
}

BtDevice *BtBase::getLocalDevice()
{
	return (btRef == sysInvalidRefNum) ? NULL : &localDevice;
}

void BtBase::btAddrToStr(BtLibDeviceAddressType *address, Char *buffer)
{
	if (btRef == sysInvalidRefNum || address == NULL || buffer != NULL)
		return;

	Err error = 0;
	buffer = (Char *)MemPtrNew(18);

	MemSet(buffer, sizeof(buffer), 0);

	if ((error = BtLibAddrBtdToA(btRef, address, buffer, 18)) != btLibErrNoError)
	{
		MemPtrFree(buffer);
		return;
	}
}

Char *BtBase::btErrToStr(Err btErr)
{
	Char *msg;

	switch (btErr)
	{
		case btLibErrNoError:
			msg = (Char *)"Bt Error: NoError";
			break;

		case btLibErrError:
			msg = (Char *)"Bt Error: Error";
			break;

		case btLibErrNotOpen:
			msg = (Char *)"Bt Error: NotOpen";
			break;

		case btLibErrBluetoothOff:
			msg = (Char *)"Bt Error: BluetoothOff";
			break;

		case btLibErrNoPrefs:
			msg = (Char *)"Bt Error: NoPrefs";
			break;

		case btLibErrAlreadyOpen:
			msg = (Char *)"Bt Error: AlreadyOpen";
			break;

		case btLibErrOutOfMemory:
			msg = (Char *)"Bt Error: OutOfMemory";
			break;

		case btLibErrFailed:
			msg = (Char *)"Bt Error: Failed";
			break;

		case btLibErrInProgress:
			msg = (Char *)"Bt Error: InProgress";
			break;

		case btLibErrParamError:
			msg = (Char *)"Bt Error: ParamError";
			break;

		case btLibErrTooMany:
			msg = (Char *)"Bt Error: TooMany";
			break;

		case btLibErrPending:
			msg = (Char *)"Bt Error: Pending";
			break;

		case btLibErrNotInProgress:
			msg = (Char *)"Bt Error: NotInProgress";
			break;

		case btLibErrRadioInitFailed:
			msg = (Char *)"Bt Error: RadioInitFailed";
			break;

		case btLibErrRadioFatal:
			msg = (Char *)"Bt Error: RadioFatal";
			break;

		case btLibErrRadioInitialized:
			msg = (Char *)"Bt Error: RadioInitialized";
			break;

		case btLibErrRadioSleepWake:
			msg = (Char *)"Bt Error: RadioSleepWake";
			break;

		case btLibErrNoConnection:
			msg = (Char *)"Bt Error: NoConnection";
			break;

		case btLibErrAlreadyRegistered:
			msg = (Char *)"Bt Error: AlreadyRegistered";
			break;

		case btLibErrNoAclLink:
			msg = (Char *)"Bt Error: NoAclLink";
			break;

		case btLibErrSdpRemoteRecord:
			msg = (Char *)"Bt Error: SdpRemoteRecord";
			break;

		case btLibErrSdpAdvertised:
			msg = (Char *)"Bt Error: SdpAdvertised";
			break;

		case btLibErrSdpFormat:
			msg = (Char *)"Bt Error: SdpFormat";
			break;

		case btLibErrSdpNotAdvertised:
			msg = (Char *)"Bt Error: SdpNotAdvertised";
			break;

		case btLibErrSdpQueryVersion:
			msg = (Char *)"Bt Error: SdpQueryVersion";
			break;

		case btLibErrSdpQueryHandle:
			msg = (Char *)"Bt Error: SdpQueryHandle";
			break;

		case btLibErrSdpQuerySyntax:
			msg = (Char *)"Bt Error: SdpQuerySyntax";
			break;

		case btLibErrSdpQueryPduSize:
			msg = (Char *)"Bt Error: SdpQueryPduSize";
			break;

		case btLibErrSdpQueryContinuation:
			msg = (Char *)"Bt Error: SdpQueryContinuation";
			break;

		case btLibErrSdpQueryResources:
			msg = (Char *)"Bt Error: SdpQueryResources";
			break;

		case btLibErrSdpQueryDisconnect:
			msg = (Char *)"Bt Error: SdpQueryDisconnect";
			break;

		case btLibErrSdpInvalidResponse:
			msg = (Char *)"Bt Error: SdpInvalidResponse";
			break;

		case btLibErrSdpAttributeNotSet:
			msg = (Char *)"Bt Error: SdpAttributeNotSet";
			break;

		case btLibErrSdpMapped:
			msg = (Char *)"Bt Error: SdpMapped";
			break;

		case btLibErrSocket:
			msg = (Char *)"Bt Error: Socket";
			break;

		case btLibErrSocketProtocol:
			msg = (Char *)"Bt Error: SocketProtocol";
			break;

		case btLibErrSocketRole:
			msg = (Char *)"Bt Error: SocketRole";
			break;

		case btLibErrSocketPsmUnavailable:
			msg = (Char *)"Bt Error: SocketPsmUnavailable";
			break;

		case btLibErrSocketChannelUnavailable:
			msg = (Char *)"Bt Error: SocketChannelUnavailable";
			break;

		case btLibErrSocketUserDisconnect:
			msg = (Char *)"Bt Error: SocketUserDisconnect";
			break;

		case btLibErrCanceled:
			msg = (Char *)"Bt Error: Canceled";
			break;

		case btLibErrBusy:
			msg = (Char *)"Bt Error: Busy";
			break;

		case btLibMeStatusUnknownHciCommand:
			msg = (Char *)"Bt Error: MeStatusUnknownHciCommand";
			break;

		case btLibMeStatusNoConnection:
			msg = (Char *)"Bt Error: MeStatusNoConnection";
			break;

		case btLibMeStatusHardwareFailure:
			msg = (Char *)"Bt Error: MeStatusHardwareFailure";
			break;

		case btLibMeStatusPageTimeout:
			msg = (Char *)"Bt Error: MeStatusPageTimeout";
			break;

		case btLibMeStatusAuthenticateFailure:
			msg = (Char *)"Bt Error: MeStatusAuthenticateFailure";
			break;

		case btLibMeStatusMissingKey:
			msg = (Char *)"Bt Error: MeStatusMissingKey";
			break;

		case btLibMeStatusMemoryFull:
			msg = (Char *)"Bt Error: MeStatusMemoryFull";
			break;

		case btLibMeStatusConnnectionTimeout:
			msg = (Char *)"Bt Error: MeStatusConnnectionTimeout";
			break;

		case btLibMeStatusMaxConnections:
			msg = (Char *)"Bt Error: MeStatusMaxConnections";
			break;

		case btLibMeStatusMaxScoConnections:
			msg = (Char *)"Bt Error: MeStatusMaxScoConnections";
			break;

		case btLibMeStatusMaxAclConnections:
			msg = (Char *)"Bt Error: MeStatusMaxAclConnections";
			break;

		case btLibMeStatusCommandDisallowed:
			msg = (Char *)"Bt Error: MeStatusCommandDisallowed";
			break;

		case btLibMeStatusLimitedResources:
			msg = (Char *)"Bt Error: MeStatusLimitedResources";
			break;

		case btLibMeStatusSecurityError:
			msg = (Char *)"Bt Error: MeStatusSecurityError";
			break;

		case btLibMeStatusPersonalDevice:
			msg = (Char *)"Bt Error: MeStatusPersonalDevice";
			break;

		case btLibMeStatusHostTimeout:
			msg = (Char *)"Bt Error: MeStatusHostTimeout";
			break;

		case btLibMeStatusUnsupportedFeature:
			msg = (Char *)"Bt Error: MeStatusUnsupportedFeature";
			break;

		case btLibMeStatusInvalidHciParam:
			msg = (Char *)"Bt Error: MeStatusInvalidHciParam";
			break;

		case btLibMeStatusUserTerminated:
			msg = (Char *)"Bt Error: MeStatusUserTerminated";
			break;

		case btLibMeStatusLowResources:
			msg = (Char *)"Bt Error: MeStatusLowResources";
			break;

		case btLibMeStatusPowerOff:
			msg = (Char *)"Bt Error: MeStatusPowerOff";
			break;

		case btLibMeStatusLocalTerminated:
			msg = (Char *)"Bt Error: MeStatusLocalTerminated";
			break;

		case btLibMeStatusRepeatedAttempts:
			msg = (Char *)"Bt Error: MeStatusRepeatedAttempts";
			break;

		case btLibMeStatusPairingNotAllowed:
			msg = (Char *)"Bt Error: MeStatusPairingNotAllowed";
			break;

		case btLibMeStatusUnknownLmpPDU:
			msg = (Char *)"Bt Error: MeStatusUnknownLmpPDU";
			break;

		case btLibMeStatusUnsupportedRemote:
			msg = (Char *)"Bt Error: MeStatusUnsupportedRemote";
			break;

		case btLibMeStatusScoOffsetRejected:
			msg = (Char *)"Bt Error: MeStatusScoOffsetRejected";
			break;

		case btLibMeStatusScoIntervalRejected:
			msg = (Char *)"Bt Error: MeStatusScoIntervalRejected";
			break;

		case btLibMeStatusScoAirModeRejected:
			msg = (Char *)"Bt Error: MeStatusScoAirModeRejected";
			break;

		case btLibMeStatusInvalidLmpParam:
			msg = (Char *)"Bt Error: MeStatusInvalidLmpParam";
			break;

		case btLibMeStatusUnspecifiedError:
			msg = (Char *)"Bt Error: MeStatusUnspecifiedError";
			break;

		case btLibMeStatusUnsupportedLmpParam:
			msg = (Char *)"Bt Error: MeStatusUnsupportedLmpParam";
			break;

		case btLibMeStatusRoleChangeNotAllowed:
			msg = (Char *)"Bt Error: MeStatusRoleChangeNotAllowed";
			break;

		case btLibMeStatusLmpResponseTimeout:
			msg = (Char *)"Bt Error: MeStatusLmpResponseTimeout";
			break;

		case btLibMeStatusLmpTransdCollision:
			msg = (Char *)"Bt Error: MeStatusLmpTransdCollision";
			break;

		case btLibMeStatusLmpPduNotAllowed:
			msg = (Char *)"Bt Error: MeStatusLmpPduNotAllowed";
			break;

		case btLibL2DiscReasonUnknown:
			msg = (Char *)"Bt Error: L2DiscReasonUnknown";
			break;

		case btLibL2DiscUserRequest:
			msg = (Char *)"Bt Error: L2DiscUserRequest";
			break;

		case btLibL2DiscRequestTimeout:
			msg = (Char *)"Bt Error: L2DiscRequestTimeout";
			break;

		case btLibL2DiscLinkDisc:
			msg = (Char *)"Bt Error: L2DiscLinkDisc";
			break;

		case btLibL2DiscQosViolation:
			msg = (Char *)"Bt Error: L2DiscQosViolation";
			break;

		case btLibL2DiscSecurityBlock:
			msg = (Char *)"Bt Error: L2DiscSecurityBlock";
			break;

		case btLibL2DiscConnPsmUnsupported:
			msg = (Char *)"Bt Error: L2DiscConnPsmUnsupported";
			break;

		case btLibL2DiscConnSecurityBlock:
			msg = (Char *)"Bt Error: L2DiscConnSecurityBlock";
			break;

		case btLibL2DiscConnNoResources:
			msg = (Char *)"Bt Error: L2DiscConnNoResources";
			break;

		case btLibL2DiscConfigUnacceptable:
			msg = (Char *)"Bt Error: L2DiscConfigUnacceptable";
			break;

		case btLibL2DiscConfigReject:
			msg = (Char *)"Bt Error: L2DiscConfigReject";
			break;

		case btLibL2DiscConfigOptions:
			msg = (Char *)"Bt Error: L2DiscConfigOptions";
			break;

		case btLibServiceShutdownAppUse:
			msg = (Char *)"Bt Error: ServiceShutdownAppUse";
			break;

		case btLibServiceShutdownPowerCycled:
			msg = (Char *)"Bt Error: ServiceShutdownPowerCycled";
			break;

		case btLibServiceShutdownAclDrop:
			msg = (Char *)"Bt Error: ServiceShutdownAclDrop";
			break;

		case btLibServiceShutdownTimeout:
			msg = (Char *)"Bt Error: ServiceShutdownTimeout";
			break;

		case btLibServiceShutdownDetached:
			msg = (Char *)"Bt Error: ServiceShutdownDetached";
			break;

		case btLibErrInUseByService:
			msg = (Char *)"Bt Error: InUseByService";
			break;

		case btLibErrNoPiconet:
			msg = (Char *)"Bt Error: NoPiconet";
			break;

		case btLibErrRoleChange:
			msg = (Char *)"Bt Error: RoleChange";
			break;

		case btLibNotYetSupported:
			msg = (Char *)"Bt Error: NotYetSupported";
			break;

		case btLibErrSdpNotMapped:
			msg = (Char *)"Bt Error: SdpNotMapped";
			break;

		case btLibErrAlreadyConnected:
			msg = (Char *)"Bt Error: AlreadyConnected";
			break;

		default:
			msg = (Char *)"Bt Error: Unknown BT error";
			break;
	}

	return msg;
}

void BtBase::debugPrint(Char *str)
{

}

void BtBase::managementCallback(BtLibManagementEventType *event, UInt32 context)
{
	if (callbackRef == NULL)
		return;

	// handle low level Bluetooth management events
	switch (event->event)
	{
		case btLibManagementEventAccessibilityChange:
			callbackRef->onAccessibilityChange(event);
			break;

		case btLibManagementEventACLConnectInbound:
			callbackRef->onACLConnectInbound(event);
			break;

		case btLibManagementEventACLConnectOutbound:
			callbackRef->onACLConnectOutbound(event);
			break;

		case btLibManagementEventACLDisconnect:
			callbackRef->onACLDisconnect(event);
			break;

		case btLibManagementEventAuthenticationComplete:
			callbackRef->onAuthenticationComplete(event);
			break;

		case btLibManagementEventEncryptionChange:
			callbackRef->onEncryptionChange(event);
			break;

		case btLibManagementEventInquiryCanceled:
			callbackRef->onInquiryCanceled(event);
			break;

		case btLibManagementEventInquiryComplete:
			callbackRef->onInquiryComplete(event);
			break;

		case btLibManagementEventInquiryResult:
			callbackRef->onInquiryResult(event);
			break;

		case btLibManagementEventLocalNameChange:
			callbackRef->onLocalNameChange(event);
			break;

		case btLibManagementEventModeChange:
			callbackRef->onModeChange(event);
			break;

		case btLibManagementEventNameResult:
			callbackRef->onNameResult(event);
			break;

		case btLibManagementEventPairingComplete:
			callbackRef->onPairingComplete(event);
			break;

		case btLibManagementEventPasskeyRequest:
			callbackRef->onPasskeyRequest(event);
			break;

		case btLibManagementEventPasskeyRequestComplete:
			callbackRef->onPasskeyRequestComplete(event);
			break;

		case btLibManagementEventPiconetCreated:
			callbackRef->onPiconetCreated(event);
			break;

		case btLibManagementEventPiconetDestroyed:
			callbackRef->onPiconetDestroyed(event);
			break;

		case btLibManagementEventRadioState:
			callbackRef->onRadioState(event);
			break;

		case btLibManagementEventRoleChange:
			callbackRef->onRoleChange(event);
			break;

		default: break;
	}
}

void BtBase::socketCallback(BtLibSocketEventType *event, UInt32 context)
{
	if (callbackRef == NULL)
		return;

	// handle Bluetooth socket level protocol events
	switch (event->event)
	{
		case btLibSocketEventConnectedInbound:
			callbackRef->onConnectedInbound(event);
			break;

		case btLibSocketEventConnectedOutbound:
			callbackRef->onConnectedOutbound(event);
			break;

		case btLibSocketEventConnectRequest:
			callbackRef->onConnectRequest(event);
			break;

		case btLibSocketEventData:
			callbackRef->onData(event);
			break;

		case btLibSocketEventDisconnected:
			callbackRef->onDisconnected(event);
			break;

		case btLibSocketEventSdpServiceRecordHandle:
			callbackRef->onSdpServiceRecordHandle(event);
			break;

		case btLibSocketEventSdpGetAttribute:
			callbackRef->onSdpGetAttribute(event);
			break;

		case btLibSocketEventSdpGetStringLen:
			callbackRef->onSdpGetStringLen(event);
			break;

		case btLibSocketEventSdpGetNumListEntries:
			callbackRef->onSdpGetNumListEntries(event);
			break;

		case btLibSocketEventSdpGetNumLists:
			callbackRef->onSdpGetNumLists(event);
			break;

		case btLibSocketEventSdpGetRawAttribute:
			callbackRef->onSdpGetRawAttribute(event);
			break;

		case btLibSocketEventSdpGetRawAttributeSize:
			callbackRef->onSdpGetRawAttributeSize(event);
			break;

		case btLibSocketEventSdpGetServerChannelByUuid:
			callbackRef->onSdpGetServerChannelByUuid(event);
			break;

		case btLibSocketEventSdpGetPsmByUuid:
			callbackRef->onSdpGetPsmByUuid(event);
			break;

		case btLibSocketEventSendComplete:
			callbackRef->onSendComplete(event);
			break;

		default: break;
	}
}

Boolean BtBase::onAccessibilityChange(BtLibManagementEventType *event)
{
	localDevice.accessibility = event->eventData.accessible;
	return true;
}

Boolean BtBase::onACLConnectInbound(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onACLConnectOutbound(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onACLDisconnect(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onAuthenticationComplete(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onEncryptionChange(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onInquiryCanceled(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onInquiryComplete(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onInquiryResult(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onLocalNameChange(BtLibManagementEventType *event)
{
	if (localDevice.friendlyName.name != NULL)
		MemPtrFree(localDevice.friendlyName.name);

	localDevice.friendlyName.name = (UInt8 *)MemPtrNew(StrLen((Char *)event->eventData.nameResult.name.name));
	MemMove(localDevice.friendlyName.name, event->eventData.nameResult.name.name, 
			event->eventData.nameResult.name.nameLength);
			
//	Char msg[StrLen("Local name: ") + StrLen((Char *)event->eventData.nameResult.name.name) + 1];
//	StrPrintF(msg, "Local name: %s", event->eventData.nameResult.name.name);
//	debugPrint(msg);
	
	return true;
}

Boolean BtBase::onModeChange(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onNameResult(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onPairingComplete(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onPasskeyRequest(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onPasskeyRequestComplete(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onPiconetCreated(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onPiconetDestroyed(BtLibManagementEventType *event)
{
	return true;
}

Boolean BtBase::onRadioState(BtLibManagementEventType *event)
{
	BtCustomEvent radioEvent;
	MemSet((void **)&radioEvent, sizeof(BtCustomEvent), 0);
	
	radioEvent.eType = firstUserEvent;
	radioEvent.data.eventData.type = btCustomRadioStateEvent;
	radioEvent.data.eventData.status = event->status;
	
	radioInitialized = (event->status == btLibErrRadioInitialized);
		
	// post event
	EvtAddEventToQueue((EventPtr)&radioEvent);
	return true;
}

Boolean BtBase::onRoleChange(BtLibManagementEventType *event)
{		
	return true;
}

Boolean BtBase::onConnectedInbound(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onConnectedOutbound(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onConnectRequest(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onData(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onDisconnected(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpServiceRecordHandle(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetAttribute(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetStringLen(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetNumListEntries(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetNumLists(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetRawAttribute(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetRawAttributeSize(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetServerChannelByUuid(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSdpGetPsmByUuid(BtLibSocketEventType *event)
{
	return true;
}

Boolean BtBase::onSendComplete(BtLibSocketEventType *event)
{
	return true;
}