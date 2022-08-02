// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 27.10.2021: Constructor requires structure - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau

#include "ModuleAnalog.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleAnalog::TextModuleAnalog() : TextBase(-1)
{
	DebugInstantiation("New TextModuleAnalog");
}

TextModuleAnalog::~TextModuleAnalog()
{
}

String TextModuleAnalog::GetObjectName()
{
	switch (GetLanguage())
	{
		TextLangE("Analog module");
		TextLangD("Analogmodul");
	}
}

String TextModuleAnalog::MenuItem100uV()
{
	return ("100uV");
}

String TextModuleAnalog::MenuItem1mV()
{
	return ("1mV");
}

String TextModuleAnalog::MenuItem10mV()
{
	return ("10mV");
}

String TextModuleAnalog::MenuItem100mV()
{
	return ("100mV");
}

String TextModuleAnalog::MenuItem1V()
{
	return ("1V");
}

String TextModuleAnalog::MenuItem10V()
{
	return ("10V");
}

String TextModuleAnalog::MenuItem100V()
{
	return ("100V");
}

/////////////////////////////////////////////////////////////

ModuleAnalog::ModuleAnalog(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
	DebugInstantiation("New ModuleAnalog: iInitializeModule[SettingsAddress, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.I2CAddress) + "]");

	_mText = new TextModuleAnalog();
	mLastMenuEntryNumber = _cNumberOfMenuEntries;

	// Initialize hardware
	if (I2EInitialize())
	{
		// initialize specific part
	}
}

ModuleAnalog::~ModuleAnalog()
{
}

void ModuleAnalog::I2EActivate()
{
}

void ModuleAnalog::I2EDeactivate()
{
}

void ModuleAnalog::I2ESelectFunction()
{
	if (!mModuleIsInitialized)
	{
		return;
	}

	// Alle Relais zur Eingangswahl ausschalten

	// switch (mSelectedFunction)
	// {
	// case 0:
	//     mHardwareModule.digitalWrite(cInputSelection1, LOW);
	//     mHardwareModule.digitalWrite(cInputSelection2, LOW);
	//     break;

	// case 1:
	//     mHardwareModule.digitalWrite(cInputSelection1, HIGH);
	//     mHardwareModule.digitalWrite(cInputSelection2, LOW);
	//     break;

	// case 2:
	//     mHardwareModule.digitalWrite(cInputSelection1, LOW);
	//     mHardwareModule.digitalWrite(cInputSelection2, HIGH);
	//     break;

	// case 3:
	//     mHardwareModule.digitalWrite(cInputSelection1, HIGH);
	//     mHardwareModule.digitalWrite(cInputSelection2, HIGH);
	//     break;
	// }
}

String ModuleAnalog::GetName()
{
	return _mText->GetObjectName();
}

bool ModuleAnalog::IsPeriodMeasurementPossible()
{
	return true;
}

bool ModuleAnalog::IsEventCountingPossible()
{
	return true;
}

String ModuleAnalog::GetCurrentMenuEntry(int iMenuEntry)
{
	switch (iMenuEntry == -1 ? mCurrentMenuEntryNumber : iMenuEntry)
	{
	case 0:
		return _mText->MenuItem100uV();
		break;
	case 1:
		return _mText->MenuItem1mV();
		break;
	case 2:
		return _mText->MenuItem10mV();
		break;
	case 3:
		return _mText->MenuItem100mV();
		break;
	case 4:
		return _mText->MenuItem1V();
		break;
	case 5:
		return _mText->MenuItem10V();
		break;
	case 6:
		return _mText->MenuItem100V();
		break;
	}
	return "";
}

char ModuleAnalog::GetModuleCode()
{
	return ModuleBase::eModuleCode::TModuleAnalog;
}
