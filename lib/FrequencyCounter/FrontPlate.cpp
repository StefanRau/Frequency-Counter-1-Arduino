// Arduino Frequency Counter
// 21.10.2021
// Stefan Rau
// History
// 21.10.2021: 1st version - Stefan Rau
// 21.10.2021: Dispatcher has now chars as input - Stefan Rau
// 22.10.2021: Function and menu selection in loop()
// 27.10.2021: Constructor requires structure - Stefan Rau
// 04.11.2021: Verbose mode implemented - Stefan Rau
// 07.11.2021: Get all menu entry items - Stefan Rau
// 12.03.2022: Menu selection by remote control is now considered - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 23.04.2022: Delay in front plate key detection - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 06.09.2022: Singleton instantiation - Stefan Rau
// 21.09.2022: use GetInstance instead of Get<Typename> - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 20.01.2023: Improve debug handling - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "FrontPlate.h"
#include "ErrorHandler.h"

// Text definitions

// extern TextFrontPlate gTextFrontPlate;
// extern TextFrontPlate gTextFrontPlate();

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextFrontPlate::TextFrontPlate() : TextBase()
{
	DEBUG_INSTANTIATION("TextI2CBase");
}

TextFrontPlate::~TextFrontPlate()
{
	DEBUG_DESTROY("TextFrontPlate");
}

String TextFrontPlate::GetObjectName()
{
	DEBUG_METHOD_CALL("TextFrontPlate::GetObjectName");

	switch (GetLanguage())
	{
		TextLangE("Front plate");
		TextLangD("Frontplatte");
	}
}

String TextFrontPlate::InitError()
{
	DEBUG_METHOD_CALL("TextFrontPlate::InitError");

	switch (GetLanguage())
	{
		TextLangE("Keyb/LEDs defect");
		TextLangD("Keyb/LEDs defekt");
	}
}

String TextFrontPlate::InitErrorCounterRequired()
{
	DEBUG_METHOD_CALL("TextFrontPlate::InitErrorCounterRequired");

	switch (GetLanguage())
	{
		TextLangE("Counter is required");
		TextLangD("Zähler wird benötigt");
	}
}

String TextFrontPlate::InitErrorModuleFactoryRequired()
{
	DEBUG_METHOD_CALL("TextFrontPlate::InitErrorModuleFactoryRequired");

	switch (GetLanguage())
	{
		TextLangE("Module factory required");
		TextLangD("Modul Factory benötigt");
	}
}

String TextFrontPlate::InitErrorLCDRequired()
{
	DEBUG_METHOD_CALL("TextFrontPlate::InitErrorLCDRequired");

	switch (GetLanguage())
	{
		TextLangE("LCD is required");
		TextLangD("LCD wird benötigt");
	}
}

String TextFrontPlate::ErrorPlausibilityViolation()
{
	DEBUG_METHOD_CALL("TextFrontPlate::ErrorPlausibilityViolation");

	switch (GetLanguage())
	{
		TextLangE("Frontplate plausibility violation");
		TextLangD("Frontplatte Plausibilitaetsverletzung");
	}
}

/////////////////////////////////////////////////////////////

static FrontPlate *gInstance = nullptr;

FrontPlate::FrontPlate(sInitializeModule iInitializeModule, LCDHandler *iLCDHandler, ModuleFactory *iModuleFactory, Counter *iCounter) : I2CBase(iInitializeModule)
{
	DEBUG_INSTANTIATION("FrontPlate: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	// Initialize hardware
	_mText = new TextFrontPlate();
	//_mText = &gTextFrontPlate;

	if (iLCDHandler == nullptr)
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitErrorLCDRequired());
		DEBUG_PRINT_LN("Front Plate: LCD is required");
		return;
	}
	_mLCDHandler = iLCDHandler;

	if (iCounter == nullptr)
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitErrorCounterRequired());
		DEBUG_PRINT_LN("Front Plate: Counter is required");
		return;
	}
	_mCounter = iCounter;

	if (iModuleFactory == nullptr)
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitErrorModuleFactoryRequired());
		DEBUG_PRINT_LN("Front Plate: ModuleFactory is required");
		return;
	}
	_mModuleFactory = iModuleFactory;

	if (mI2CAddress < 0)
	{
		DEBUG_PRINT_LN("Front Plate I2C address not defined");
		return;
	}

	_mI2EModule = new Adafruit_MCP23X17();
	if (!_mI2EModule->begin_I2C(mI2CAddress, &Wire))
	{
		ErrorPrint(Error::eSeverity::TWarning, _mText->InitError());
		DEBUG_PRINT_LN("Front Plate can't be initialized");
		return;
	}
	DEBUG_PRINT_LN("Front Plate is initialized at address: " + String(mI2CAddress));
	//	_mI2EModule->enableAddrPins();

	// Key frequency measurement
	_mI2EModule->pinMode(_cIKeySelectFrequency, INPUT);
	_mI2EModule->pinMode(_cOLEDSelectFrequency, OUTPUT);

	// Key duration of positive level
	_mI2EModule->pinMode(_cIKeySelectTPositive, INPUT);
	_mI2EModule->pinMode(_cOLEDSelectTPositive, OUTPUT);

	// Key duration of negative level
	_mI2EModule->pinMode(_cIKeySelectTNegative, INPUT);
	_mI2EModule->pinMode(_cOLEDSelectTNegative, OUTPUT);

	// Key trigger by positive edge
	_mI2EModule->pinMode(_cIKeySelectTEdgePositive, INPUT);
	_mI2EModule->pinMode(_cOLEDSelectTEdgePositive, OUTPUT);

	// Key trigger by negative edge
	_mI2EModule->pinMode(_cIKeySelectTEdgeNegative, INPUT);
	_mI2EModule->pinMode(_cOLEDSelectTEdgeNegative, OUTPUT);

	// Menu keys
	_mI2EModule->pinMode(_cIKeySelectMenuUp, INPUT);
	_mI2EModule->pinMode(_cIKeySelectMenuDown, INPUT);

	// unassigned pins => set as input with pull ups
	_mI2EModule->pinMode(_cA2Unassigned, INPUT_PULLUP);
	_mI2EModule->pinMode(_cB5Unassigned, INPUT_PULLUP);
	_mI2EModule->pinMode(_cB6Unassigned, INPUT_PULLUP);
	_mI2EModule->pinMode(_cB7Unassigned, INPUT_PULLUP);

	_mSelectedCounterFunctionCode = Counter::eFunctionCode::TNoSelection;

	// swith on all LEDs for lamp test
	_I2ESwitchLEDs(HIGH);

	mModuleIsInitialized = true;
}

FrontPlate::~FrontPlate()
{
	DEBUG_DESTROY("FrontPlate");
}

FrontPlate *FrontPlate::GetInstance(sInitializeModule iInitializeModule, LCDHandler *iLCDHandler, ModuleFactory *iModuleFactory, Counter *iCounter)
{
	DEBUG_METHOD_CALL("FrontPlate::GetInstance");

	gInstance = (gInstance == nullptr) ? new FrontPlate(iInitializeModule, iLCDHandler, iModuleFactory, iCounter) : gInstance;
	return gInstance;
}

void FrontPlate::loop()
{
	DEBUG_METHOD_CALL("FrontPlate::loop");

	bool lFunctionKeyFrequencyPressed;
	bool lFunctionKeyPositivePressed;
	bool lFunctionKeyNegativePressed;
	bool lFunctionKeyEdgePositivePressed;
	bool lFunctionKeyEdgeNegativePressed;
	bool lIsPeriodMeasurementPossible;
	bool lIsEventCountingPossible;
	bool lMenuUpKeyPressed;
	bool lMenuDownKeyPressed;
	char lSetting;

	if (!mModuleIsInitialized)
	{
		return;
	}

	// switch off LEDs and show the current function on the LCD
	if (_mTriggerLampTestOff)
	{
		lSetting = GetSetting(_cEepromIndexFunction); // Read setting from processor internal EEPROM
		if (lSetting == cNullSetting)				  // Defaulting, if EEPROM does not exist
		{
			lSetting = Counter::eFunctionCode::TFrequency;
		}
		_I2ESelectFunction((Counter::eFunctionCode)lSetting);
		_mLCDHandler->SetSelectedFunction(_mCounter->GetSelectedFunctionName()); // Output at LCD
		_mCurrentModuleCode = _mModuleFactory->GetSelectedModule()->GetModuleCode();
		_mTriggerLampTestOff = false;
		return;
	}

	// if module has changed: set frequency measurement on, because that is sopported by all modules
	// e.g. HF module does not support periode measurement
	if (_mCurrentModuleCode != _mModuleFactory->GetSelectedModule()->GetModuleCode())
	{
		DEBUG_PRINT_LN("Old Function code:" + String(_mCurrentModuleCode));
		_I2ESelectFunction(Counter::eFunctionCode::TFrequency);
		_mCurrentModuleCode = _mModuleFactory->GetSelectedModule()->GetModuleCode();
		DEBUG_PRINT_LN("New Function code:" + String(_mCurrentModuleCode));
		return;
	}

	// Read keys
	if (!_mChangeFunctionDetected)
	{
		lFunctionKeyFrequencyPressed = (_mI2EModule->digitalRead(_cIKeySelectFrequency) == HIGH);
		lFunctionKeyPositivePressed = (_mI2EModule->digitalRead(_cIKeySelectTPositive) == HIGH);
		lFunctionKeyNegativePressed = (_mI2EModule->digitalRead(_cIKeySelectTNegative) == HIGH);
		lFunctionKeyEdgePositivePressed = (_mI2EModule->digitalRead(_cIKeySelectTEdgePositive) == HIGH);
		lFunctionKeyEdgeNegativePressed = (_mI2EModule->digitalRead(_cIKeySelectTEdgeNegative) == HIGH);

		if (lFunctionKeyFrequencyPressed && lFunctionKeyPositivePressed && lFunctionKeyNegativePressed && lFunctionKeyEdgePositivePressed && lFunctionKeyEdgeNegativePressed)
		{
			// Crash of MCP23X17 detected
			// DEBUG_PRINT_LN("\nCrash");
			return;
		}

		lIsPeriodMeasurementPossible = _mModuleFactory->GetSelectedModule()->IsPeriodMeasurementPossible();
		lIsEventCountingPossible = _mModuleFactory->GetSelectedModule()->IsEventCountingPossible();

		if (lFunctionKeyFrequencyPressed)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TFrequency)
			{
				DEBUG_PRINT_LN("\nFrequency selected");
				_I2ESelectFunction(Counter::eFunctionCode::TFrequency);
				delay(100);
			}
		}

		if (lFunctionKeyPositivePressed && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TPositive)
			{
				DEBUG_PRINT_LN("\nLevel positive selected");
				_I2ESelectFunction(Counter::eFunctionCode::TPositive);
				delay(100);
			}
		}

		if (lFunctionKeyNegativePressed && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TNegative)
			{
				DEBUG_PRINT_LN("\nLevel negative selected");
				_I2ESelectFunction(Counter::eFunctionCode::TNegative);
				delay(100);
			}
		}

		if (lFunctionKeyEdgePositivePressed && (!lFunctionKeyEdgeNegativePressed) && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TEdgePositive)
			{
				DEBUG_PRINT_LN("\nEdge negative selected");
				_I2ESelectFunction(Counter::eFunctionCode::TEdgePositive);
				delay(100);
			}
		}

		if ((!lFunctionKeyEdgePositivePressed) && lFunctionKeyEdgeNegativePressed && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TEdgeNegative)
			{
				DEBUG_PRINT_LN("\nEdge positive selected");
				_I2ESelectFunction(Counter::eFunctionCode::TEdgeNegative);
				delay(100);
			}
		}

		if (lFunctionKeyEdgePositivePressed && lFunctionKeyEdgeNegativePressed && lIsEventCountingPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TEventCounting)
			{
				DEBUG_PRINT_LN("\nEvent counting selected");
				_I2ESelectFunction(Counter::eFunctionCode::TEventCounting);
				delay(500);
			}
		}

		// // No function key is pressed
		// if ((!lFunctionKeyFrequencyPressed) && (!lFunctionKeyPositivePressed) && (!lFunctionKeyNegativePressed) && (!lFunctionKeyEdgePositivePressed) && (!lFunctionKeyEdgeNegativePressed))
		// {
		// 	_mSelectedCounterFunctionCode = Counter::eFunctionCode::TNoSelection;
		// }
	}

	// Menu key processing
	if (!_mChangeMenuDecected)
	{
		lMenuUpKeyPressed = (_mI2EModule->digitalRead(_cIKeySelectMenuUp) == HIGH);
		lMenuDownKeyPressed = (_mI2EModule->digitalRead(_cIKeySelectMenuDown) == HIGH);

		// Menu up is pressed?
		if (lMenuUpKeyPressed)
		{
			if (_mSelectedeMenuKeyCode != eMenuKeyCode::TMenuKeyUp)
			{
				DEBUG_PRINT_LN("\nMenu up");
				_mModuleFactory->GetSelectedModule()->I2EScrollFunctionUp();
				_mSelectedeMenuKeyCode = eMenuKeyCode::TMenuKeyUp;
				_mChangeMenuDecected = true;
				delay(100);
			}
		}

		// Menu down is pressed?
		if (lMenuDownKeyPressed)
		{
			if (_mSelectedeMenuKeyCode != eMenuKeyCode::TMenuKeyDown)
			{
				DEBUG_PRINT_LN("\nMenu down");
				_mModuleFactory->GetSelectedModule()->I2EScrollFunctionDown();
				_mSelectedeMenuKeyCode = eMenuKeyCode::TMenuKeyDown;
				_mChangeMenuDecected = true;
				delay(100);
			}
		}

		// No menu key is pressed
		if ((!lMenuUpKeyPressed) && (!lMenuDownKeyPressed))
		{
			_mSelectedeMenuKeyCode = eMenuKeyCode::TMenuKeyNo;
		}
	}
}

#if DEBUG_APPLICATION == 0
String FrontPlate::DispatchSerial(char iModuleIdentifyer, char iParameter)
{
	String lReturn = "";

	switch (iModuleIdentifyer)
	{
	case eFunctionCode::TNameFunction:

		switch (iParameter)
		{

		case Counter::eFunctionCode::TFrequency:
			_I2ESelectFunction((Counter::eFunctionCode)iParameter);
			return String(iParameter);

		case Counter::eFunctionCode::TPositive:
		case Counter::eFunctionCode::TNegative:
		case Counter::eFunctionCode::TEdgePositive:
		case Counter::eFunctionCode::TEdgeNegative:
		case Counter::eFunctionCode::TEventCounting:
			if (_mModuleFactory->GetSelectedModule()->IsPeriodMeasurementPossible())
			{
				_I2ESelectFunction((Counter::eFunctionCode)iParameter);
			}
			return String(iParameter);

		case ProjectBase::eFunctionCode::TParameterGetAll:
			lReturn += ProjectBase::GetVerboseMode() ? _mCounter->_mText->FunctionNameFrequency() : String((char)Counter::eFunctionCode::TFrequency);

			if (_mModuleFactory->GetSelectedModule()->IsPeriodMeasurementPossible())
			{
				lReturn += ProjectBase::GetVerboseMode() ? ',' + _mCounter->_mText->FunctionNamePositive() +
															   ',' + _mCounter->_mText->FunctionNameNegative() +
															   ',' + _mCounter->_mText->FunctionNameEdgePositive() +
															   ',' + _mCounter->_mText->FunctionNameEdgeNegative() +
															   ',' + _mCounter->_mText->FunctionNameEventCounting()
														 : String((char)Counter::eFunctionCode::TPositive) +
															   String((char)Counter::eFunctionCode::TNegative) +
															   String((char)Counter::eFunctionCode::TEdgePositive) +
															   String((char)Counter::eFunctionCode::TEdgeNegative) +
															   String((char)Counter::eFunctionCode::TEventCounting);
			}

			return lReturn;

		case ProjectBase::eFunctionCode::TParameterGetCurrent:
			switch (_mSelectedCounterFunctionCode)
			{
			case Counter::eFunctionCode::TFrequency:
			case Counter::eFunctionCode::TPositive:
			case Counter::eFunctionCode::TNegative:
			case Counter::eFunctionCode::TEdgePositive:
			case Counter::eFunctionCode::TEdgeNegative:
			case Counter::eFunctionCode::TEventCounting:
				if (ProjectBase::GetVerboseMode())
				{
					return _mCounter->GetSelectedFunctionName();
				}
				else
				{
					return String((char)_mSelectedCounterFunctionCode);
				}
			default:
				break;
			}
			return String((char)Counter::eFunctionCode::TNoSelection);
		}

		return _mCounter->_mText->FunctionNameUnknown();

	case eFunctionCode::TNameMenu:

		if ((iParameter >= '0') && (iParameter <= '9'))
		{
			_mModuleFactory->GetSelectedModule()->I2ESetCurrentMenuEntryNumber(atoi(&iParameter));
			_mChangeMenuDecected = true;
			return String(iParameter);
		}

		else if (iParameter == ProjectBase::eFunctionCode::TParameterGetAll)
		{
			return _mModuleFactory->GetSelectedModule()->GetAllMenuEntryItems();
		}

		else if (iParameter == ProjectBase::eFunctionCode::TParameterGetCurrent)
		{
			if (ProjectBase::GetVerboseMode())
			{
				return _mModuleFactory->GetSelectedModule()->GetCurrentMenuEntry(-1);
			}
			else
			{
				return String(_mModuleFactory->GetSelectedModule()->GetCurrentMenuEntryNumber());
			}
		}

		return _mCounter->_mText->FunctionNameUnknown();
	}

	return String("");
}
#endif

void FrontPlate::_I2ESelectFunction(Counter::eFunctionCode iFunctionCode)
{
	DEBUG_METHOD_CALL("FrontPlate::_I2ESelectFunction");

	_I2ESwitchLEDs(LOW);

	switch (iFunctionCode)
	{
	case Counter::eFunctionCode::TFrequency:
		_I2ESelectSingleFunction(Counter::eFunctionCode::TFrequency);
		_mI2EModule->digitalWrite(_cOLEDSelectFrequency, HIGH);
		break;
	case Counter::eFunctionCode::TPositive:
		_I2ESelectSingleFunction(Counter::eFunctionCode::TPositive);
		_mI2EModule->digitalWrite(_cOLEDSelectTPositive, HIGH);
		break;
	case Counter::eFunctionCode::TNegative:
		_I2ESelectSingleFunction(Counter::eFunctionCode::TNegative);
		_mI2EModule->digitalWrite(_cOLEDSelectTNegative, HIGH);
		break;
	case Counter::eFunctionCode::TEdgePositive:
		_I2ESelectSingleFunction(Counter::eFunctionCode::TEdgePositive);
		_mI2EModule->digitalWrite(_cOLEDSelectTEdgePositive, HIGH);
		break;
	case Counter::eFunctionCode::TEdgeNegative:
		_I2ESelectSingleFunction(Counter::eFunctionCode::TEdgeNegative);
		_mI2EModule->digitalWrite(_cOLEDSelectTEdgeNegative, HIGH);
		break;
	case Counter::eFunctionCode::TEventCounting:
		_I2ESelectSingleFunction(Counter::eFunctionCode::TEventCounting);
		_mI2EModule->digitalWrite(_cOLEDSelectTEdgePositive, HIGH);
		_mI2EModule->digitalWrite(_cOLEDSelectTEdgeNegative, HIGH);
		break;
	default:
		break;
	}
}

void FrontPlate::_I2ESelectSingleFunction(Counter::eFunctionCode iFunctionCode)
{
	DEBUG_METHOD_CALL("FrontPlate::_I2ESelectSingleFunction");

	_mSelectedCounterFunctionCode = iFunctionCode;
	_mCounter->I2ESetFunctionCode(_mSelectedCounterFunctionCode);
	_mLCDHandler->SetSelectedFunction(_mCounter->GetSelectedFunctionName());
	_mLCDHandler->TriggerShowCounter();
	switch (iFunctionCode)
	{
	case Counter::eFunctionCode::TFrequency:
	case Counter::eFunctionCode::TEventCounting:
		_mModuleFactory->GetSelectedModule()->I2ESelectFrequencyCounter();
		break;
	case Counter::eFunctionCode::TPositive:
	case Counter::eFunctionCode::TNegative:
	case Counter::eFunctionCode::TEdgePositive:
	case Counter::eFunctionCode::TEdgeNegative:
		_mModuleFactory->GetSelectedModule()->I2ESelectPeriodMeasurement();
		break;
	default:
		break;
	}

	_mChangeFunctionDetected = true;
	SetSetting(_cEepromIndexFunction, iFunctionCode);
	DEBUG_PRINT_LN("New function selected: " + _mCounter->GetSelectedFunctionName());
}

void FrontPlate::_I2ESwitchLEDs(uint8_t iTest)
{
	DEBUG_METHOD_CALL("FrontPlate::_I2ESwitchLEDs");

	_mI2EModule->digitalWrite(_cOLEDSelectFrequency, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTPositive, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTNegative, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTEdgePositive, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTEdgeNegative, iTest);
}

String FrontPlate::GetName()
{
	DEBUG_METHOD_CALL("FrontPlate::GetName");

	return _mText->GetObjectName();
}

void FrontPlate::TriggerLampTestOff()
{
	DEBUG_METHOD_CALL("FrontPlate::TriggerLampTestOff");

	_mTriggerLampTestOff = true;
}

bool FrontPlate::IsNewFunctionSelected()
{
	DEBUG_METHOD_CALL("FrontPlate::IsNewFunctionSelected");

	// the result is only one time valid
	bool lReturn = _mChangeFunctionDetected;

	_mChangeFunctionDetected = false;
	return lReturn;
}

bool FrontPlate::IsNewMenuSelected()
{
	DEBUG_METHOD_CALL("FrontPlate::IsNewMenuSelected");

	// the result is only one time valid
	bool lReturn = _mChangeMenuDecected;

	_mChangeMenuDecected = false;
	return lReturn;
}
