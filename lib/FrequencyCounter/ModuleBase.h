// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// Base class of input modules

#ifndef _ModuleBase_h
#define _ModuleBase_h

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include "Debug.h"
#include "TextBase.h"
#include "I2CBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextModuleBase : public TextBase
{
public:
	TextModuleBase();
	~TextModuleBase();

	String GetObjectName() override;
};

/////////////////////////////////////////////////////////////

/// <summary>
/// Base class of input modules
/// </summary>
class ModuleBase : public I2CBase
{
public:
	enum eModuleCode : char
	{
		TModuleTTLCMOS = 'T',
		TModuleAnalog = 'A',
		TModuleHF = 'H',
		TModuleNone = 'N',
		TNoSelection = '-'
	};

private:
	// I/O bits of MCP23017 - for all modues the same
	const uint8_t _cOSelectionFrequency = 0;	 // Selects output relais of frequency output
	const uint8_t _cOSelectionPeriod = 1;		 // Selects output relais of period output
	const uint8_t _cIAddressSelectionButton = 8; // Button at front plate
	const uint8_t _cOAddressLED = 9;			 // LED at front plate
	const int _cEepromIndexMenu = 1;			 // Entry used for selected menu
	const int _cEepromIndexFunction = 2;		 // Entry used for selected function

	TextModuleBase *_mText; // Pointer to current text objekt of the class

protected:
	Adafruit_MCP23X17 *mI2EModule;	 // Reference of port extender
	int mLastMenuEntryNumber;		 // Number of menu entries of the current input module
	int mCurrentMenuEntryNumber = 0; // Number of current menu entry

public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	ModuleBase(sInitializeModule iInitializeModule);

	~ModuleBase();

	/// <summary>
	/// Is called periodically from main loop
	/// </summary>
	void loop() override;

#ifndef _DebugApplication
	/// <summary>
	/// Dispatches commands got from en external input, e.g. a serial interface - only a dummy implementation here
	/// </summary>
	/// <param name="iModuleIdentifyer">If this matches with the identifyer of this module, then iParameter is analyzed</param>
	/// <param name="iParameter">Parameter or command that is to be analyzed</param>
	/// <returns>Reaction of dispatching</returns>
	String Dispatch(char iModuleIdentifyer, char iParameter) override;
#endif

	/// <summary>
	/// Activates the module => restores the last stored state
	/// </summary>
	virtual void I2EActivate();

	/// <summary>
	/// Deactivates the module - switch off all outputs
	/// </summary>
	virtual void I2EDeactivate();

	/// <summary>
	/// Triggers hardware for selected menu entry
	/// </summary>
	virtual void I2ESelectFunction() = 0;

	/// <summary>
	/// Selects the next menu entry
	/// </summary>
	void I2EScrollFunctionUp();

	/// <summary>
	/// Selects the previous menu entry
	/// </summary>
	void I2EScrollFunctionDown();

	/// <summary>
	/// Selects a specific menu entry
	/// </summary>
	/// <param name="iCurrentMenuEntryNumber">Code of the menu item</param>
	void I2ESetCurrentMenuEntryNumber(int iCurrentMenuEntryNumber);

	/// <summary>
	/// Switches front plate LED of the module to the given state
	/// </summary>
	/// <param name="iState">true: LED on, false: LED off</param>
	void I2ESwitchLamp(bool iState);

	/// <summary>
	/// Returns state of front plate key of the module
	/// </summary>
	/// <returns>true: Button is pressed, false: buuton is not pressed</returns>
	bool I2EIsKeySelected();

	/// <summary>
	/// Selects the module
	/// </summary>
	/// <param name="iSelect">true: activae module, false: deactivate the module</param>
	void I2ESelectModule(bool iSelect);

	/// <summary>
	/// Switches on relais for frequency output
	/// </summary>
	void I2ESelectFrequencyCounter();

	/// <summary>
	/// Switches on relais for period output
	/// </summary>
	void I2ESelectPeriodMeasurement();

protected:
	/// <summary>
	/// Called from constructor of derived classes. Initializes the module hardware.
	/// </summary>
	/// <returns>true: module is initialized, false: module is not initialized</returns>
	bool I2EInitialize();

public:
	// Functions that can be called also from tasks

	/// <summary>
	/// Gets the information about possibility of period measurement of the module.
	/// </summary>
	/// <returns>true: module supports period measurement, false: module does not support period measurement</returns>
	virtual bool IsPeriodMeasurementPossible() = 0;

	/// <summary>
	/// Gets the information about possibility of counting single events.
	/// </summary>
	/// <returns>true: module supports event counting, false: module does not support event counting</returns>
	virtual bool IsEventCountingPossible() = 0;

	/// <summary>
	/// Returns text of the current menu item
	/// </summary>
	/// <param name="iMenuEntry">-1: use current menu entry number, >0: use this number</param>
	/// <returns>Text of the menu item</returns>
	virtual String GetCurrentMenuEntry(int iMenuEntry) = 0;

	/// <summary>
	/// Retrieves the specific code of the module
	/// </summary>
	/// <returns>Module code</returns>
	virtual eModuleCode GetModuleCode() = 0;

	/// <summary>
	/// Checks if the module exists in the system
	/// </summary>
	/// <returns>true: yes, false: no</returns>
	bool IsModuleInitialized();

	/// <summary>
	/// Gets the number of the current menu entry
	/// </summary>
	/// <returns>Number of menu entry</returns>
	int GetCurrentMenuEntryNumber();

	/// <summary>
	/// Gets the number of the last menu entry
	/// </summary>
	/// <returns>Number of menu entry</returns>
	int GetLastMenuEntryNumber();

#ifndef _DebugApplication
	/// <summary>
	/// Gets a list of all menu entries
	/// </summary>
	/// <returns>String with comma separated list of menu entries</returns>
	String GetAllMenuEntryItems();
#endif
};

#endif