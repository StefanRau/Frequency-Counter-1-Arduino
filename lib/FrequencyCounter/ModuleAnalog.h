// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// Driver for module 100 MHz Analog

#pragma once
#ifndef _ModuleAnalog_h
#define _ModuleAnalog_h

#include "ModuleBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextModuleAnalog : public TextBase
{
public:
	TextModuleAnalog();
	~TextModuleAnalog();

	String GetObjectName() override;
	String MenuItem100uV();
	String MenuItem1mV();
	String MenuItem10mV();
	String MenuItem100mV();
	String MenuItem1V();
	String MenuItem10V();
	String MenuItem100V();
};

/////////////////////////////////////////////////////////////

class ModuleAnalog : public ModuleBase
{
private:
	static const int _cNumberOfMenuEntries = 7;
	const uint8_t _cOSelect100uV = 0;
	const uint8_t _cOSelect1mV = 1;
	const uint8_t _cOSelect10mV = 2;
	const uint8_t _cOSelect100mV = 3;
	const uint8_t _cOSelect1V = 4;
	const uint8_t _cOSelect10V = 5;
	const uint8_t _cOSelect100V = 6;

	/// <summary>
	/// Pointer to current text objekt of the class
	/// </summary>
	TextModuleAnalog *_mText;

public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	ModuleAnalog(sInitializeModule iInitializeModule);

	~ModuleAnalog();

	// Functions that can be called from within main loop

	/// <summary>
	/// Activates the module => restores the last stored state
	/// </summary>
	void I2EActivate() override;

	/// <summary>
	/// Deactivates the module - switch off all outputs
	/// </summary>
	void I2EDeactivate() override;

protected:
	/// <summary>
	/// Triggers hardware for selected menu entry
	/// </summary>
	void I2ESelectFunction() override;

public:
	// Functions that can be called also from tasks

	/// <summary>
	/// Readable name of the module
	/// </summary>
	/// <returns>Gets the current name depending on current language</returns>
	String GetName() override;

	/// <summary>
	/// Gets the information about possibility of period measurement of the module.
	/// </summary>
	/// <returns>true: module supports period measurement, false: module does not support period measurement</returns>
	bool IsPeriodMeasurementPossible() override;

	/// <summary>
	/// Gets the information about possibility of counting single events.
	/// </summary>
	/// <returns>true: module supports event counting, false: module does not support event counting</returns>
	bool IsEventCountingPossible() override;

	/// <summary>
	/// Returns text of the current menu item
	/// </summary>
	/// <param name="iMenuEntry">-1: use current menu entry number, >0: use this number</param>
	/// <returns>Text of the menu item</returns>
	String GetCurrentMenuEntry(int iMenuEntry) override;

	/// <summary>
	/// Retrieves the specific code of the module
	/// </summary>
	/// <returns>Module code</returns>
	ModuleBase::eModuleCode GetModuleCode() override;
};

#endif
