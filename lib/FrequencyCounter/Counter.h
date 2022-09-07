// Arduino Frequency Counter
// 21.10.2021
// Stefan Rau
// Central counter module

#pragma once
#ifndef _Counter_h
#define _Counter_h

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include "Debug.h"
#include "TextBase.h"
#include "I2CBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextCounter : public TextBase
{
public:
	TextCounter();
	~TextCounter();

	String GetObjectName() override;
	String InitError(String iICNumber);
	String ResultFrequency(float iNumber);
	String ResultPeriod(float iNumber);
	String ResultEventCount(float iNumber);
	String Overflow();
	String FunctionNameFrequency();
	String FunctionNameEdgeNegative();
	String FunctionNameEdgePositive();
	String FunctionNameNegative();
	String FunctionNamePositive();
	String FunctionNameNoSelection();
	String FunctionNameEventCounting();
	String FunctionNameUnknown();
};

/////////////////////////////////////////////////////////////

// Zählerlogik
class Counter : public I2CBase
{
public:
	enum eFunctionCode : char
	{
		TFrequency = 'f',
		TEdgeNegative = 'n',
		TEdgePositive = 'p',
		TNegative = 'N',
		TPositive = 'P',
		TEventCounting = 'C',
		TNoSelection = '-'
	};

	TextCounter *_mText = nullptr; // Pointer to current text objekt of the class

private:
	const uint8_t _cOSelectFunctionS0 = 12;
	const uint8_t _cOSelectFunctionS1 = 13;
	const uint8_t _cOSelectPeriod = 14;
	const uint8_t _cIOverflow = 15;

	// MCP23017 IC 4 - lower word input 0 .. 15
	Adafruit_MCP23X17 *_mI2LowerWord = nullptr;

	// MCP23017 IC 5 - upper word input 16 .. 26, reset counter, input selection
	Adafruit_MCP23X17 *_mI2UpperWord = nullptr;

	eFunctionCode _mFunctionCode; // Code of the currently selected function

protected:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	Counter(sInitializeModule iInitializeModule);
	
	~Counter();

public:
	static Counter *GetCounter(sInitializeModule iInitializeModule);

	// Functions that can be called from within main loop

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
	/// Gets the current value of the counter. The caller must be sure, that the counter is not counting in this moment.
	/// </summary>
	/// <returns>Value with unit.</returns>
	String I2EGetCounterValue();

	/// <summary>
	/// Sets counter to dedicated function
	/// </summary>
	/// <param name="iFunctionCode">Function code of frequency or period measurement</param>
	void I2ESetFunctionCode(eFunctionCode iFunctionCode);

	///////////////////////////////////////////////
	// Functions that can be called from everywhere
	///////////////////////////////////////////////

	/// <summary>
	/// Sets counter to dedicated function
	/// </summary>
	/// <returns="iFunctionCode">Function code of frequency or period measurement</returns>
	eFunctionCode GetFunctionCode();

	/// <summary>
	/// Returns the name of the currently selected function
	/// </summary>
	/// <returns>Readable name</returns>
	String GetSelectedFunctionName();

	/// <summary>
	/// Readable name of the module
	/// </summary>
	/// <returns>Gets the current name depending on current language</returns>
	String GetName() override;
};

#endif
