// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// Driver for module 100 MHz TTL / CMOS

#pragma once
#ifndef _ModuleStandard_h
#define _ModuleStandard_h

#include "ModuleBase.h"
#include "TextBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextModuleTTLCMOS : public TextBase
{
public:
	TextModuleTTLCMOS();
	~TextModuleTTLCMOS();

	String GetObjectName() override;
	String MenuItemTTL();
	String MenuItemCMOS();
	String MenuItemOC();
	String MenuItemOE();
};

/////////////////////////////////////////////////////////////

class ModuleTTLCMOS : public ModuleBase
{
private:
	static const int _cNumberOfMenuEntries = 4;
	const uint8_t _cOSelectTTL = 2;
	const uint8_t _cOSelectCMOS = 3;
	const uint8_t _cOSelectOpenEmitter = 4;
	const uint8_t _cOSelectOpenCollector = 5;
	const uint8_t _cOInputSelection0 = 6;
	const uint8_t _cOInputSelection1 = 7;

	// unassigned pins
	const uint8_t _cB2Unassigned = 10;
	const uint8_t _cB3Unassigned = 11;
	const uint8_t _cB4Unassigned = 12;
	const uint8_t _cB5Unassigned = 13;
	const uint8_t _cB6Unassigned = 14;
	const uint8_t _cB7Unassigned = 15;

	TextModuleTTLCMOS *_mText; // Pointer to current text objekt of the class

public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	ModuleTTLCMOS(sInitializeModule iInitializeModule);
	~ModuleTTLCMOS();

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
