// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Base class of all modules that can store settings

#pragma once
#ifndef _ProjectBase_h
#define _ProjectBase_h

#include <Arduino.h>

#ifdef EXTERNAL_EEPROM
#include <I2C_eeprom.h>
#endif

#ifdef ARDUINO_AVR_NANO_EVERY
#include <EEPROM.h>
#else
#ifndef EXTERNAL_EEPROM
#define NO_EEPROM
#warning "No storage for settings"
#endif
#endif

#include "Debug.h"

class ProjectBase
{
public:
#ifndef DEBUG_APPLICATION
	// Global commands for remote control
	enum eFunctionCode : char
	{
		TParameterGetCurrent = '?', // Get current state or current function
		TParameterGetAll = '*',		// Get a list or all possibilities
		TReturnUnknown = '?'		// return of "unknown"
	};
#endif

	const unsigned char cNullSetting = 255; // There is either no setting in EEPROM or no EEPROM defined

private:
	int _mSettingAdddress = -1; // EEPROM Address of the settings of this module. Per default, the setting is inactive.
	int _mNumberOfSettings = 0; // Number of reserved settings in the EEPROM.

protected:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iSettingsAddress">1st Address of the EEPROM for storing the current value, if -1, then this value is ignored</param>
	/// <param name="iNumberOfSettings">Number of settings, relevant for this instance. If iSettingsAddress = -1 => this parameter is ignored and set to 0.</param>
	ProjectBase(int iSettingsAddress, int iNumberOfSettings);

	/// <summary>
	/// Constructor without setting
	/// </summary>
	ProjectBase();

	~ProjectBase();

public:
#ifdef EXTERNAL_EEPROM
	/// <summary>
	/// Sets the I2C address of the large EEPROM
	/// </summary>
	/// <param name="iI2CAddress">Address of the EEPROM</param>
	static void SetI2CAddressGlobalEEPROM(short iI2CAddress);

	/// <summary>
	/// Gets an instance of the EEPROM
	/// </summary>
	/// <returns>Instance of the EEPROM</returns>
	static I2C_eeprom *GetI2CGlobalEEPROM();
#endif

#ifndef DEBUG_APPLICATION
	/// <summary>
	/// Dispatches commands got from en external input, e.g. a serial interface
	/// </summary>
	/// <param name="iModuleIdentifyer">If this matches with the identifyer of this module, then iParameter is analyzed</param>
	/// <param name="iParameter">Parameter or command that is to be analyzed</param>
	/// <returns>Reaction of dispatching</returns>
	virtual String Dispatch(char iModuleIdentifyer, char iParameter) = 0;
#endif

	/// <summary>
	/// Sets the verbose mode fo all modules
	/// </summary>
	/// <param name="iVerboseMode">true: verbose mode on, false: verbose mode off</param>
	static void SetVerboseMode(bool iVerboseMode);

	/// <summary>
	/// Sets the verbose mode fo all modules
	/// </summary>
	/// <returns>The stored verbose mode</returns>
	static bool GetVerboseMode();

protected:
	/// <summary>
	/// Saves a setting parameter in the internal EEPROM, if the settings address is larger or equal than 0
	/// </summary>
	/// <param name="iNumberOfSetting">The number of the current setting.</param>
	/// <param name="iValue">Value to be saved. A blank value is not stored.</param>
	void SetSetting(int iNumberOfSetting, char iValue);

	/// <summary>
	/// Reads the setting parameter from the internal EEPROM, if the settings address is larger or equal than 0
	/// </summary>
	/// <param name="iNumberOfSetting">The number of the current setting.</param>
	/// <returns>Value from EEPROM. Returns blank for settings address is smaller than 0.</returns>
	char GetSetting(int iNumberOfSetting);
};

#endif