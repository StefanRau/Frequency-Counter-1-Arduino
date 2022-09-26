// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Remote control class, that encapsulates serial access

#pragma once
#ifndef _RemoteControl_h
#define _RemoteControl_h

#include "Debug.h"

// Either debugger uses serial interface or remote control
#ifndef DEBUG_APPLICATION
#define RemoteControlInstance() RemoteControl::GetInstance()
#define RemoteControlGetCommand() RemoteControl::GetInstance()->GetCommand()
#define RemoteControlPrint(Text) RemoteControl::GetInstance()->Print(Text)
#else
#define RemoteControlInstance()
#define RemoteControlGetCommand() ""
#define RemoteControlPrint(Text)
#endif

#ifndef DEBUG_APPLICATION

#include <Arduino.h>
#include "TextBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextRemoteControl : public TextBase
{
public:
	TextRemoteControl();
	~TextRemoteControl();

	String GetObjectName() override;
	String Start();
};

/////////////////////////////////////////////////////////////

class RemoteControl
{
private:
	String _mBuffer;					 // Receiver buffer
	TextRemoteControl *_mText = nullptr; // Pointer to current text objekt of the class

	RemoteControl();
	~RemoteControl();

public:
	/// <summary>
	/// Gets a singleton
	/// </summary>
	/// <returns>Instance of remote control</returns>
	static RemoteControl *GetInstance();

	/// <summary>
	/// Receives a command string from serial interface. Must be called in a cycle.
	/// Cummulates single characters in buffer from serial interface until CR or LF is detected.
	/// </summary>
	/// <returns>Returns a string after serial input detects a CR or LF. Otherwise returns empty string when characters are collected.</returns>
	String GetCommand();

	/// <summary>
	/// Outputs a string to serial interface
	/// </summary>
	/// <param name="iOutput">String to output</param>
	void Print(String iOutput);
};

#endif

#endif