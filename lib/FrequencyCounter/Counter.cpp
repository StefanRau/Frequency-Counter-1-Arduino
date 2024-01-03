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
// 06.09.2022: Singleton instantiation - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "ErrorHandler.h"
#include "Counter.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextCounter::TextCounter() : TextBase()
{
	DEBUG_INSTANTIATION("TextCounter");
}

TextCounter::~TextCounter()
{
	DEBUG_DESTROY("TextCounter");
}

String TextCounter::GetObjectName()
{
	DEBUG_METHOD_CALL("TextCounter::GetObjectName");

	switch (GetLanguage())
	{
		TextLangE("Counter");
		TextLangD("Zaehler");
	}
}

String TextCounter::InitError(String iICName)
{
	DEBUG_METHOD_CALL("TextCounter::InitError");

	switch (GetLanguage())
	{
		TextLangE("Counter IC" + iICName + " defect");
		TextLangD("Zaehler IC" + iICName + " defekt");
	}
}

String TextCounter::ResultFrequency(float iNumber)
{
	DEBUG_METHOD_CALL("TextCounter::ResultFrequency");

	return String(iNumber, 0) + " Hz";
}

String TextCounter::ResultPeriod(float iNumber)
{
	DEBUG_METHOD_CALL("TextCounter::ResultPeriod");

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
	DEBUG_METHOD_CALL("TextCounter::ResultEventCount");

	return String(iNumber, 0);
}

String TextCounter::Overflow()
{
	DEBUG_METHOD_CALL("TextCounter::Overflow");

	switch (GetLanguage())
	{
		TextLangE("Overflow");
		TextLangD("Ueberlauf");
	}
}

String TextCounter::FunctionNameFrequency()
{
	DEBUG_METHOD_CALL("TextCounter::FunctionNameFrequency");

	switch (GetLanguage())
	{
		TextLangE("Frequency");
		TextLangD("Frequenz");
	}
}

String TextCounter::FunctionNameEdgeNegative()
{
	DEBUG_METHOD_CALL("TextCounter::FunctionNameEdgeNegative");

	switch (GetLanguage())
	{
		TextLangE("Dur. neg. edge");
		TextLangD("Dauer neg.Flanke");
	}
}

String TextCounter::FunctionNameEdgePositive()
{
	DEBUG_METHOD_CALL("TextCounter::FunctionNameEdgePositive");

	switch (GetLanguage())
	{
		TextLangE("Dur. pos. edge");
		TextLangD("Dauer pos.Flanke");
	}
}

String TextCounter::FunctionNameNegative()
{
	DEBUG_METHOD_CALL("TextCounter::FunctionNameNegative");

	switch (GetLanguage())
	{
		TextLangE("Length neg.level");
		TextLangD("Dauer neg. Pegel");
	}
}

String TextCounter::FunctionNamePositive()
{
	DEBUG_METHOD_CALL("TextCounter::FunctionNamePositive");

	switch (GetLanguage())
	{
		TextLangE("Length pos.level");
		TextLangD("Dauer pos. Pegel");
	}
}

String TextCounter::FunctionNameEventCounting()
{
	DEBUG_METHOD_CALL("TextCounter::FunctionNameEventCounting");

	switch (GetLanguage())
	{
		TextLangE("Event counting");
		TextLangD("Ereigniszaehlung");
	}
}

String TextCounter::FunctionNameNoSelection()
{
	DEBUG_METHOD_CALL("");

	switch (GetLanguage())
	{
		TextLangE("Initial");
		TextLangD("Initial");
	}
}

String TextCounter::FunctionNameUnknown()
{
	DEBUG_METHOD_CALL("TextCounter::FunctionNameUnknown");

	switch (GetLanguage())
	{
		TextLangE("Unknown function");
		TextLangD("Unbekannte Funktion");
	}
}

/////////////////////////////////////////////////////////////

static Counter *gInstance = nullptr;

Counter::Counter(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
	DEBUG_INSTANTIATION("Counter: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	short lI2CAddress = mI2CAddress;
	// Initialize hardware

	_mText = new TextCounter();
	//	_mText = &gTextCounter;

	if (lI2CAddress < 0)
	{
		DEBUG_PRINT_LN("Counter I2C address is not defined");
		return;
	}

	// I2C address of lower word
	_mI2LowerWord = new Adafruit_MCP23X17();
	if (!_mI2LowerWord->begin_I2C(lI2CAddress, &Wire))
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitError("lower word"));
		DEBUG_PRINT_LN("Counter IC for lower word can't be initialized");
		return;
	}
	DEBUG_PRINT_LN("Counter IC for lower word is initialized at address: " + String(lI2CAddress));
	//_mI2LowerWord->enableAddrPins();
	lI2CAddress += 1;

	// I2C address of upper word
	_mI2UpperWord = new Adafruit_MCP23X17();
	if (!_mI2UpperWord->begin_I2C(lI2CAddress, &Wire))
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitError("upper word"));
		DEBUG_PRINT_LN("Counter IC for upper word can't be initialized");
		return;
	}
	DEBUG_PRINT_LN("Counter IC for upper word is initialized at address: " + String(lI2CAddress));
	//_mI2UpperWord->enableAddrPins();

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
	DEBUG_DESTROY("Counter");
}

Counter *Counter::GetInstance(sInitializeModule iInitializeModule)
{
	DEBUG_METHOD_CALL("Counter::GetInstance");

	gInstance = (gInstance == nullptr) ? new Counter(iInitializeModule) : gInstance;
	return gInstance;
}

void Counter::loop()
{
	DEBUG_METHOD_CALL("Counter::loop");
}

#if DEBUG_APPLICATION == 0
String Counter::DispatchSerial(char iModuleIdentifyer, char iParameter)
{
	return String("");
}
#endif

String Counter::I2EGetCounterValue()
{
	DEBUG_METHOD_CALL("Counter::I2EGetCounterValue");

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
	if (_mI2UpperWord->digitalRead(_cIOverflow) == HIGH)
	{
		lResultString = _mText->Overflow();
	}

	// DEBUG_PRINT_LN("Counter value: " + lResultString);
	return lResultString;
}

void Counter::I2ESetFunctionCode(eFunctionCode iFunctionCode)
{
	DEBUG_METHOD_CALL("Counter::I2ESetFunctionCode");

	if (!mModuleIsInitialized)
	{
		return;
	}

	_mFunctionCode = iFunctionCode;

	switch (_mFunctionCode)
	{
	case eFunctionCode::TFrequency:
	case eFunctionCode::TEventCounting:
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

	default:
		break;
	}
}

Counter::eFunctionCode Counter::GetFunctionCode()
{
	DEBUG_METHOD_CALL("Counter::GetFunctionCode");

	return _mFunctionCode;
}

String Counter::GetSelectedFunctionName()
{
	DEBUG_METHOD_CALL("Counter::GetSelectedFunctionName");

	switch (_mFunctionCode)
	{
	case Counter::eFunctionCode::TFrequency:
		return _mText->FunctionNameFrequency();
	case Counter::eFunctionCode::TPositive:
		return _mText->FunctionNamePositive();
	case Counter::eFunctionCode::TNegative:
		return _mText->FunctionNameNegative();
	case Counter::eFunctionCode::TEdgePositive:
		return _mText->FunctionNameEdgePositive();
	case Counter::eFunctionCode::TEdgeNegative:
		return _mText->FunctionNameEdgeNegative();
	case Counter::eFunctionCode::TEventCounting:
		return _mText->FunctionNameEventCounting();
	default:
		break;
	}
	return _mText->FunctionNameUnknown();
}

String Counter::GetName()
{
	DEBUG_METHOD_CALL("Counter::GetName");

	return _mText->GetObjectName();
}
