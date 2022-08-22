// Arduino Frequency Counter
// 20.10.2021
// Stefan Rau
// Factory for the different input modules

#pragma once
#ifndef _ModuleFactory_h
#define _ModuleFactory_h

#include <Arduino.h>
#include "Debug.h"
#include "ModuleTTLCMOS.h"
#include "ModuleAnalog.h"
#include "ModuleHF.h"
#include "ModuleNone.h"
#include "TextBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextModuleFactory : public TextBase
{
public:
	TextModuleFactory();
	~TextModuleFactory();

	String GetObjectName() override;
	String Unknown();
};

/////////////////////////////////////////////////////////////

class ModuleFactory : public I2CBase
{
private:
#ifndef _DebugApplication
	// Commands for remote control
	enum eFunctionCode : char
	{
		TName = 'M' // Code for this class, if controlled remotely
	};
#endif

	bool _mTriggerLampTestOff = false;		  // In queue: Switch lamps off
	ModuleTTLCMOS *_mModuleTTLCMOS = nullptr; // Instance of TTL/CMOS module
	ModuleAnalog *_mModuleAnalog = nullptr;	  // Instance of analog module
	ModuleHF *_mModuleHF = nullptr;			  // Instance of HF module
	ModuleNone *_mModuleNone = nullptr;		  // Instance of dummy module
	ModuleBase *_mSelectedModule = nullptr;	  // Instance of the currently selected module
	TextModuleFactory *_mText = nullptr;	  // Pointer to current text objekt of the class

public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	ModuleFactory(sInitializeModule iInitializeModule);

	~ModuleFactory();

	// Functions that can be called from within main loop

	/// <summary>
	/// Is called periodically from main loop
	/// </summary>
	void loop() override;

#ifndef _DebugApplication
	/// <summary>
	/// Dispatches commands got from en external input, e.g. a serial interface
	/// </summary>
	/// <param name="iModuleIdentifyer">If this matches with the identifyer of this module, then iParameter is analyzed</param>
	/// <param name="iParameter">Parameter or command that is to be analyzed</param>
	/// <returns>Reaction of dispatching</returns>
	String Dispatch(char iModuleIdentifyer, char iParameter) override;
#endif

	/// <summary>
	/// Switches lamps on for all modules
	/// </summary>
	void I2ELampTestOn();

private:
	/// <summary>
	/// Selects the module with the given module code
	/// </summary>
	/// <param name=""></param>
	void _I2ESelectModule(char iModuleCode);

public:
	// Functions that can be called also from tasks

	/// <summary>
	/// Readable name of the module
	/// </summary>
	/// <returns>Gets the current name depending on current language</returns>
	String GetName() override;

	/// <summary>
	/// Get the status of the current module as text
	/// </summary>
	/// <returns>Readable Status</returns>
	String GetStatus();

	/// <summary>
	/// Switches lamps off for all modules - sets a trigger tha will be processed in main loop
	/// </summary>
	void TriggerLampTestOff(); // Schaltet die Lampen wieder aus

	/// <summary>
	/// Reads the currently active module
	/// </summary>
	/// <returns>Instance of the module' object</returns>
	ModuleBase *GetSelectedModule();
};

#endif
