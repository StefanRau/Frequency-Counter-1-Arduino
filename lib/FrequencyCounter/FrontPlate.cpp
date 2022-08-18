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

#include "FrontPlate.h"
#include "ErrorHandler.h"

// Text definitions

// extern TextFrontPlate gTextFrontPlate;
//extern TextFrontPlate gTextFrontPlate();

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextFrontPlate::TextFrontPlate() : TextBase(-1)
{
	DebugInstantiation("New TextI2CBase");
}

TextFrontPlate::~TextFrontPlate()
{
}

String TextFrontPlate::GetObjectName()
{
	switch (GetLanguage())
	{
		TextLangE("Front plate");
		TextLangD("Frontplatte");
	}
}

String TextFrontPlate::InitError()
{
	switch (GetLanguage())
	{
		TextLangE("Keyb/LEDs defect");
		TextLangD("Keyb/LEDs defekt");
	}
}

String TextFrontPlate::InitErrorCounterRequired()
{
	switch (GetLanguage())
	{
		TextLangE("Counter is required");
		TextLangD("Zähler wird benötigt");
	}
}

String TextFrontPlate::InitErrorModuleFactoryRequired()
{
	switch (GetLanguage())
	{
		TextLangE("Module factory required");
		TextLangD("Modul Factory benötigt");
	}
}

String TextFrontPlate::InitErrorLCDRequired()
{
	switch (GetLanguage())
	{
		TextLangE("LCD is required");
		TextLangD("LCD wird benötigt");
	}
}

String TextFrontPlate::ErrorPlausibilityViolation()
{
	switch (GetLanguage())
	{
		TextLangE("Frontplate plausibility violation");
		TextLangD("Frontplatte Plausibilitaetsverletzung");
	}
}

/////////////////////////////////////////////////////////////

FrontPlate::FrontPlate(sInitializeModule iInitializeModule, LCDHandler *iLCDHandler, ModuleFactory *iModuleFactory, Counter *iCounter) : I2CBase(iInitializeModule)
{
	DebugInstantiation("New FrontPlate: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	// Initialize hardware
	_mText = new TextFrontPlate();
	//_mText = &gTextFrontPlate;

	if (iLCDHandler == nullptr)
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitErrorLCDRequired());
		DebugPrint("Front Plate: LCD is required");
		return;
	}
	_mLCDHandler = iLCDHandler;

	if (iCounter == nullptr)
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitErrorCounterRequired());
		DebugPrint("Front Plate: Counter is required");
		return;
	}
	_mCounter = iCounter;

	if (iModuleFactory == nullptr)
	{
		ErrorPrint(Error::eSeverity::TFatal, _mText->InitErrorModuleFactoryRequired());
		DebugPrint("Front Plate: ModuleFactory is required");
		return;
	}
	_mModuleFactory = iModuleFactory;

	if (mI2CAddress < 0)
	{
		DebugPrint("Front Plate I2C address not defined");
		return;
	}

	_mI2EModule = new Adafruit_MCP23X17();
	if (!_mI2EModule->begin_I2C(mI2CAddress, &Wire))
	{
		ErrorPrint(Error::eSeverity::TWarning, _mText->InitError());
		DebugPrint("Front Plate can't be initialized");
		return;
	}
	DebugPrint("Front Plate is initialized at address: " + String(mI2CAddress));
	_mI2EModule->enableAddrPins();

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
}

void FrontPlate::loop()
{
	bool lFunctionKeyFrequencyPressed;
	bool lFunctionKeyPositivePressed;
	bool lFunctionKeyNegativePressed;
	bool lFunctionKeyEdgePositivePressed;
	bool lFunctionKeyEdgeNegativePressed;
	bool lIsPeriodMeasurementPossible;
	bool lIsEventCountingPossible;
	bool lMenuUpKeyPressed;
	bool lMenuDownKeyPressed;

	if (!mModuleIsInitialized)
	{
		return;
	}

	// switch off LEDs and show the current function on the LCD
	if (_mTriggerLampTestOff)
	{
		_I2ESelectFunction((Counter::eFunctionCode)GetSetting(_cEepromIndexFunction)); // Read setting from processor internal EEPROM
		_mLCDHandler->SetSelectedFunction(_mCounter->GetSelectedFunctionName());	   // Output at LCD
		_mCurrentModuleCode = _mModuleFactory->GetSelectedModule()->GetModuleCode();
		_mTriggerLampTestOff = false;
		return;
	}

	// if module has changed: set frequency measurement on, because that is sopported by all modules
	// e.g. HF module does not support periode measurement
	if (_mCurrentModuleCode != _mModuleFactory->GetSelectedModule()->GetModuleCode())
	{
		DebugPrint("Old Function code:" + String(_mCurrentModuleCode));
		_I2ESelectFunction(Counter::eFunctionCode::TFrequency);
		_mCurrentModuleCode = _mModuleFactory->GetSelectedModule()->GetModuleCode();
		DebugPrint("New Function code:" + String(_mCurrentModuleCode));
		return;
	}

	// Read keys
	if (!_mChangeFunctionDetected)
	{
		lFunctionKeyFrequencyPressed = _mI2EModule->digitalRead(_cIKeySelectFrequency);
		lFunctionKeyPositivePressed = _mI2EModule->digitalRead(_cIKeySelectTPositive);
		lFunctionKeyNegativePressed = _mI2EModule->digitalRead(_cIKeySelectTNegative);
		lFunctionKeyEdgePositivePressed = _mI2EModule->digitalRead(_cIKeySelectTEdgePositive);
		lFunctionKeyEdgeNegativePressed = _mI2EModule->digitalRead(_cIKeySelectTEdgeNegative);

		lIsPeriodMeasurementPossible = _mModuleFactory->GetSelectedModule()->IsPeriodMeasurementPossible();
		lIsEventCountingPossible = _mModuleFactory->GetSelectedModule()->IsEventCountingPossible();

		if (lFunctionKeyFrequencyPressed)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TFrequency)
			{
				DebugPrint("Frequency selected");
				_I2ESelectFunction(Counter::eFunctionCode::TFrequency);
				_mChangeFunctionDetected = true;
			}
		}

		if (lFunctionKeyPositivePressed && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TPositive)
			{
				DebugPrint("Level positive selected");
				_I2ESelectFunction(Counter::eFunctionCode::TPositive);
				_mChangeFunctionDetected = true;
			}
		}

		if (lFunctionKeyNegativePressed && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TNegative)
			{
				DebugPrint("Level negative selected");
				_I2ESelectFunction(Counter::eFunctionCode::TNegative);
				_mChangeFunctionDetected = true;
			}
		}

		if (lFunctionKeyEdgePositivePressed && (!lFunctionKeyEdgeNegativePressed) && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TEdgePositive)
			{
				DebugPrint("Edge negative selected");
				_I2ESelectFunction(Counter::eFunctionCode::TEdgePositive);
				_mChangeFunctionDetected = true;
			}
		}

		if ((!lFunctionKeyEdgePositivePressed) && lFunctionKeyEdgeNegativePressed && lIsPeriodMeasurementPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TEdgeNegative)
			{
				DebugPrint("Edge positive selected");
				_I2ESelectFunction(Counter::eFunctionCode::TEdgeNegative);
				_mChangeFunctionDetected = true;
			}
		}

		if (lFunctionKeyEdgePositivePressed && lFunctionKeyEdgeNegativePressed && lIsEventCountingPossible)
		{
			if (_mSelectedCounterFunctionCode != Counter::eFunctionCode::TEventCounting)
			{
				DebugPrint("Event counting selected");
				_I2ESelectFunction(Counter::eFunctionCode::TEventCounting);
				_mChangeFunctionDetected = true;
			}
		}

		// No function key is pressed
		if ((!lFunctionKeyFrequencyPressed) && (!lFunctionKeyPositivePressed) && (!lFunctionKeyNegativePressed) && (!lFunctionKeyEdgePositivePressed) && (!lFunctionKeyEdgeNegativePressed))
		{
			_mSelectedCounterFunctionCode = Counter::eFunctionCode::TNoSelection;
		}
	}

	// Menu key processing
	if (!_mChangeMenuDecected)
	{
		lMenuUpKeyPressed = _mI2EModule->digitalRead(_cIKeySelectMenuUp);
		lMenuDownKeyPressed = _mI2EModule->digitalRead(_cIKeySelectMenuDown);

		// Menu up is pressed?
		if (lMenuUpKeyPressed)
		{
			if (_mSelectedeMenuKeyCode != eMenuKeyCode::TMenuKeyUp)
			{
				DebugPrint("Menu up");
				_mModuleFactory->GetSelectedModule()->I2EScrollFunctionUp();
				_mSelectedeMenuKeyCode = eMenuKeyCode::TMenuKeyUp;
				_mChangeMenuDecected = true;
			}
		}

		// Menu down is pressed?
		if (lMenuDownKeyPressed)
		{
			if (_mSelectedeMenuKeyCode != eMenuKeyCode::TMenuKeyDown)
			{
				DebugPrint("Menu down");
				_mModuleFactory->GetSelectedModule()->I2EScrollFunctionDown();
				_mSelectedeMenuKeyCode = eMenuKeyCode::TMenuKeyDown;
				_mChangeMenuDecected = true;
			}
		}

		// No menu key is pressed
		if ((!lMenuUpKeyPressed) && (!lMenuDownKeyPressed))
		{
			_mSelectedeMenuKeyCode = eMenuKeyCode::TMenuKeyNo;
		}
	}
}

#ifndef _DebugApplication
String FrontPlate::Dispatch(char iModuleIdentifyer, char iParameter)
{
	String lReturn = "";

	switch (iModuleIdentifyer)
	{
	case eFunctionCode::TNameFunction:

		switch (iParameter)
		{

		case Counter::eFunctionCode::TFrequency:
			_I2ESelectFunction((Counter::eFunctionCode)iParameter);
			_mChangeFunctionDetected = true;
			return String(iParameter);

		case Counter::eFunctionCode::TPositive:
		case Counter::eFunctionCode::TNegative:
		case Counter::eFunctionCode::TEdgePositive:
		case Counter::eFunctionCode::TEdgeNegative:
		case Counter::eFunctionCode::TEventCounting:
			if (_mModuleFactory->GetSelectedModule()->IsPeriodMeasurementPossible())
			{
				_I2ESelectFunction((Counter::eFunctionCode)iParameter);
				_mChangeFunctionDetected = true;
			}
			return String(iParameter);

		case ProjectBase::eFunctionCode::TParameterGetAll:
			lReturn += ProjectBase::GetVerboseMode() ? _mText->FunctionNameFrequency() : String((char)Counter::eFunctionCode::TFrequency);

			if (_mModuleFactory->GetSelectedModule()->IsPeriodMeasurementPossible())
			{
				lReturn += ProjectBase::GetVerboseMode() ? ',' + _mText->FunctionNamePositive() +
															   ',' + _mText->FunctionNameNegative() +
															   ',' + _mText->FunctionNameEdgePositive() +
															   ',' + _mText->FunctionNameEdgeNegative() +
															   ',' + _mText->FunctionNameEventCounting()
														 : String((char)Counter::eFunctionCode::TPositive) +
															   String((char)Counter::eFunctionCode::TNegative) +
															   String((char)Counter::eFunctionCode::TEdgePositive) +
															   String((char)Counter::eFunctionCode::TEdgeNegative) +
															   String((char)Counter::eFunctionCode::TEventCounting);
			}

			return lReturn;

		case ProjectBase::eFunctionCode::TParameterGetCurrent:
			switch (_mSelectedFunctionCode)
			{
			case Counter::eFunctionCode::TFrequency:
			case Counter::eFunctionCode::TPositive:
			case Counter::eFunctionCode::TNegative:
			case Counter::eFunctionCode::TEdgePositive:
			case Counter::eFunctionCode::TEdgeNegative:
			case Counter::eFunctionCode::TEventCounting:
				if (ProjectBase::GetVerboseMode())
				{
					return GetSelectedFunctionName();
				}
				else
				{
					return String((char)_mSelectedFunctionCode);
				}
			}
			return String((char)Counter::eFunctionCode::TNoSelection);
		}

		return _mText->FunctionNameUnknown();

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

		return _mText->FunctionNameUnknown();
	}

	return String("");
}
#endif

void FrontPlate::_I2ESelectFunction(Counter::eFunctionCode iFunctionCode)
{
	DebugPrint("FrontPlate::_I2ESelectFunction:" + String(iFunctionCode));

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
	DebugPrint("FrontPlate::_I2ESelectSingleFunction:" + String(iFunctionCode));

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

	SetSetting(_cEepromIndexFunction, iFunctionCode);
	DebugPrint("New function selected: " + _mCounter->GetSelectedFunctionName());
}

void FrontPlate::_I2ESwitchLEDs(uint8_t iTest)
{
	_mI2EModule->digitalWrite(_cOLEDSelectFrequency, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTPositive, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTNegative, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTEdgePositive, iTest);
	_mI2EModule->digitalWrite(_cOLEDSelectTEdgeNegative, iTest);
}

String FrontPlate::GetName()
{
	return _mText->GetObjectName();
}

void FrontPlate::TriggerLampTestOff()
{
	_mTriggerLampTestOff = true;
}

bool FrontPlate::IsNewFunctionSelected()
{
	// the result is only one time valid
	bool lReturn = _mChangeFunctionDetected;

	_mChangeFunctionDetected = false;
	return lReturn;
}

bool FrontPlate::IsNewMenuSelected()
{
	// the result is only one time valid
	bool lReturn = _mChangeMenuDecected;

	_mChangeMenuDecected = false;
	return lReturn;
}
