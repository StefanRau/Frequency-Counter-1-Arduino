// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 19.10.2021: 1st version - Stefan Rau
// 21.10.2021: enumeration eModule is of type char - Stefan Rau
// 21.10.2021: Dispatcher has now chars as input - Stefan Rau
// 27.10.2021: Constructor requires structure - Stefan Rau
// 07.11.2021: Get all menu entry items - Stefan Rau
// 17.11.2021: LED of module did not work => fixed - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 20.01.2023: Improve debug handling - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "ModuleBase.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleBase::TextModuleBase() : TextBase()
{
	DebugInstantiation("TextModuleBase");
}

TextModuleBase::~TextModuleBase()
{
	DebugDestroy("TextModuleBase");
}

String TextModuleBase::GetObjectName()
{
	DebugMethodCalls("TextModuleBase::GetObjectName");

	switch (GetLanguage())
	{
		TextLangE("Base");
		TextLangD("Basis");
	}
}

/////////////////////////////////////////////////////////////

ModuleBase::ModuleBase(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
	DebugInstantiation("ModuleBase: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	_mText = new TextModuleBase();

	mI2EModule = new Adafruit_MCP23X17();
	mCurrentMenuEntryNumber = GetSetting(_cEepromIndexMenu);
	if (mCurrentMenuEntryNumber == cNullSetting)
	{
		mCurrentMenuEntryNumber = 0;
	}
}

ModuleBase::~ModuleBase()
{
	DebugDestroy("ModuleBase");
}

void ModuleBase::loop()
{
	DebugMethodCalls("ModuleBase::loop");
}

#if DEBUG_APPLICATION == 0
String ModuleBase::DispatchSerial(char iModuleIdentifyer, char iParameter)
{
	return String("");
}
#endif

void ModuleBase::I2EActivate()
{
	DebugMethodCalls("ModuleBase::I2EActivate");

	DebugPrintLn("Switch on module: " + GetName());
	I2ESwitchLamp(true);
}

void ModuleBase::I2EDeactivate()
{
	DebugMethodCalls("ModuleBase::I2EDeactivate");

	DebugPrintLn("Switch off module: " + GetName());
	mI2EModule->digitalWrite(_cOSelectionFrequency, LOW);
	mI2EModule->digitalWrite(_cOSelectionPeriod, LOW);
	I2ESwitchLamp(false);
}

void ModuleBase::I2EScrollFunctionUp()
{
	DebugMethodCalls("ModuleBase::I2EScrollFunctionUp");

	if (mCurrentMenuEntryNumber < (mLastMenuEntryNumber - 1))
	{
		mCurrentMenuEntryNumber += 1;
		SetSetting(_cEepromIndexMenu, mCurrentMenuEntryNumber);
		I2ESelectFunction();
	}
}

void ModuleBase::I2EScrollFunctionDown()
{
	DebugMethodCalls("ModuleBase::I2EScrollFunctionDown");

	if (mCurrentMenuEntryNumber > 0)
	{
		mCurrentMenuEntryNumber -= 1;
		SetSetting(_cEepromIndexMenu, mCurrentMenuEntryNumber);
		I2ESelectFunction();
	}
}

void ModuleBase::I2ESetCurrentMenuEntryNumber(int iCurrentMenuEntryNumber)
{
	DebugMethodCalls("ModuleBase::I2ESetCurrentMenuEntryNumber");

	int lCurrentMenuEntryNumber = mCurrentMenuEntryNumber;

	if (iCurrentMenuEntryNumber < 0)
	{
		mCurrentMenuEntryNumber = 0;
	}
	else if (iCurrentMenuEntryNumber > mLastMenuEntryNumber)
	{
		mCurrentMenuEntryNumber = mLastMenuEntryNumber - 1;
	}
	else
	{
		mCurrentMenuEntryNumber = iCurrentMenuEntryNumber;
	}

	// Something changed?
	if (lCurrentMenuEntryNumber != mCurrentMenuEntryNumber)
	{
		SetSetting(_cEepromIndexMenu, mCurrentMenuEntryNumber);
		I2ESelectFunction();
	}
}

void ModuleBase::I2ESwitchLamp(bool iState)
{
	DebugMethodCalls("ModuleBase::I2ESwitchLamp");

	if (!mModuleIsInitialized)
	{
		return;
	}

	mI2EModule->digitalWrite(_cOAddressLED, Bool2State(iState));
}

bool ModuleBase::I2EIsKeySelected()
{
	DebugMethodCalls("ModuleBase::I2EIsKeySelected");

	if (!mModuleIsInitialized)
	{
		return false;
	}

	if (mI2EModule->digitalRead(_cIAddressSelectionButton) == HIGH)
	{
		return true;
	}

	return false;
}

void ModuleBase::I2ESelectModule(bool iSelect)
{
	DebugMethodCalls("ModuleBase::I2ESelectModule");

	if (!mModuleIsInitialized)
	{
		return;
	}

	iSelect ? I2EActivate() : I2EDeactivate();
}

void ModuleBase::I2ESelectFrequencyCounter()
{
	DebugMethodCalls("ModuleBase::I2ESelectFrequencyCounter");

	if (!mModuleIsInitialized)
	{
		return;
	}

	DebugPrintLn("Switch on frequency measurement for: " + GetName());
	mI2EModule->digitalWrite(_cOSelectionFrequency, HIGH);
	mI2EModule->digitalWrite(_cOSelectionPeriod, LOW);
}

void ModuleBase::I2ESelectPeriodMeasurement()
{
	DebugMethodCalls("ModuleBase::I2ESelectPeriodMeasurement");

	if (!mModuleIsInitialized)
	{
		return;
	}

	DebugPrintLn("Switch on period measurement for: " + GetName());
	mI2EModule->digitalWrite(_cOSelectionFrequency, LOW);
	mI2EModule->digitalWrite(_cOSelectionPeriod, HIGH);
}

bool ModuleBase::I2EInitialize()
{
	DebugMethodCalls("ModuleBase::I2EInitialize");

	mModuleIsInitialized = false;

	if (mI2CAddress < 0)
	{
		DebugPrintLn(GetName() + " I2C address is not defined");
		return false;
	}

	if (!mI2EModule->begin_I2C(mI2CAddress, &Wire))
	{
		DebugPrintLn(GetName() + " is not initialized");
		return false;
	}
	DebugPrintLn(GetName() + " is initialized at address: " + String(mI2CAddress));
	// mI2EModule->enableAddrPins();

	// set common I/O
	mI2EModule->pinMode(_cOSelectionFrequency, OUTPUT);
	mI2EModule->pinMode(_cOSelectionPeriod, OUTPUT);
	mI2EModule->pinMode(_cOAddressLED, OUTPUT);
	mI2EModule->pinMode(_cIAddressSelectionButton, INPUT);

	mModuleIsInitialized = true;
	return true;
}

bool ModuleBase::IsModuleInitialized()
{
	DebugMethodCalls("ModuleBase::IsModuleInitialized");

	return mModuleIsInitialized;
}

int ModuleBase::GetCurrentMenuEntryNumber()
{
	DebugMethodCalls("ModuleBase::GetCurrentMenuEntryNumber");

	return mCurrentMenuEntryNumber;
}

int ModuleBase::GetLastMenuEntryNumber()
{
	DebugMethodCalls("ModuleBase::GetLastMenuEntryNumber");

	return mLastMenuEntryNumber;
}

#if DEBUG_APPLICATION == 0
String ModuleBase::GetAllMenuEntryItems()
{
	String lReturn = "";

	for (int lMenuEntryNumberIterator = 0; lMenuEntryNumberIterator < mLastMenuEntryNumber; lMenuEntryNumberIterator++)
	{
		if (ProjectBase::GetVerboseMode())
		{
			if (lReturn == "")
			{
				lReturn = GetCurrentMenuEntry(lMenuEntryNumberIterator);
			}
			else
			{
				lReturn += "," + GetCurrentMenuEntry(lMenuEntryNumberIterator);
			}
		}
		else
		{
			lReturn += String(lMenuEntryNumberIterator);
		}
	}
	return lReturn;
}
#endif
