/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: RCXInterface.cpp
 * Author: Johan Johanson
 * Date: February 11, 2005
 * Updated: February 11, 2005
 *
 * Description: See RCXInterface.h
 * 
 */
 
 #include "RCXInterface.h"
 
 RCXInterface::RCXInterface() : orRef(sysInvalidRefNum), sendFrequency(72000), recvFrequency(76000), baudRate(2400),
 								parity(parity_odd), dataBits(8), lsbFirst(true), invertData(true), internalIR(true),
 								lastOpcode(0)
 {
 	
 }
 
 RCXInterface::~RCXInterface()
 {
 	shutdown();
 }
 
 // loads the OR library and initializes the IR transceiver
Boolean RCXInterface::init()
 {
 	if (orRef != sysInvalidRefNum)
 		return false;
 	
 	UInt16 status = 0;
 	
 	// open / find library
 	if (SysLibFind("ORLib.prc", &orRef))
 		if (SysLibLoad('libr', 'ORlb', &orRef))
 			return false;

	OrLibOpen(orRef);
 			
 	// identify IR hardware and setup the type of IR used
 	OrLibDeviceType(orRef, NULL, &status, internalIR);
 	
 	// check OmniRemote software status
 	if (status & STATUS_BETAEXPIRED || status & STATUS_PORTBLOCKED || !(status & 0xFF))
 		return false;
 	
 	return true;
 }
 
void RCXInterface::shutdown()
{
	UInt16 numApps;
	
	// close library	
	OrLibClose(orRef, &numApps);
	
	// if we were the last app to close the lib, remove it from memory
	if (!numApps)
		SysLibRemove(orRef);
		
	orRef = sysInvalidRefNum;
}
 
// builds a IR sendable	Lego protocol packet from a number of opcodes
Boolean RCXInterface::buildPacket(const Char *opcodes, UInt16 length, UInt8 *buffer)
{
	if (opcodes == NULL || buffer == NULL || !length)
		return false;
	
	// fill in the header (0x55 0xff 0x00)
	buffer[0] = 0x55;
	buffer[1] = 0xff;
	buffer[2] = 0x00;
	
	UInt16 j = 3, sum = 0;
	Char op = 0;
	
	// insert the opcodes and their complement into buffer
	for (UInt16 i = 0; i < length; i++)
	{
		op = opcodes[i];

		// check if the first opcode in the sequence is the same as in the previous packet
		if (i == 0)
			if (opcodes[i] == lastOpcode)
				op = (lastOpcode ^= 0x08);	// if so, toggle (XOR) its 0x08 bit so that the RCX accepts it
			else
				lastOpcode = op;	// remember the last opcode

		buffer[j++] = op;				// the opcode
		buffer[j++] = (~(op)) & 0xff;	// its complement
		
		// add to the sum
		sum += op;
	}
	
	// insert the the checksum and its complement to the end of the packet
	buffer[j++] = sum & 0xff;
	buffer[j] = (~sum) & 0xff;
	
	return true;	
}	
	
Boolean RCXInterface::sendPacket(UInt8 *data, UInt16 length)
{
	if (orRef == sysInvalidRefNum || data == NULL)
		return false;
	
	// fire up IR
	OrLibStartPlaySample(orRef, sendFrequency);
		
	// calculate the minimum size for the generated sample buffer
	// (1 / baud rate) * bits per frame * data length * 1024
	UInt16 bitsPerFrame = (parity == parity_none) ? 10 : 11;
	UInt16 minBuffLen = (UInt16)((1.0f / (float)baudRate) * bitsPerFrame * length * 1024);
	
	UInt8 sampleBuffer[minBuffLen + 16];
	Int16 sampleLen = sizeof(sampleBuffer);
	
	// generate a sample from the data packet
	OrLibGenSampleFromData(orRef, sampleBuffer, &sampleLen, data, length, baudRate, sendFrequency, (ParityEnum)parity, 
						   lsbFirst, invertData);
		
	// send sample
	if (OrLibSendData(orRef, sampleBuffer, sampleLen, sendFrequency))
	{
		OrLibEndPlaySample(orRef);
		return false;
	}
	
	// put IR to sleep
	OrLibEndPlaySample(orRef);
	
	// receive reply
	//if (!recvPacket(data, &length, 10))
	//	return false;
	
	return true;
}

Boolean RCXInterface::recvPacket(UInt8 *data, UInt16 *length, UInt16 timeout)
{
	if (orRef == sysInvalidRefNum || data == NULL || length == NULL)
		return false;
	
	// start receiving data, fire up the IR	
	if (OrLibStartRcvData(orRef, baudRate) == 0)
	{
		OrLibEndRcvData(orRef);
		return false;
	}
		
	// receive the data
	if (OrLibRcvData(orRef, data, length, recvFrequency, dataBits, (parity != parity_none), lsbFirst, 
				     invertData, timeout))
	{
		OrLibEndRcvData(orRef);
		
		data = NULL;
		*length = 0;
		
		return false;
	}
	
	// done receiving data
	OrLibEndRcvData(orRef);
	
	return true;
}

void RCXInterface::setTransceiverMode(UInt16 mode)
{
	if (orRef != sysInvalidRefNum)
	{
		internalIR = (Boolean)mode;
		OrLibDeviceType(orRef, NULL, NULL, internalIR);
	}
}

UInt16 RCXInterface::getTransceiverMode()
{
	return (UInt16)internalIR;
}

UInt16 RCXInterface::getLibRef()
{
	return orRef;
}

UInt32 RCXInterface::getSendFrequency()
{
	return sendFrequency;
}

UInt32 RCXInterface::getRecvFrequency()
{
	return recvFrequency;
}

UInt32 RCXInterface::getBaudRate()
{
	return baudRate;
}

UInt32 RCXInterface::getParity()
{
	return parity;
}

UInt16 RCXInterface::getDataBits()
{
	return dataBits;
}

Boolean RCXInterface::isLsbFirst()
{
	return lsbFirst;
}

Boolean RCXInterface::isInvertData()
{
	return invertData;
}