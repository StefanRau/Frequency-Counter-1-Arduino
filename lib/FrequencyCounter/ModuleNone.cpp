// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 27.10.2021: Constructor requires structure - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau

#include "ModuleNone.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleNone::TextModuleNone() : TextBase(-1)
{
	DebugInstantiation("New TextModuleNone");
}

TextModuleNone::~TextModuleNone()
{
}

String TextModuleNone::GetObjectName()
{
	switch (GetLanguage())
	{
		TextLangE("Dummy module");
		TextLangD("Dummymodul");
	}
}

String TextModuleNone::MenuItemStart()
{
	switch (GetLanguage())
	{
		TextLangE("Start");
		TextLangD("Start");
	}
}

String TextModuleNone::MenuItemCenter()
{
	switch (GetLanguage())
	{
		TextLangE("Center");
		TextLangD("Mitte");
	}
}

String TextModuleNone::MenuItemEnd()
{
	switch (GetLanguage())
	{
		TextLangE("End");
		TextLangD("Ende");
	}
}

/////////////////////////////////////////////////////////////

ModuleNone::ModuleNone(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
	DebugInstantiation("New ModuleNone: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	_mText = new TextModuleNone();
	mLastMenuEntryNumber = _cNumberOfMenuEntries;

	DebugPrintLn(GetName() + " module is initialized");
	mModuleIsInitialized = true;
}

ModuleNone::~ModuleNone()
{
}

void ModuleNone::I2EActivate()
{
}

void ModuleNone::I2EDeactivate()
{
}

void ModuleNone::I2ESelectFunction()
{
}

String ModuleNone::GetName()
{
	return _mText->GetObjectName();
}

bool ModuleNone::IsPeriodMeasurementPossible()
{
	return true;
}

bool ModuleNone::IsEventCountingPossible()
{
	return true;
}

String ModuleNone::GetCurrentMenuEntry(int iMenuEntry)
{
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
	return ModuleBase::eModuleCode::TModuleNone;
}
