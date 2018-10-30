#ifndef RCXINTERFACE_
#define RCXINTERFACE_

/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: RCXInterface.h
 * Author: Johan Johanson
 * Date: February 10, 2005
 * Updated: April 15, 2005
 *
 * Description: Wraps IR functions to be able communicate with the RCX
 * 				Uses the OmniRemote developer kit to emulate the RCX consumer
 * 				infrared transceiver (http://www.pacificneotek.com/)
 * 
 * 				Written and tested with the OmniRemote Professional 2.14 software
 * 				and the OmniRemote Library 1.59
 * 
 * 				IR settings used:
 * 					2400 baud, 72/76kHz, odd parity, 1 stop bit, 8 data bits, invert data, LSB first
 * 
 * 				Basically this is how a Lego Protocol packet looks like:
 * 				
 * 				0x55 0xff 0x00 O1 ~O1 O2 ~O2 .. On ~On S ~S
 * 				
 * 				Where the first three bytes (the header) are constants
 * 				The following n bytes are the opcodes/instructions for the RCX to execute
 * 				The last byte is the checksum, the sum of all the opcode bytes
 * 				
 * 				NOTE: All opcodes are followed by its complement, as is the checksum
 * 
 * 				You may only send the same opcode packet twice in a row if you toggle the 0x08 bit in the opcode
 * 				being sent (the RCX doesn't execute the same opcode twice in a row)
 * 
 * 				The RCX immediately echoes the sent packet, with the bytes backwards
 * 
 * 				References:
 * 				http://graphics.stanford.edu/~kekoa/rcx/
 * 				http://www.generation5.org/content/2001/rob08.asp
 *
*/

#include <PalmOS.h>

#include "ORLib.h"
#include "sections.h"

class RCXInterface
{
public:
	RCXInterface() EXT_SEG;
	virtual ~RCXInterface() EXT_SEG;
	
	virtual Boolean init() EXT_SEG;
	virtual void shutdown() EXT_SEG;
	
	virtual Boolean buildPacket(const Char *opcodes, UInt16 length, UInt8 *buffer) EXT_SEG;
	virtual Boolean sendPacket(UInt8 *data, UInt16 length) EXT_SEG;
	virtual Boolean recvPacket(UInt8 *data, UInt16 *length, UInt16 timeout) EXT_SEG;
	
	// 0 = internal, 1 = external
	void setTransceiverMode(UInt16 mode) EXT_SEG;
	UInt16 getTransceiverMode() EXT_SEG;
	
	UInt16 getLibRef();
	
	UInt32 getSendFrequency();
	UInt32 getRecvFrequency();
	UInt32 getBaudRate();
	UInt32 getParity();
	
	UInt16 getDataBits();
	
	Boolean isLsbFirst();
	Boolean isInvertData();
	
protected:
	UInt16 orRef;
	
	UInt32 sendFrequency;
	UInt32 recvFrequency;
	UInt32 baudRate;
	UInt32 parity;
	
	UInt16 dataBits;

	Boolean lsbFirst;
	Boolean invertData;
	Boolean internalIR;
	
	Char lastOpcode;	// to automatically toggle the 0x08 bit if sent twice in a row
};

#endif