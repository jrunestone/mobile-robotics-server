/*
 * Copyright (c) 2004-2005 Mobile Robotics
 * http://mobilerobotics.sf.net
 *
 * File: main.cpp
 * Author: Johan Johanson
 * Date: November 26, 2004
 * Updated: April 15, 2005
 *
 * Description: Main entry point for application
 *
*/

#define ERROR_CHECK_LEVEL ERROR_CHECK_PARTIAL // enable error alerts

#include <PalmOS.h>

#include "MobileRobotics.h"
#include "BtCustomEvent.h"
#include "resource.h"

FormPtr mainForm;
FieldPtr debugField;
ScrollBarPtr debugFieldScrollBar;

RectangleType pdaBatteryRect;	// palm battery level rect
RectangleType rcxBatteryRect;	// rcx battery level rect
UInt8 pdaPercent;
UInt8 rcxPercent;

MobileRobotics server;	// our bluetooth/ir server manager

DateTimeType counterStart;	// used for counting minutes

// prints out text to the field in the main form field
void debugPrint(Char *str)
{
	if (str != NULL && debugField != NULL)
	{
		// apply time stamp to message
		DateTimeType dateTime;
		Char date[timeStringLength + 3 + StrLen(str) + 1];

		TimSecondsToDateTime(TimGetSeconds(), &dateTime);
		TimeToAscii(dateTime.hour, dateTime.minute, tfColon24h, date);

		StrCat(date, " - ");
		StrNCat(date, str, timeStringLength + StrLen(str) + 1);

		FldSetInsPtPosition(debugField, FldGetTextLength(debugField));
		FldInsert(debugField, date, StrLen(date));
		FldInsert(debugField, "\n", 1);
	}
}

// draws a rounded rectangle frame around the debug field
void drawFieldFrame()
{
	if (debugField == NULL)
		return;

	UInt16 indexField = FrmGetObjectIndex(mainForm, MobileRoboticsDebugField);
	RectangleType fieldRect = { { 0, 0 }, { 0, 0 } };
	FrmGetObjectBounds(mainForm, indexField, &fieldRect);

	fieldRect.topLeft.x -= 1;
	fieldRect.topLeft.y -= 1;
	fieldRect.extent.x += 2;
	fieldRect.extent.y += 2;

	RGBColorType newColor = { 0, 255, 127, 63 };

	WinPushDrawState();
		WinSetForeColorRGB(&newColor, NULL);
		WinDrawRectangleFrame(boldRoundFrame, &fieldRect);
	WinPopDrawState();
}

// draws battery status
void drawBatteryStatus()
{
	Int16 timeout = 0;

	// retrieve battery info
	UInt16 volts = SysBatteryInfo(false, NULL, NULL, &timeout, NULL, NULL, &pdaPercent);
	volts = volts;

	// calculate new coords for pda battery status rect
	UInt16 startX = pdaBatteryRect.topLeft.x + 1;
	UInt16 startY = pdaBatteryRect.topLeft.y;

	UInt16 endX = startX + pdaBatteryRect.extent.x - 2;
	UInt16 endY = startY + pdaBatteryRect.extent.y;

	UInt16 diffY = endY - startY;

	// calculate for pda battery level
	UInt16 newStartY = startY + (UInt16)(((1.0f - ((float)pdaPercent / (float)100.0)) * (float)diffY));

	UInt16 extentX = endX - startX;
	UInt16 extentY = endY - newStartY;

	// pda battery level bar
	RectangleType pdaBar = { { startX, newStartY }, { extentX, extentY } };
	RGBColorType newColor = { 0, 0, 0, 0 };

	// draw pda battery frame
	WinPushDrawState();
		WinSetForeColorRGB(&newColor, NULL);
		WinDrawRectangleFrame(simpleFrame, &pdaBatteryRect);
	WinPopDrawState();

	// calculate new coords for rcx battery status rect
	startX = rcxBatteryRect.topLeft.x + 1;
	startY = rcxBatteryRect.topLeft.y;

	endX = startX + rcxBatteryRect.extent.x - 2;
	endY = startY + rcxBatteryRect.extent.y;

	diffY = endY - startY;

	// if one minute since last call has elapsed,
	// calculate rcx battery level
	DateTimeType curTime;
	TimSecondsToDateTime(TimGetSeconds(), &curTime);

	if ((curTime.minute - counterStart.minute) >= 1)
	{
		UInt16 mV = server.getRCXBatteryLevel();
		const UInt16 maxMV = 9000;
		
		if (mV > 0)
			rcxPercent = (UInt8)((float)mV / (float)maxMV * 100);
		else
			rcxPercent = 0;
			
		// save last time
		TimSecondsToDateTime(TimGetSeconds(), &counterStart);
	}
	
	newStartY = startY + (UInt16)(((1.0f - ((float)rcxPercent / (float)100.0)) * (float)diffY));

	extentX = endX - startX;
	extentY = endY - newStartY;
	
	// rcx battery bar
	RectangleType rcxBar = { { startX, newStartY }, { extentX, extentY } };

	// draw rcx battery frame
	WinPushDrawState();
		WinSetForeColorRGB(&newColor, NULL);
		WinDrawRectangleFrame(simpleFrame, &rcxBatteryRect);
	WinPopDrawState();

	newColor.r = 101;
	newColor.g = 186;
	newColor.b = 206;

	// draw battery levels
	WinPushDrawState();
		WinSetForeColorRGB(&newColor, NULL);
		WinDrawRectangle(&pdaBar, 0);
		WinDrawRectangle(&rcxBar, 0);
	WinPopDrawState();
}

// updates graphics etc each frame
void updateFrame()
{
	drawBatteryStatus();
	server.update();
}

// initializes app
Boolean startApp()
{
	mainForm = NULL;
	debugField = NULL;
	debugFieldScrollBar = NULL;

	pdaBatteryRect.topLeft.x = 21;
	pdaBatteryRect.topLeft.y = 123;
	pdaBatteryRect.extent.x = 7;
	pdaBatteryRect.extent.y = 34;

	rcxBatteryRect.topLeft.x = 34;
	rcxBatteryRect.topLeft.y = 123;
	rcxBatteryRect.extent.x = 7;
	rcxBatteryRect.extent.y = 34;

	FrmGotoForm(MobileRoboticsForm);

	// prevent palm to automatically power off when idle
	SysSetAutoOffTime(0);

	// get start time
	TimSecondsToDateTime(TimGetSeconds(), &counterStart);

	pdaPercent = rcxPercent = 0;

	return true;
}

// initializes Bluetooth server and RCX interface
Boolean startServer()
{
	// Bluetooth radio should be initialized
	debugPrint("Server initializing..");
	if (!server.init("MRRfComm", 1, true))
	{
		debugPrint("Server init error");
		return false;
	}

	debugPrint("Server init OK");

	// set server in listening state, accepting only one client
	if (!server.listen())
		debugPrint("Server listen error");
	else
		debugPrint("Server in listening state");

	return true;
}

// shuts down app
void stopApp()
{
	// shut down server and Bluetooth library
	debugPrint("System shutting down..");
	server.shutdown();
	BtBase::shutdownSystem();
	debugPrint("Shutdown complete");

	WinEraseChars("ok", 2, 47 + 13, 126 + 13 / 2 - 5);
	WinEraseChars("ok", 2, 47 + 13, 141 + 13 / 2 - 5);
	WinDrawChars("--", 2, 47 + 13, 126 + 13 / 2 - 5);
	WinDrawChars("--", 2, 47 + 13, 141 + 13 / 2 - 5);

	FrmCloseAllForms();
}

// application event handler
Boolean frmEventHandler(EventPtr event)
{
	switch (event->eType)
	{
		// form is opening
		case frmOpenEvent:
		{
			mainForm = FrmGetActiveForm();

			UInt16 indexField = FrmGetObjectIndex(mainForm, MobileRoboticsDebugField);
			debugField = (FieldPtr)FrmGetObjectPtr(mainForm, indexField);
			FldSetMaxChars(debugField, maxFieldTextLen);
			FldSetFont(debugField, stdFont);

			UInt16 indexScrollBar = FrmGetObjectIndex(mainForm, MobileRoboticsDebugFieldScrollBar);
			debugFieldScrollBar = (ScrollBarPtr)FrmGetObjectPtr(mainForm, indexScrollBar);

			FrmDrawForm(mainForm);
			drawFieldFrame();

			// connection status
			WinEraseChars("ok", 2, 47 + 13, 126 + 13 / 2 - 5);
			WinEraseChars("ok", 2, 47 + 13, 141 + 13 / 2 - 5);
			WinDrawChars("--", 2, 47 + 13, 126 + 13 / 2 - 5);
			WinDrawChars("--", 2, 47 + 13, 141 + 13 / 2 - 5);

			// try to initialize the Bluetooth radio
			// BtBase will post an event with the radio initialization
			// status once it's complete
			debugPrint("Bluetooth initializing..");
			BtBase::setCallbackRef(&server);
			if (!BtBase::initSystem())
			{
				debugPrint("Bluetooth init error");
				/*// failed, post quit event
				EventType e;
				MemSet((void **)&e, sizeof(EventType), 0);
				e.eType = appStopEvent;
				EvtAddEventToQueue(&e);*/
			}

			break;
		}

		// form is closing
		case frmCloseEvent:
		{
			FrmEraseForm(mainForm);
			FrmDeleteForm(mainForm);

			debugFieldScrollBar = NULL;
			debugField = NULL;
			mainForm = NULL;
			break;
		}

		// our debug field has changed
		case fldChangedEvent:
		{
			if (debugField == NULL || debugFieldScrollBar == NULL)
				return true;

			// do scroll bar boohagi
			UInt16 scrollPos = 0;
			UInt16 textHeight = 0;
			UInt16 fieldHeight = 0;
			UInt16 maxValue = 0;

			FldGetScrollValues(debugField, &scrollPos,  &textHeight, &fieldHeight);

			if (textHeight > fieldHeight)
				maxValue = textHeight - fieldHeight;
			else if (scrollPos)
			  	maxValue = scrollPos;
			else
			  	maxValue = 0;

			SclSetScrollBar(debugFieldScrollBar, scrollPos, 0, maxValue,  fieldHeight - 1);
			return false;
		}

		// scrollbar is updating
		case sclRepeatEvent:
		{
			if (debugFieldScrollBar == NULL)
				return true;

			// do scroll bar boohagi
		    UInt16 newValue = event->data.sclRepeat.newValue;
			static UInt16 curValue = 0;

			if (newValue > curValue)
				FldScrollField(debugField, newValue - curValue, winDown);
			else
				FldScrollField(debugField, curValue - newValue, winUp);

			curValue = newValue;
			return false;
		}

		default:
			return false;
	}

	return true;
}

// application event handler
Boolean appEventHandler(EventPtr event)
{
	switch (event->eType)
	{
		// form is being loaded, show it
		case frmLoadEvent:
		{
			FormPtr form = FrmInitForm(event->data.frmLoad.formID);
			FrmSetActiveForm(form);
			FrmSetEventHandler(form, frmEventHandler);
			break;
		}

		// custom user event
		case firstUserEvent:
		{
			BtCustomEvent *e = (BtCustomEvent *)event;

			// our radio state event
			if (e->data.eventData.type == btCustomRadioStateEvent)
			{
				// radio initialized
				if (e->data.eventData.status == btLibErrRadioInitialized ||
					e->data.eventData.status == btLibErrRadioSleepWake)
				{
					debugPrint("Bluetooth init OK");
					WinDrawChars("ok", 2, 47 + 13, 126 + 13 / 2 - 5);

					// fire up server
					if (!startServer())
					{
						/*EventType evt;
						MemSet((void **)&evt, sizeof(EventType), 0);
						evt.eType = appStopEvent;
						EvtAddEventToQueue(&evt);*/
					}
				}
				else
				{
					// radio error
					debugPrint("Bluetooth radio error");
					WinEraseChars("ok", 2, 47 + 13, 126 + 13 / 2 - 5);
					WinDrawChars("--", 2, 47 + 13, 126 + 13 / 2 - 5);
					/*EventType evt;
					MemSet((void **)&evt, sizeof(EventType), 0);
					evt.eType = appStopEvent;
					EvtAddEventToQueue(&evt);*/
				}
			}

			break;
		}

		default:
			return false;
	}

	return true;
}

// application event loop
void eventLoop()
{
	EventType event;

	// while our event isn't a quit event
	while (event.eType != appStopEvent)
	{
		// get a new event
		EvtGetEvent(&event, evtNoWait);

		// if there's an important event, handle it
		if (event.eType != nilEvent)
		{
			if (!SysHandleEvent(&event))
				if (!MenuHandleEvent(NULL, &event, NULL))
					if (!appEventHandler(&event))
						FrmDispatchEvent(&event);
		}
		else
			updateFrame();	// else update stuff
	}
}

// main application entry point
UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	if (cmd != sysAppLaunchCmdNormalLaunch)
		return 1;

	if (!startApp())
		return 1;

	eventLoop();
	stopApp();

	return 0;
}