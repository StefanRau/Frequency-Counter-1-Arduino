// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// Handles output on an LCD connected via I2C

#pragma once
#ifndef _LCDHandler_h
#define _LCDHandler_h

#include <wire.h>
#include <hd44780.h>					   // main hd44780 header; use this library because others have issues
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include "I2CBase.h"
#include "TextBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextLCDHandler : public TextBase
{
public:
	TextLCDHandler();
	~TextLCDHandler();

	String GetObjectName() override;
	String FrequencyCounter();
	String Selection();
	String Error();
	String InitError();
};

/////////////////////////////////////////////////////////////

/// <summary>
/// LCD class imkplementation
/// </summary>
class LCDHandler : public I2CBase
{
	// private:
public:
	enum class eStateCode : char
	{
		TDone = 'D',
		TInitialize = 'I',
		TInitializeDone = 'i',
		TShowMenu = 'M',
		TShowMenuDone = 'm',
		TShowCounter = 'C',
		TShowCounterDone = 'c',
		TShowError = 'E'
	};

	static LCDHandler *GetInstance(sInitializeModule iInitializeModule);

	/// <summary>
	/// Is called periodically from main loop
	/// </summary>
	void loop() override;

	/// <summary>
	/// Readable name of the module
	/// </summary>
	/// <returns>Gets the current name depending on current language</returns>
	String GetName() override;

#if DEBUG_APPLICATION == 0
	/// <summary>
	/// Dispatches commands got from en external input, e.g. a serial interface - only a dummy implementation here
	/// </summary>
	/// <param name="iModuleIdentifyer">If this matches with the identifyer of this module</param>
	/// <param name="iParameter">Parameter or command that is to be analyzed</param>
	/// <returns>Reaction of dispatching</returns>
	String DispatchSerial(char iModuleIdentifyer, char iParameter) override;
#endif

	/// <summary>
	///
	/// </summary>
	/// <param name="iText"></param>
	/// <param name="iCurrentMenuEntryNumber"></param>
	/// <param name="iLastMenuEntryNumber"></param>
	void TriggerMenuSelectedFunction(String iText, int iCurrentMenuEntryNumber, int iLastMenuEntryNumber);

	/// <summary>
	/// Shows the text of the selected menu item
	/// </summary>
	/// <param name="iText">Text to show</param>
	void SetSelectedFunction(String iText);

	/// <summary>
	/// Output of the measureent value
	/// </summary>
	/// <param name="iText">Value and unit to show</param>
	void SetMeasurementValue(String iText);

	/// <summary>
	/// Shows the error text
	/// </summary>
	/// <param name="iText">Text to show</param>
	void SetErrorText(String iText);

	/// <summary>
	/// Triggers refresh of the display
	/// </summary>
	void TriggerShowRefresh();

	/// <summary>
	/// Triggers output of measurement value
	/// </summary>
	void TriggerShowCounter();

protected:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	LCDHandler(sInitializeModule iInitializeModule);
	~LCDHandler();

private:
	hd44780_I2Cexp *mI2ELCD = nullptr; // LDC driver
	TextLCDHandler *mText = nullptr;   // Pointer to current text objekt of the class
	String mMenuSelectedFunction;	   // In case of manu output: name of the menu
	int mCurrentMenuEntryNumber;	   // In case of manu output: current index of the menu entry
	int mLastMenuEntryNumber;		   // In case of manu output: number of menu entries for the given module
	String mInputSelectedFunction;	   // Name of the function to show
	String mInputCurrentValue;		   // Measurement value to show
	String mInputError;				   // Error messge to show
	bool mIsCritical = false;

	/// <summary>
	/// This function limits the maximum size of a text to 16
	/// </summary>
	/// <param name="iText">The text that shall be trimmed</param>
	/// <returns>Result: Text with 16 characters</returns>
	String TrimLine(String iText);

	/// <summary>
	/// Writes up / down arrows for menue selection
	/// </summary>
	void I2EWriteMenuNavigator();
};

#endif