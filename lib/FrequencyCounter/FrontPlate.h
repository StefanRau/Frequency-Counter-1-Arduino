// Arduino Frequency Counter
// 21.10.2021
// Stefan Rau
// Front plate ressources like keys and LEDs - delegates events to counter and modules

#pragma once
#ifndef _FrontPlate_h
#define _FrontPlate_h

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include "Debug.h"
#include "I2CBase.h"
#include "Counter.h"
#include "LCDHandler.h"
#include "ModuleFactory.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextFrontPlate : public TextBase
{
public:
	TextFrontPlate();
	~TextFrontPlate();

	String GetObjectName() override;
	String InitError();
	String InitErrorCounterRequired();
	String InitErrorModuleFactoryRequired();
	String InitErrorLCDRequired();
	String ErrorPlausibilityViolation();
};

// static TextFrontPlate gTextFrontPlate;

/////////////////////////////////////////////////////////////

/// <summary>
/// Front plate handling
/// </summary>
class FrontPlate : public I2CBase
{
private:
	enum eMenuKeyCode : char
	{
		TMenuKeyUp = 'U',
		TMenuKeyDown = 'D',
		TMenuKeyNo = 'N'
	};

	// MCP23017 port extender of front plate
	Adafruit_MCP23X17 *_mI2EModule = nullptr;

	// I/O bits of MCP23017
	// Frequency measurement: Key = A3, LED = B0
	const uint8_t _cIKeySelectFrequency = 3;
	const uint8_t _cOLEDSelectFrequency = 8;
	// Period measurement positive level: Key = A4, LED = B1
	const uint8_t _cIKeySelectTPositive = 4;
	const uint8_t _cOLEDSelectTPositive = 9;
	// Period measurement negative level: Key = A5, LED = B2
	const uint8_t _cIKeySelectTNegative = 5;
	const uint8_t _cOLEDSelectTNegative = 10;
	// Period measurement positive edge: Key = A6, LED = B3
	const uint8_t _cIKeySelectTEdgePositive = 6;
	const uint8_t _cOLEDSelectTEdgePositive = 11;
	// Period measurement negative edge: Key = A7, LED = B4
	const uint8_t _cIKeySelectTEdgeNegative = 7;
	const uint8_t _cOLEDSelectTEdgeNegative = 12;
	// Menu keys
	const uint8_t _cIKeySelectMenuUp = 0;
	const uint8_t _cIKeySelectMenuDown = 1;

	// unassigned pins
	const uint8_t _cA2Unassigned = 2;
	const uint8_t _cB5Unassigned = 13;
	const uint8_t _cB6Unassigned = 14;
	const uint8_t _cB7Unassigned = 15;

#ifndef _DebugApplication
	// Remote commands
	enum eFunctionCode : char
	{
		TNameFunction = 'F', // Code for function, if controlled remotely
		TNameMenu = 'K'		 // Code for menu, if controlled remotely
	};
#endif

	TextFrontPlate *_mText = nullptr;													 // Pointer to current text objekt of the class
	ModuleFactory *_mModuleFactory = nullptr;											 // Reference to factory of input modules
	Counter *_mCounter = nullptr;														 // Reference to counter
	LCDHandler *_mLCDHandler = nullptr;													 // Reference to LCD handler
	bool _mTriggerLampTestOff;															 // Stores information, if LCD must be switched off at next loop
	Counter::eFunctionCode _mSelectedCounterFunctionCode;								 // Code of the currently selected function
	ModuleBase::eModuleCode _mCurrentModuleCode = ModuleBase::eModuleCode::TNoSelection; // code of the currently activemodule - for checking in loop() if a new module was selected
	eMenuKeyCode _mSelectedeMenuKeyCode;												 // the last pressed menu button
	bool _mChangeFunctionDetected;														 // there is a new function detected
	bool _mChangeMenuDecected;															 // there is a new menu entry detected
	const int _cEepromIndexFunction = 1;												 // Selected function on front plate

protected:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	/// <param name="iLCDHandler">Reference of LCD handler</param>
	/// <param name="iModuleFactory">Reference of the module factory</param>
	/// <param name="iCounter">Reference of counter module</param>
	FrontPlate(sInitializeModule iInitializeModule, LCDHandler *iLCDHandler, ModuleFactory *iModuleFactory, Counter *iCounter);
	~FrontPlate();

public:
	static FrontPlate *GetInstance(sInitializeModule iInitializeModule, LCDHandler *iLCDHandler, ModuleFactory *iModuleFactory, Counter *iCounter);

	/////////////////////////////////////////////////////
	// Functions that can be called from within main loop
	/////////////////////////////////////////////////////

	/// <summary>
	/// Is called periodically from main loop
	/// </summary>
	void loop() override;

#ifndef _DebugApplication
	/// <summary>
	/// Dispatches commands got from en external input, e.g. a serial interface - only a dummy implementation here
	/// </summary>
	/// <param name="iModuleIdentifyer">If this matches with the identifyer of this module, then iParameter is analyzed:
	/// 'F' : Command for function selection</param>
	/// <param name="iParameter">Parameter or command that is to be analyzed:
	/// 'f' : Selects frequency measurement
	/// 'n' : Selects period measurement triggert by negative edge
	/// 'p' : Selects period measurement triggert by positive edge
	/// 'N' : Selects period measurement triggert by negative level
	/// 'P' : Selects period measurement triggert by positive level
	/// 'C' : Selects event counting
	/// '?' : Returns the currently selected function, 'f', 'n', 'p', 'N', 'P', 'C' or '-' (nothing is selected)
	/// </param>
	/// <returns>Reaction of dispatching</returns>
	String Dispatch(char iModuleIdentifyer, char iParameter) override;
#endif

private:
	/// <summary>
	/// Selects a specified function
	/// </summary>
	/// <param name="iFunctionCode">Function code to be selected</param>
	void _I2ESelectFunction(Counter::eFunctionCode iFunctionCode);

	/// <summary>
	/// Bundles all required functionality about function selection - called by _I2ESelectFunction
	/// </summary>
	/// <param name="iFunctionCode">Function code to be selected</param>
	void _I2ESelectSingleFunction(Counter::eFunctionCode iFunctionCode);

	/// <summary>
	/// Sets all LEDs to a specified state
	/// </summary>
	/// <param name="iTest">State: HIGH = switches on, LOW = switches off</param>
	void _I2ESwitchLEDs(uint8_t iTest);

public:
	///////////////////////////////////////////////
	// Functions that can be called from everywhere
	///////////////////////////////////////////////

	/// <summary>
	/// Gets the readable name of this class
	/// </summary>
	/// <returns>Name</returns>
	String GetName() override;

	/// <summary>
	/// Sets a trigger for switchin off LED in the next loop() call
	/// </summary>
	void TriggerLampTestOff();

	/// <summary>
	/// Scans keys for selecting the function
	/// </summary>
	/// <returns>true: a new function was selected, false: no new function was selected</returns>
	bool IsNewFunctionSelected();

	/// <summary>
	/// Scans menu keys
	/// </summary>
	/// <returns>true: a menu button was pressed, false: no menu button was pressed</returns>
	bool IsNewMenuSelected();
};

#endif