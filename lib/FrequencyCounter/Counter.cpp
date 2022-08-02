// Arduino Frequency Counter
// 21.10.2021
// Stefan Rau
// History
// 21.10.2021: 1st version - Stefan Rau
// 21.10.2021: enumeration eFunctionCode is of type char - Stefan Rau
// 21.10.2021: Dispatcher has now chars as input - Stefan Rau
// 27.10.2021: Constructor requires structure - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 13.06.2022: Use constants for bits 12 - 15 of 2nd input expander and other renamings - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau

#include "ErrorHandler.h"
#include "Counter.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextCounter::TextCounter() : TextBase(-1)
{
	DebugInstantiation("New TextCounter");
}

TextCounter::~TextCounter()
{
}

String TextCounter::GetObjectName()
{
	switch (GetLanguage())
	{
		TextLangE("Counter");
		TextLangD("Zaehler");
	}
}

String TextCounter::InitError(String iICName)
{
	switch (GetLanguage())
	{
		TextLangE("Counter IC" + iICName + " defect");
		TextLangD("Zaehler IC" + iICName + " defekt");
	}
}

String TextCounter::ResultFrequency(float iNumber)
{
	return String(iNumber, 0) + " Hz";
}

String TextCounter::ResultPeriod(float iNumber)
{
	if (iNumber >= 1)
	{
		return String(iNumber, 7) + " s";
	}
	else if (iNumber < 0.001)
	{
		return String(iNumber * 1000000, 1) + " us";
	}
	else if (iNumber < 1)
	{
		return String(iNumber * 1000, 4) + " ms";
	}

	return "";
}

String TextCounter::ResultEventCount(float iNumber)
{
	return String(iNumber, 0);
}

String TextCounter::Overflow()
{
	switch (GetLanguage())
	{
		TextLangE("Overflow");
		TextLangD("Ueberlauf");
	}
}

/////////////////////////////////////////////////////////////

Counter::Counter(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
	DebugInstantiation("New Counter: iInitializeModule[SettingsAddress, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.I2CAddress) + "]");

	short lI2CAddress = mI2CAddress;
	// Initialize hardware

	_mText = new TextCounter();

	if (lI2CAddress < 0)
	{
		DebugPrint("Counter I2C address is not defined");
		return;
	}

	// I2C address of lower word
	_mI2LowerWord = new Adafruit_MCP23X17();
	if (!_mI2LowerWord->begin_I2C(lI2CAddress, &Wire))
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitError("lower word"));
		DebugPrint("Counter IC for lower word can't be initialized");
		return;
	}
	DebugPrint("Counter IC for lower word is initialized at address: " + String(lI2CAddress));
	_mI2LowerWord->enableAddrPins();
	lI2CAddress += 1;

	// I2C address of upper word
	_mI2UpperWord = new Adafruit_MCP23X17();
	if (!_mI2UpperWord->begin_I2C(lI2CAddress, &Wire))
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitError("upper word"));
		DebugPrint("Counter IC for upper word can't be initialized");
		return;
	}
	DebugPrint("Counter IC for upper word is initialized at address: " + String(lI2CAddress));
	_mI2UpperWord->enableAddrPins();

	_mI2LowerWord->pinMode(0, INPUT);
	_mI2LowerWord->pinMode(1, INPUT);
	_mI2LowerWord->pinMode(2, INPUT);
	_mI2LowerWord->pinMode(3, INPUT);
	_mI2LowerWord->pinMode(4, INPUT);
	_mI2LowerWord->pinMode(5, INPUT);
	_mI2LowerWord->pinMode(6, INPUT);
	_mI2LowerWord->pinMode(7, INPUT);

	_mI2LowerWord->pinMode(8, INPUT);
	_mI2LowerWord->pinMode(9, INPUT);
	_mI2LowerWord->pinMode(10, INPUT);
	_mI2LowerWord->pinMode(11, INPUT);
	_mI2LowerWord->pinMode(12, INPUT);
	_mI2LowerWord->pinMode(13, INPUT);
	_mI2LowerWord->pinMode(14, INPUT);
	_mI2LowerWord->pinMode(15, INPUT);

	_mI2UpperWord->pinMode(0, INPUT);
	_mI2UpperWord->pinMode(1, INPUT);
	_mI2UpperWord->pinMode(2, INPUT);
	_mI2UpperWord->pinMode(3, INPUT);
	_mI2UpperWord->pinMode(4, INPUT);
	_mI2UpperWord->pinMode(5, INPUT);
	_mI2UpperWord->pinMode(6, INPUT);
	_mI2UpperWord->pinMode(7, INPUT);

	_mI2UpperWord->pinMode(8, INPUT);
	_mI2UpperWord->pinMode(9, INPUT);
	_mI2UpperWord->pinMode(10, INPUT);
	_mI2UpperWord->pinMode(11, INPUT);
	_mI2UpperWord->pinMode(_cOSelectFunctionS0, OUTPUT);
	_mI2UpperWord->pinMode(_cOSelectFunctionS1, OUTPUT);
	_mI2UpperWord->pinMode(_cOSelectPeriod, OUTPUT);
	_mI2UpperWord->pinMode(_cIOverflow, INPUT);

	_mFunctionCode = eFunctionCode::TFrequency;
	mModuleIsInitialized = true;
}

Counter::~Counter()
{
}

void Counter::loop()
{
}

#ifndef _DebugApplication
String Counter::Dispatch(char iModuleIdentifyer, char iParameter)
{
	return String("");
}
#endif

String Counter::I2EGetCounterValue()
{
	uint16_t lLowerWord;
	uint16_t lUpperWord;
	uint32_t lResultInt32;
	float lResultFloat;
	String lResultString = "";

	if (!mModuleIsInitialized)
	{
		return lResultString;
	}

	// Read 28 bit
	lLowerWord = _mI2LowerWord->readGPIOAB();
	lUpperWord = _mI2UpperWord->readGPIOAB();
	lUpperWord = lUpperWord & 0x0fff;
	lResultInt32 = (uint32_t)lUpperWord;
	lResultInt32 = lResultInt32 << 16;
	lResultInt32 = lResultInt32 | lLowerWord;

	switch (_mFunctionCode)
	{
	case eFunctionCode::TFrequency:
		// lResultFloat = (float)lResultInt32;
		lResultFloat = (float)lResultInt32 / 1.0000002;
		lResultString = _mText->ResultFrequency(lResultFloat);
		break;

	case eFunctionCode::TNegative:
		lResultFloat = (float)lResultInt32 / 10000000;
		lResultString = _mText->ResultPeriod(lResultFloat);
		break;

	case eFunctionCode::TPositive:
		lResultFloat = (float)lResultInt32 / 10000000;
		lResultString = _mText->ResultPeriod(lResultFloat);
		break;

	case eFunctionCode::TEdgeNegative:
		lResultFloat = (float)lResultInt32 / 10000000;
		lResultString = _mText->ResultPeriod(lResultFloat);
		break;

	case eFunctionCode::TEdgePositive:
		lResultFloat = (float)lResultInt32 / 10000000;
		lResultString = _mText->ResultPeriod(lResultFloat);
		break;

	case eFunctionCode::TEventCounting:
		lResultFloat = (float)lResultInt32;
		lResultString = _mText->ResultEventCount(lResultFloat);
		break;

	default:
		break;
	}

	// Check for overflow
	if (_mI2UpperWord->digitalRead(_cIOverflow))
	{
		lResultString = _mText->Overflow();
	}

	// DebugPrint("Counter value: " + lResultString);
	return lResultString;
}

void Counter::I2ESetFunctionCode(char iFunctionCode)
{

	DebugPrint("Counter::I2ESetFunctionCode:" + String(iFunctionCode));

	if (!mModuleIsInitialized)
	{
		return;
	}

	_mFunctionCode = iFunctionCode;

	switch (iFunctionCode)
	{
	case eFunctionCode::TFrequency:
		_mI2UpperWord->digitalWrite(_cOSelectPeriod, LOW);
		break;

	case eFunctionCode::TPositive:
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS0, LOW);
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS1, LOW);
		_mI2UpperWord->digitalWrite(_cOSelectPeriod, HIGH);
		break;

	case eFunctionCode::TNegative:
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS0, HIGH);
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS1, LOW);
		_mI2UpperWord->digitalWrite(_cOSelectPeriod, HIGH);
		break;

	case eFunctionCode::TEdgePositive:
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS0, LOW);
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS1, HIGH);
		_mI2UpperWord->digitalWrite(_cOSelectPeriod, HIGH);
		break;

	case eFunctionCode::TEdgeNegative:
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS0, HIGH);
		_mI2UpperWord->digitalWrite(_cOSelectFunctionS1, HIGH);
		_mI2UpperWord->digitalWrite(_cOSelectPeriod, HIGH);
		break;

	case eFunctionCode::TEventCounting:
		_mI2UpperWord->digitalWrite(_cOSelectPeriod, LOW);
		break;

	default:
		break;
	}
}

String Counter::GetName()
{
	return _mText->GetObjectName();
}
