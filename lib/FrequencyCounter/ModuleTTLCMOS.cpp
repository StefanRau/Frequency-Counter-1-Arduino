// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 19.10.2021: 1st version - Stefan Rau
// 27.10.2021: Constructor requires structure - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "ModuleTTLCMOS.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleTTLCMOS::TextModuleTTLCMOS() : TextBase()
{
	DEBUG_INSTANTIATION("TextModuleTTLCMOS");
}

TextModuleTTLCMOS::~TextModuleTTLCMOS()
{
	DEBUG_DESTROY("TextModuleTTLCMOS");
}

String TextModuleTTLCMOS::GetObjectName()
{
	DEBUG_METHOD_CALL("TextModuleTTLCMOS::GetObjectName");

	switch (GetLanguage())
	{
		TextLangE("TTL/CMOS module");
		TextLangD("TTL/CMOS Modul");
	}
}

String TextModuleTTLCMOS::MenuItemTTL()
{
	DEBUG_METHOD_CALL("TextModuleTTLCMOS::MenuItemTTL");

	return ("TTL");
}

String TextModuleTTLCMOS::MenuItemCMOS()
{
	DEBUG_METHOD_CALL("TextModuleTTLCMOS::MenuItemCMOS");

	return ("CMOS");
}

String TextModuleTTLCMOS::MenuItemOC()
{
	DEBUG_METHOD_CALL("TextModuleTTLCMOS::MenuItemOC");

	switch (GetLanguage())
	{
		TextLangE("Open collector");
		TextLangD("Open Collector");
	}
}

String TextModuleTTLCMOS::MenuItemOE()
{
	DEBUG_METHOD_CALL("TextModuleTTLCMOS::MenuItemOE");

	switch (GetLanguage())
	{
		TextLangE("Open emitter");
		TextLangD("Open Emitter");
	}
}

/////////////////////////////////////////////////////////////

ModuleTTLCMOS::ModuleTTLCMOS(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
	DEBUG_INSTANTIATION("ModuleTTLCMOS: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	mText = new TextModuleTTLCMOS();
	mLastMenuEntryNumber = cNumberOfMenuEntries;

	// Initialize hardware
	if (I2EInitialize())
	{
		// initialize specific part
		mI2EModule->pinMode(cOSelectTTL, OUTPUT);
		mI2EModule->pinMode(cOSelectCMOS, OUTPUT);
		mI2EModule->pinMode(cOSelectOpenEmitter, OUTPUT);
		mI2EModule->pinMode(cOSelectOpenCollector, OUTPUT);
		mI2EModule->pinMode(cOInputSelection0, OUTPUT);
		mI2EModule->pinMode(cOInputSelection1, OUTPUT);

		// unassigned pins => set as input with pull ups
		mI2EModule->pinMode(cB2Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(cB3Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(cB4Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(cB5Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(cB6Unassigned, INPUT_PULLUP);
		mI2EModule->pinMode(cB7Unassigned, INPUT_PULLUP);
	}
}

ModuleTTLCMOS::~ModuleTTLCMOS()
{
	DEBUG_DESTROY("ModuleTTLCMOS");
}

void ModuleTTLCMOS::I2EActivate()
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::I2EActivate");

	DEBUG_PRINT_LN("Switch on derived: " + GetName());
	ModuleBase::I2EActivate();
	I2ESelectFunction();
}

void ModuleTTLCMOS::I2EDeactivate()
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::I2EDeactivate");

	DEBUG_PRINT_LN("Switch off derived: " + GetName());
	ModuleBase::I2EDeactivate();

	// Switch off all relais for input selection
	mI2EModule->digitalWrite(cOSelectTTL, LOW);
	mI2EModule->digitalWrite(cOSelectCMOS, LOW);
	mI2EModule->digitalWrite(cOSelectOpenEmitter, LOW);
	mI2EModule->digitalWrite(cOSelectOpenCollector, LOW);
}

void ModuleTTLCMOS::I2ESelectFunction()
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::I2ESelectFunction");

	if (!mModuleIsInitialized)
	{
		return;
	}

	// Alle Relais zur Eingangswahl ausschalten
	mI2EModule->digitalWrite(cOSelectTTL, LOW);
	mI2EModule->digitalWrite(cOSelectCMOS, LOW);
	mI2EModule->digitalWrite(cOSelectOpenEmitter, LOW);
	mI2EModule->digitalWrite(cOSelectOpenCollector, LOW);
	delay(50);

	switch (mCurrentMenuEntryNumber)
	{
	case 0:
		// TTL Eingang
		DEBUG_PRINT_LN("TTL input");
		mI2EModule->digitalWrite(cOInputSelection0, LOW);
		mI2EModule->digitalWrite(cOInputSelection1, LOW);
		mI2EModule->digitalWrite(cOSelectTTL, HIGH); // Relais
		break;

	case 1:
		// CMOS Eingang
		DEBUG_PRINT_LN("CMOS input");
		mI2EModule->digitalWrite(cOInputSelection0, HIGH);
		mI2EModule->digitalWrite(cOInputSelection1, LOW);
		mI2EModule->digitalWrite(cOSelectCMOS, HIGH); // Relais
		break;

	case 2:
		// Open Emitter Eingang
		DEBUG_PRINT_LN("Open emitter input");
		mI2EModule->digitalWrite(cOInputSelection0, LOW);
		mI2EModule->digitalWrite(cOInputSelection1, HIGH);
		mI2EModule->digitalWrite(cOSelectOpenEmitter, HIGH); // Relais
		break;

	case 3:
		// Open Kollektor Eingang
		DEBUG_PRINT_LN("Open collector input");
		mI2EModule->digitalWrite(cOInputSelection0, HIGH);
		mI2EModule->digitalWrite(cOInputSelection1, HIGH);
		mI2EModule->digitalWrite(cOSelectOpenCollector, HIGH); // Relais
		break;
	}
}

String ModuleTTLCMOS::GetName()
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::GetName");

	return mText->GetObjectName();
}

bool ModuleTTLCMOS::IsPeriodMeasurementPossible()
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::IsPeriodMeasurementPossible");

	return true;
}

bool ModuleTTLCMOS::IsEventCountingPossible()
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::IsEventCountingPossible");

	return true;
}

String ModuleTTLCMOS::GetCurrentMenuEntry(int iMenuEntry)
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::GetCurrentMenuEntry");

	switch (iMenuEntry == -1 ? mCurrentMenuEntryNumber : iMenuEntry)
	{
	case 0:
		return mText->MenuItemTTL();
	case 1:
		return mText->MenuItemCMOS();
	case 2:
		return mText->MenuItemOE();
	case 3:
		return mText->MenuItemOC();
	}
	return "";
}

ModuleBase::eModuleCode ModuleTTLCMOS::GetModuleCode()
{
	DEBUG_METHOD_CALL("ModuleTTLCMOS::GetModuleCode");

	return ModuleBase::eModuleCode::TModuleTTLCMOS;
}
