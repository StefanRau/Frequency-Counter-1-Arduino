// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// Driver for test module - that is used, if no hardware module is installed

#pragma once
#ifndef _ModuleNone_h
#define _ModuleNone_h

#include "ModuleBase.h"
#include "TextBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextModuleNone : public TextBase
{
public:
	TextModuleNone();
	~TextModuleNone();

	String GetObjectName() override;
	String MenuItemStart();
	String MenuItemCenter();
	String MenuItemEnd();
};

/////////////////////////////////////////////////////////////

class ModuleNone : public ModuleBase
{

public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	ModuleNone(sInitializeModule iInitializeModule);
	~ModuleNone();

	/// <summary>
	/// Activates the module => restores the last stored state
	/// </summary>
	void I2EActivate() override;

	/// <summary>
	/// Deactivates the module - switch off all outputs
	/// </summary>
	void I2EDeactivate() override;

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

protected:
	/// <summary>
	/// Triggers hardware for selected menu entry
	/// </summary>
	void I2ESelectFunction() override;

private:
	static const int cNumberOfMenuEntries = 3;

	/// <summary>
	/// Pointer to current text objekt of the class
	/// </summary>
	TextModuleNone *mText = nullptr;
};

#endif
