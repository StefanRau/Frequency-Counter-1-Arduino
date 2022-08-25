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

#include "ModuleBase.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleBase::TextModuleBase() : TextBase(-1)
{
	DebugInstantiation("New TextModuleBase");
}

TextModuleBase::~TextModuleBase()
{
}

String TextModuleBase::GetObjectName()
{
	switch (GetLanguage())
	{
		TextLangE("Base");
		TextLangD("Basis");
	}
}

/////////////////////////////////////////////////////////////

ModuleBase::ModuleBase(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
    DebugInstantiation("New ModuleBase: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	_mText = new TextModuleBase();

	mI2EModule = new Adafruit_MCP23X17();
	mCurrentMenuEntryNumber = GetSetting(_cEepromIndexMenu);
}

ModuleBase::~ModuleBase()
{
}

void ModuleBase::loop()
{
}

#ifndef _DebugApplication
String ModuleBase::Dispatch(char iModuleIdentifyer, char iParameter)
{
	return String("");
}
#endif

void ModuleBase::I2EActivate()
{
	DebugPrint("Switch on module: " + GetName());
	I2ESwitchLamp(true);
}

void ModuleBase::I2EDeactivate()
{
	DebugPrint("Switch off module: " + GetName());
	mI2EModule->digitalWrite(_cOSelectionFrequency, LOW);
	mI2EModule->digitalWrite(_cOSelectionPeriod, LOW);
	I2ESwitchLamp(false);
}

void ModuleBase::I2EScrollFunctionUp()
{
	if (mCurrentMenuEntryNumber < (mLastMenuEntryNumber - 1))
	{
		mCurrentMenuEntryNumber += 1;
		SetSetting(_cEepromIndexMenu,mCurrentMenuEntryNumber);
		I2ESelectFunction();
	}
}

void ModuleBase::I2EScrollFunctionDown()
{
	if (mCurrentMenuEntryNumber > 0)
	{
		mCurrentMenuEntryNumber -= 1;
		SetSetting(_cEepromIndexMenu,mCurrentMenuEntryNumber);
		I2ESelectFunction();
	}
}

void ModuleBase::I2ESetCurrentMenuEntryNumber(int iCurrentMenuEntryNumber)
{
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
		SetSetting(_cEepromIndexMenu,mCurrentMenuEntryNumber);
		I2ESelectFunction();
	}
}

void ModuleBase::I2ESwitchLamp(bool iState)
{
	if (!mModuleIsInitialized)
	{
		return;
	}

	mI2EModule->digitalWrite(_cOAddressLED, Bool2State(iState));
}

bool ModuleBase::I2EIsKeySelected()
{
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
	if (!mModuleIsInitialized)
	{
		return;
	}

	iSelect ? I2EActivate() : I2EDeactivate();
}

void ModuleBase::I2ESelectFrequencyCounter()
{
	if (!mModuleIsInitialized)
	{
		return;
	}

	DebugPrint("Switch on frequency measurement for: " + GetName());
	mI2EModule->digitalWrite(_cOSelectionFrequency, HIGH);
	mI2EModule->digitalWrite(_cOSelectionPeriod, LOW);
}

void ModuleBase::I2ESelectPeriodMeasurement()
{
	if (!mModuleIsInitialized)
	{
		return;
	}

	DebugPrint("Switch on period measurement for: " + GetName());
	mI2EModule->digitalWrite(_cOSelectionFrequency, LOW);
	mI2EModule->digitalWrite(_cOSelectionPeriod, HIGH);
}

bool ModuleBase::I2EInitialize()
{
	mModuleIsInitialized = false;

	if (mI2CAddress < 0)
	{
		DebugPrint(GetName() + " I2C address is not defined");
		return false;
	}

	if (!mI2EModule->begin_I2C(mI2CAddress, &Wire))
	{
		DebugPrint(GetName() + " is not initialized");
		return false;
	}
	DebugPrint(GetName() + " is initialized at address: " + String(mI2CAddress));
	//mI2EModule->enableAddrPins();

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
	return mModuleIsInitialized;
}

int ModuleBase::GetCurrentMenuEntryNumber()
{
	return mCurrentMenuEntryNumber;
}

int ModuleBase::GetLastMenuEntryNumber()
{
	return mLastMenuEntryNumber;
}

#ifndef _DebugApplication
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
