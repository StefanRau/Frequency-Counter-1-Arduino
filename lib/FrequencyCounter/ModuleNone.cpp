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
	DEBUG_INSTANTIATION("TextModuleNone");
}

TextModuleNone::~TextModuleNone()
{
	DEBUG_DESTROY("TextModuleNone");
}

String TextModuleNone::GetObjectName()
{
	DEBUG_METHOD_CALL("TextModuleNone::GetObjectName");

	switch (GetLanguage())
	{
		TextLangE("Dummy module");
		TextLangD("Dummymodul");
	}
}

String TextModuleNone::MenuItemStart()
{
	DEBUG_METHOD_CALL("TextModuleNone::MenuItemStart");

	switch (GetLanguage())
	{
		TextLangE("Start");
		TextLangD("Start");
	}
}

String TextModuleNone::MenuItemCenter()
{
	DEBUG_METHOD_CALL("TextModuleNone::MenuItemCenter");

	switch (GetLanguage())
	{
		TextLangE("Center");
		TextLangD("Mitte");
	}
}

String TextModuleNone::MenuItemEnd()
{
	DEBUG_METHOD_CALL("TextModuleNone::MenuItemEnd");

	switch (GetLanguage())
	{
		TextLangE("End");
		TextLangD("Ende");
	}
}

/////////////////////////////////////////////////////////////

ModuleNone::ModuleNone(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
	DEBUG_INSTANTIATION("ModuleNone: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	mText = new TextModuleNone();
	mLastMenuEntryNumber = cNumberOfMenuEntries;

	DEBUG_PRINT_LN(GetName() + " module is initialized");
	mModuleIsInitialized = true;
}

ModuleNone::~ModuleNone()
{
	DEBUG_DESTROY("ModuleNone");
}

void ModuleNone::I2EActivate()
{
	DEBUG_METHOD_CALL("ModuleNone::I2EActivate");
}

void ModuleNone::I2EDeactivate()
{
	DEBUG_METHOD_CALL("ModuleNone::I2EDeactivate");
}

void ModuleNone::I2ESelectFunction()
{
	DEBUG_METHOD_CALL("ModuleNone::I2ESelectFunction");
}

String ModuleNone::GetName()
{
	DEBUG_METHOD_CALL("ModuleNone::GetName");

	return mText->GetObjectName();
}

bool ModuleNone::IsPeriodMeasurementPossible()
{
	DEBUG_METHOD_CALL("ModuleNone::IsPeriodMeasurementPossible");

	return true;
}

bool ModuleNone::IsEventCountingPossible()
{
	DEBUG_METHOD_CALL("ModuleNone::IsEventCountingPossible");

	return true;
}

String ModuleNone::GetCurrentMenuEntry(int iMenuEntry)
{
	DEBUG_METHOD_CALL("ModuleNone::GetCurrentMenuEntry");

	switch (iMenuEntry == -1 ? mCurrentMenuEntryNumber : iMenuEntry)
	{
	case 0:
		return mText->MenuItemStart();
	case 1:
		return mText->MenuItemCenter();
	case 2:
		return mText->MenuItemEnd();
	}
	return "";
}

ModuleBase::eModuleCode ModuleNone::GetModuleCode()
{
	DEBUG_METHOD_CALL("ModuleNone::GetModuleCode");

	return ModuleBase::eModuleCode::TModuleNone;
}
