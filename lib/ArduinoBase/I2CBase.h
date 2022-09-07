// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Base class of all classes that implement I2C based hardware

#pragma once
#ifndef _I2CBase_h
#define _I2CBase_h

#include <Arduino.h>
#include "ProjectBase.h"
#include "TextBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextI2CBase : public TextBase
{
public:
	TextI2CBase();
	~TextI2CBase();

	String GetObjectName() override;
	String ModuleInitialized();
	String ModuleNotInitialized();
};

/////////////////////////////////////////////////////////////

/// <summary>
/// Base class of all classes that implement I2C based hardware
/// </summary>
class I2CBase : public ProjectBase
{
public:
	struct sInitializeModule
	{
		int SettingsAddress;
		int NumberOfSettings;
		short I2CAddress;
	};

private:
	/// <summary>
	/// Pointer to current text objekt of the class
	/// </summary>
	TextI2CBase *_mText = nullptr;

protected:
	/// <summary>
	/// Flags the module as availlable and ready
	/// </summary>
	bool mModuleIsInitialized = false;

	/// <summary>
	/// I2C address of the module, if -1, then this value is ignored
	/// </summary>
	short mI2CAddress = -1;

	// public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	I2CBase(sInitializeModule iInitializeModule);

	~I2CBase();

public:
	/// <summary>
	/// Is called periodically from main loop
	/// </summary>
	virtual void loop() = 0;

	/// <summary>
	/// Readable name of the module
	/// </summary>
	/// <returns>Gets the current name depending on current language</returns>
	virtual String GetName() = 0;

	/// <summary>
	/// Get the status of the current module as text
	/// </summary>
	/// <returns>Readable Status</returns>
	String GetStatus();

protected:
	/// <summary>
	/// Maps boolean to voltage level
	/// </summary>
	/// <param name="ivalue">true or false</param>
	/// <returns>HIGH or LOW</returns>
	static uint8_t Bool2State(bool ivalue);
};

#endif
