// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 19.10.2021: 1st version - Stefan Rau
// 27.10.2021: Constructor requires structure - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau

#include "ModuleTTLCMOS.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleTTLCMOS::TextModuleTTLCMOS() : TextBase(-1)
{
	DebugInstantiation("New TextModuleTTLCMOS");
}

TextModuleTTLCMOS::~TextModuleTTLCMOS()
{
}

String TextModuleTTLCMOS::GetObjectName()
{
	switch (GetLanguage())
	{
		TextLangE("TTL/CMOS module");
		TextLangD("TTL/CMOS Modul");
	}
}

String TextModuleTTLCMOS::MenuItemTTL()
{
	return ("TTL");
}

String TextModuleTTLCMOS::MenuItemCMOS()
{
	return ("CMOS");
}

String TextModuleTTLCMOS::MenuItemOC()
{
	switch (GetLanguage())
	{
		TextLangE("Open collector");
		TextLangD("Open Collector");
	}
}

String TextModuleTTLCMOS::MenuItemOE()
{
	switch (GetLanguage())
	{
		TextLangE("Open emitter");
		TextLangD("Open Emitter");
	}
}

/////////////////////////////////////////////////////////////

ModuleTTLCMOS::ModuleTTLCMOS(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
    DebugInstantiation("New ModuleTTLCMOS: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	_mText = new TextModuleTTLCMOS();
	mLastMenuEntryNumber = _cNumberOfMenuEntries;

	// Initialize hardware
	if (I2EInitialize())
	{
		// initialize specific part
		mI2EModule->pinMode(_cOSelectTTL, OUTPUT);
		mI2EModule->pinMode(_cOSelectCMOS, OUTPUT);
		mI2EModule->pinMode(_cOSelectOpenEmitter, OUTPUT);
		mI2EModule->pinMode(_cOSelectOpenCollector, OUTPUT);
		mI2EModule->pinMode(_cOInputSelection0, OUTPUT);
		mI2EModule->pinMode(_cOInputSelection1, OUTPUT);

		// unassigned pins => set as input with pull ups
		mI2EModule->pinMode(_cB2Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(_cB3Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(_cB4Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(_cB5Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(_cB6Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(_cB7Unassigned, INPUT_PULLUP);
	}
}

ModuleTTLCMOS::~ModuleTTLCMOS()
{
}

void ModuleTTLCMOS::I2EActivate()
{
	DebugPrintLn("Switch on derived: " + GetName());
	ModuleBase::I2EActivate();
	I2ESelectFunction();
}

void ModuleTTLCMOS::I2EDeactivate()
{
	DebugPrintLn("Switch off derived: " + GetName());
	ModuleBase::I2EDeactivate();

	// Switch off all relais for input selection
	mI2EModule->digitalWrite(_cOSelectTTL, LOW);
	mI2EModule->digitalWrite(_cOSelectCMOS, LOW);
	mI2EModule->digitalWrite(_cOSelectOpenEmitter, LOW);
	mI2EModule->digitalWrite(_cOSelectOpenCollector, LOW);
}

void ModuleTTLCMOS::I2ESelectFunction()
{
	if (!mModuleIsInitialized)
	{
		return;
	}

	// Alle Relais zur Eingangswahl ausschalten
	mI2EModule->digitalWrite(_cOSelectTTL, LOW);
	mI2EModule->digitalWrite(_cOSelectCMOS, LOW);
	mI2EModule->digitalWrite(_cOSelectOpenEmitter, LOW);
	mI2EModule->digitalWrite(_cOSelectOpenCollector, LOW);
	delay(50);

	switch (mCurrentMenuEntryNumber)
	{
	case 0:
		// TTL Eingang
		DebugPrintLn("TTL input");
		mI2EModule->digitalWrite(_cOInputSelection0, LOW);
		mI2EModule->digitalWrite(_cOInputSelection1, LOW);
		mI2EModule->digitalWrite(_cOSelectTTL, HIGH); // Relais
		break;

	case 1:
		// CMOS Eingang
		DebugPrintLn("CMOS input");
		mI2EModule->digitalWrite(_cOInputSelection0, HIGH);
		mI2EModule->digitalWrite(_cOInputSelection1, LOW);
		mI2EModule->digitalWrite(_cOSelectCMOS, HIGH); // Relais
		break;

	case 2:
		// Open Emitter Eingang
		DebugPrintLn("Open emitter input");
		mI2EModule->digitalWrite(_cOInputSelection0, LOW);
		mI2EModule->digitalWrite(_cOInputSelection1, HIGH);
		mI2EModule->digitalWrite(_cOSelectOpenEmitter, HIGH); // Relais
		break;

	case 3:
		// Open Kollektor Eingang
		DebugPrintLn("Open collector input");
		mI2EModule->digitalWrite(_cOInputSelection0, HIGH);
		mI2EModule->digitalWrite(_cOInputSelection1, HIGH);
		mI2EModule->digitalWrite(_cOSelectOpenCollector, HIGH); // Relais
		break;
	}
}

String ModuleTTLCMOS::GetName()
{
	return _mText->GetObjectName();
}

bool ModuleTTLCMOS::IsPeriodMeasurementPossible()
{
	return true;
}

bool ModuleTTLCMOS::IsEventCountingPossible()
{
	return true;
}

String ModuleTTLCMOS::GetCurrentMenuEntry(int iMenuEntry)
{
	switch (iMenuEntry == -1 ? mCurrentMenuEntryNumber : iMenuEntry)
	{
	case 0:
		return _mText->MenuItemTTL();
	case 1:
		return _mText->MenuItemCMOS();
	case 2:
		return _mText->MenuItemOE();
	case 3:
		return _mText->MenuItemOC();
	}
	return "";
}

ModuleBase::eModuleCode ModuleTTLCMOS::GetModuleCode()
{
	return ModuleBase::eModuleCode::TModuleTTLCMOS;
}
