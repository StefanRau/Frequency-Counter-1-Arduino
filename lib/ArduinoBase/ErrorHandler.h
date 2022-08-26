// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Records and handles errors inclusive multiple errors

#pragma once
#ifndef _ErrorHandler_h
#define _ErrorHandler_h

#include <Arduino.h>
#include <time.h>
#include "Debug.h"
#include "List.h"
#include "I2CBase.h"

/// <summary>
/// Local text class of the module
/// </summary>
class TextErrorHandler : public TextBase
{
public:
	TextErrorHandler();
	~TextErrorHandler();

	String GetObjectName() override;
	String FunctionNameUnknown(char iModuleIdentifyer, char iParameter);
	String FormatDone();
	String FormatFailed();
	String ErrorListDone();
	String SeverityMessage();
	String SeverityWarning();
	String SeverityError();
	String SeverityFatal();
	String SeverityUnknown();
};

/////////////////////////////////////////////////////////////

/// <summary>
/// This instance contains all information about one error
/// </summary>
class Error
{

public:
	/// <summary>
	/// Error severity
	/// </summary>
	enum eSeverity : char
	{
		TMessage = 'M',
		TWarning = 'W',
		TError = 'E', // shall be raised if a hardware module can't be initialized - fallback shall be possible here
		TFatal = 'F'  // shall be raised if a required hardware module can't be initialized
	};

	struct sErrorHeader
	{
		eSeverity Severity; // severity of an error message
		int Count;			// error count
		tm Time;			// time of the error message
	};

	union uErrorHeader
	{
		sErrorHeader ErrorHeader;
		uint8_t Buffer[sizeof(sErrorHeader)];
	};

	struct sErrorEntry
	{
		union uErrorHeader ErrorHeader; // serializable header of an error message
		String ErrorMessage;			// the error message itself
	};

private:
	sErrorEntry _mErrorEntry; // Text of the current error message

public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iSeverity">Severtity of the new error message</param>
	/// <param name="iErrorMessage">Text of the new error message</param>
	Error(int iNumber, eSeverity iSeverity, String iErrorMessage);
	~Error();

	/// <summary>
	/// Gets the current error
	/// </summary>
	/// <returns>A complete error structure</returns>
	sErrorEntry GetErrorEntry();
};

#define ErrorPrint(iSeverity, iErrorMessage) ErrorHandler::GetErrorHandler()->Print(iSeverity, iErrorMessage)

#ifdef ARDUINO_AVR_NANO_EVERY
#define ErrorHandlerStartAddress 0x000 // nano uses internal EEPROM for settings
#endif
#ifdef ARDUINO_SAMD_NANO_33_IOT
#define ErrorHandlerStartAddress 0x0100 // 1st address for logging
#endif
#ifdef ARDUINO_ARDUINO_NANO33BLE
#define ErrorHandlerStartAddress 0x0100 // 1st address for logging
#endif

class ErrorHandler : public I2CBase
{
private:
	struct sErrorEEPROMHeader
	{
		char Checksum;
		uint16_t NextErrorWritePointer;
		short NumberOfErrors;
	};

	union uErrorEEPROMHeader
	{
		sErrorEEPROMHeader ErrorHeader;
		uint8_t Buffer[sizeof(sErrorEEPROMHeader)];
	};

	TextErrorHandler *_mText = nullptr;													// Pointer to current text objekt of the class
	ListCollection *_mErrorList = nullptr;												// List of errors
	int _mEEPROMMemoryIterator = sizeof(sErrorEEPROMHeader) + ErrorHandlerStartAddress; // pointer to address of next error log item, initially that's the bype after the last entry of header
	int _mEEPROMErrorIterator = 0;														// number of next error log item

#ifndef _DebugApplication
	// Commands for remote control
	enum eFunctionCode : char
	{
		TName = 'E',	  // Code for this class, if controlled remotely
		TFormat = 'F',	  // Formatting of EEPROM
		TReadNext = 'R',  // Read the next item of the error log and increase pointer to error log item
		TReadReset = '0', // Reset pointer to error log item
		TReadSize = 'S'	  // Get number of error entries
	};
#endif

	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iInitializeModule">Structure that contains EEPROM settings address (or starting address) as well as I2C address (or starting address) of the module</param>
	ErrorHandler(sInitializeModule iInitializeModule);

	~ErrorHandler();

public:
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

private:
	/// <summary>
	/// Checks the header of the logger EEPROM
	/// </summary>
	/// <returns>true: EEPROM is o.k., false: EEPROM is not o.k.</returns>
	bool _I2ECheckEEPROMHeader();

	/// <summary>
	/// Writes the header of the logger EEPROM
	/// </summary>
	/// <param name="iBuffer">Header to write</param>
	/// <returns>true: EEPROM is o.k., false: EEPROM is not o.k.</returns>
	bool _I2EWriteEEPROMHeader(union uErrorEEPROMHeader iBuffer);

	// Functions that can be called from everywhere

	/// <summary>
	/// Returns the check sum of the EEPROM header
	/// </summary>
	/// <returns>checksum value</returns>
	char _GetEEPROMHeaderChecksum(union uErrorEEPROMHeader iBuffer);

public:
	/// <summary>
	/// Gets a singleton. If InitializeErrorHandler was not called before, logging isn't be used.
	/// </summary>
	/// <returns>Instance of the error handler</returns>
	static ErrorHandler *GetErrorHandler();

	/// <summary>
	/// Write a new error message
	/// </summary>
	/// <param name="iSeverity">Severtity of the new error message</param>
	/// <param name="iErrorMessage">Text of the new error message</param>
	void Print(Error::eSeverity iSeverity, String iErrorMessage);

	/// <summary>
	/// Returns the 1st error of the list
	/// </summary>
	/// <returns>The error object</returns>
	Error *GetRootCause();

	/// <summary>
	/// Checks if the error list contains error or fatal
	/// </summary>
	/// <returns>true: the list of errors contains one, false: there was no error ot fatal</returns>
	bool ContainsErrors();

	/// <summary>
	/// Returns a list object with all errors
	/// </summary>
	/// <returns></returns>
	ListCollection *GetErrorList();

	/// <summary>
	/// Readable name of the module
	/// </summary>
	/// <returns>Gets the current name depending on current language</returns>
	String GetName() override;
};

#endif