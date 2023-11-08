// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 27.10.2021: Constructor requires structure - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "ModuleNone.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleNone::TextModuleNone() : TextBase()
{
	DebugInstantiation("TextModuleNone");
}

TextModuleNone::~TextModuleNone()
{
	DebugDestroy("TextModuleNone");
}

String TextModuleNone::GetObjectName()
{
	DebugMethodCalls("TextModuleNone::GetObjectName");

	switch (GetLanguage())
	{
		TextLangE("Dummy module");
		TextLangD("Dummymodul");
	}
}

String TextModuleNone::MenuItemStart()
{
	DebugMethodCalls("TextModuleNone::MenuItemStart");

	switch (GetLanguage())
	{
		TextLangE("Start");
		TextLangD("Start");
	}
}

String TextModuleNone::MenuItemCenter()
{
	DebugMethodCalls("TextModuleNone::MenuItemCenter");

	switch (GetLanguage())
	{
		TextLangE("Center");
		TextLangD("Mitte");
	}
}

String TextModuleNone::MenuItemEnd()
{
	DebugMethodCalls("TextModuleNone::MenuItemEnd");

	switch (GetLanguage())
	{
		TextLangE("End");
		TextLangD("Ende");
	}
}

/////////////////////////////////////////////////////////////

ModuleNone::ModuleNone(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
	DebugInstantiation("ModuleNone: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	_mText = new TextModuleNone();
	mLastMenuEntryNumber = _cNumberOfMenuEntries;

	DebugPrintLn(GetName() + " module is initialized");
	mModuleIsInitialized = true;
}

ModuleNone::~ModuleNone()
{
	DebugDestroy("ModuleNone");
}

void ModuleNone::I2EActivate()
{
	DebugMethodCalls("ModuleNone::I2EActivate");
}

void ModuleNone::I2EDeactivate()
{
	DebugMethodCalls("ModuleNone::I2EDeactivate");
}

void ModuleNone::I2ESelectFunction()
{
	DebugMethodCalls("ModuleNone::I2ESelectFunction");
}

String ModuleNone::GetName()
{
	DebugMethodCalls("ModuleNone::GetName");

	return _mText->GetObjectName();
}

bool ModuleNone::IsPeriodMeasurementPossible()
{
	DebugMethodCalls("ModuleNone::IsPeriodMeasurementPossible");

	return true;
}

bool ModuleNone::IsEventCountingPossible()
{
	DebugMethodCalls("ModuleNone::IsEventCountingPossible");

	return true;
}

String ModuleNone::GetCurrentMenuEntry(int iMenuEntry)
{
	DebugMethodCalls("ModuleNone::GetCurrentMenuEntry");

	switch (iMenuEntry == -1 ? mCurrentMenuEntryNumber : iMenuEntry)
	{
	case 0:
		return _mText->MenuItemStart();
	case 1:
		return _mText->MenuItemCenter();
	case 2:
		return _mText->MenuItemEnd();
	}
	return "";
}

ModuleBase::eModuleCode ModuleNone::GetModuleCode()
{
	DebugMethodCalls("ModuleNone::GetModuleCode");

	return ModuleBase::eModuleCode::TModuleNone;
}
