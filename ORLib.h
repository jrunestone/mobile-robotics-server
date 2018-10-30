

#ifndef __ORLIB_H__
#define __ORLIB_H__

// return flags for OrLibDeviceType function's DeviceType variable
	
#define	DEVICE_UPGCARD			0x0001		/* OS3.0 upgrade card is installed */
#define	DEVICE_EZPROC				0x0002		/* Dragonball EZ detected */
#define	DEVICE_PALM3				0x0004		/* Internal IR selected (not on EZ processor) */
#define	DEVICE_PALM5				0x0008		/* Internal IR selected (on EZ processor) */
#define DEVICE_HANDSPRING		0x0010		/* Handspring device detected */
#define	DEVICE_PALM3C				0x0020		/* At least OS3.5, not Handspring, with color support */
#define	DEVICE_SPRINGBOARD	0x0040		/* Springboard module is installed */
#define DEVICE_PRISM				0x0080		/* OS3.5 or greater and Prism detected */
#define DEVICE_BYTECOLOR		0x0100		/* Current screen mode has 8 bits of color per pixel */
#define	DEVICE_OS35					0x0200		/* At least OS3.5 */
#define	DEVICE_ORPLUS				0x0400		/* Springboard module with RF support is installed */
#define	DEVICE_UART2				0x0800		/* seropen on this device uses UART#2 */

// return flags for OrLibDeviceType function's Status variable

#define STATUS_UNREGISTERED			0x0100			/* OmniRemote is unregistered (no valid RegCode or Springboard Module detected) */
#define STATUS_BETAEXPIRED				0x0200		/* This Library is a beta version and it has expired, please install a newer version */
#define STATUS_DAYSREMAININGWARN	0x0400		/* Warn the user that they have only x days remaining.  This flag will only be
																							 set once per day during the last 5 days, and is cleared automatically. */
#define STATUS_TEMPORARY					0x0800		/* A temporary regcode has been entered which extended the evaluation period by
																							 five days.  This will still be set even if those days have expired.*/
#define STATUS_PORTBLOCKED				0x1000		/* The IR port is being blocked by another application */


/* TypeDefs */

typedef enum {
	parity_none,
	parity_space,
	parity_mark,
	parity_odd,
	parity_even
	
} ParityEnum;


/* Function declarations */

Err OrLibOpen(UInt16 refnum) SYS_TRAP(sysLibTrapOpen);
Err OrLibClose(UInt16 refnum, UInt16 *numappsP) SYS_TRAP(sysLibTrapClose);
Err OrLibSleep(UInt16 refnum) SYS_TRAP(sysLibTrapSleep);
Err OrLibWake(UInt16 refnum) SYS_TRAP(sysLibTrapWake);
Int16 OrLibVersion(UInt16 refnum, char *VersionString) SYS_TRAP(sysLibTrapCustom);
Err OrLibDeviceType(UInt16 refnum, UInt32 *DeviceType, UInt16 *RegStatus, Boolean InternalIR) SYS_TRAP(sysLibTrapCustom+1);
unsigned char OrLibRecordSample(UInt16 refnum, UInt8 *Sample, Int16 MaxLength, Int16 *Length, UInt32 SampleRate,
		Int16 SilenceThresh, Int16 GapThresh) SYS_TRAP(sysLibTrapCustom+2);
unsigned short OrLibStartTestIR(UInt16 refnum) SYS_TRAP(sysLibTrapCustom+3);
Boolean OrLibTestIR(UInt16 refnum) SYS_TRAP(sysLibTrapCustom+4);
Err OrLibEndTestIR(UInt16 refnum) SYS_TRAP(sysLibTrapCustom+5);
Err OrLibStartPlaySample(UInt16 refnum, UInt32 Playrate) SYS_TRAP(sysLibTrapCustom+6);
Err OrLibPlaySample(UInt16 refnum, UInt8 *Sample, Int16 Length, UInt32 CarrierFreq, Int16 GapThresh, Boolean Repeat) SYS_TRAP(sysLibTrapCustom+7);
Err OrLibEndPlaySample(UInt16 refnum) SYS_TRAP(sysLibTrapCustom+8);
Err OrLibSendData(UInt16 refnum, UInt8 *SampleBuffer, Int16 SampleLength, UInt32 CarrierFreq) SYS_TRAP(sysLibTrapCustom+9);
unsigned short OrLibStartRcvData(UInt16 refnum, UInt32 BaudRate) SYS_TRAP(sysLibTrapCustom+10);
Err OrLibRcvData(UInt16 refnum, UInt8 *Data, UInt16 *Length, UInt32 CarrierFreq, UInt16 DataBits,
		Boolean Parity, Boolean LSBFirst, Boolean InvertData, UInt16 Timeout) SYS_TRAP(sysLibTrapCustom+11);
Err OrLibEndRcvData(UInt16 refnum) SYS_TRAP(sysLibTrapCustom+12);
void OrLibGenSampleFromData(UInt16 refnum, UInt8 *SampleBuffer, Int16 *SampleLength, UInt8 *Data, Int16 Length,
		UInt32 BaudRate, UInt32 CarrierFreq, ParityEnum ParityType, Boolean LSBFirst, Boolean InvertData) SYS_TRAP(sysLibTrapCustom+13);
Int16 OrLibIrDir(UInt16 refnum) SYS_TRAP(sysLibTrapCustom+14);
UInt32 OrLibAutoFreq(UInt16 refnum, UInt32 User, char *TestingStr, Int16 x, Int16 y, UInt16 SoundAmp) SYS_TRAP(sysLibTrapCustom+15);

#endif
