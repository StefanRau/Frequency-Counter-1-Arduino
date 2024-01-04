// Arduino Frequency Counter
// 20.10.2021
// Stefan Rau
// History
// 20.10.2021: 1st version - Stefan Rau
// 21.10.2021: Dispatcher has now chars as input
// 27.10.2021: Constructor requires structure - Stefan Rau
// 04.11.2021: Verbose mode implemented - Stefan Rau
// 17.11.2021: LED of module did not work => fixed - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 06.09.2022: Singleton instantiation - Stefan Rau
// 21.09.2022: use GetInstance instead of Get<Typename> - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 20.01.2023: Improve debug handling - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "ModuleFactory.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleFactory::TextModuleFactory() : TextBase()
{
	DEBUG_INSTANTIATION("TextModuleFactory");
}

TextModuleFactory::~TextModuleFactory()
{
	DEBUG_DESTROY("TextModuleFactory");
}

String TextModuleFactory::GetObjectName()
{
	DEBUG_METHOD_CALL("TextModuleFactory::GetObjectName");

	switch (GetLanguage())
	{
		TextLangE("Module factory");
		TextLangD("Module Factory");
	}
}

String TextModuleFactory::Unknown()
{
	DEBUG_METHOD_CALL("TextModuleFactory::Unknown");

	switch (GetLanguage())
	{
		TextLangE("Unknown function");
		TextLangD("Unbekannte Funktion");
	}
}

/////////////////////////////////////////////////////////////

static ModuleFactory *gInstance = nullptr;

ModuleFactory::ModuleFactory(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
	DEBUG_INSTANTIATION("ModuleFactory: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	sInitializeModule lInitializeModule = iInitializeModule;

	mText = new TextModuleFactory();

	// Initialize the hardware of all modules
	// 100 MHz TTL / CMOS
	mModuleTTLCMOS = new ModuleTTLCMOS(lInitializeModule);

	// 100 MHz analog
	lInitializeModule.I2CAddress += 1;
	lInitializeModule.SettingsAddress += iInitializeModule.NumberOfSettings;
	mModuleAnalog = new ModuleAnalog(lInitializeModule);

	// 10GHz
	lInitializeModule.I2CAddress += 1;
	lInitializeModule.SettingsAddress += iInitializeModule.NumberOfSettings;
	mModuleHF = new ModuleHF(lInitializeModule);

	// Dummy module
	lInitializeModule.I2CAddress = -1;
	lInitializeModule.SettingsAddress += iInitializeModule.NumberOfSettings;
	mModuleNone = new ModuleNone(lInitializeModule);

	if (mModuleTTLCMOS->IsModuleInitialized())
	{
		mSelectedModule = mModuleTTLCMOS;
	}
	else
	{
		if (mModuleAnalog->IsModuleInitialized())
		{
			mSelectedModule = mModuleAnalog;
		}
		else
		{
			if (mModuleHF->IsModuleInitialized())
			{
				mSelectedModule = mModuleHF;
			}
			else
			{
				mSelectedModule = mModuleNone;
			}
		}
	}

	mSelectedModule->I2ESelectFunction();
	DEBUG_PRINT_LN("Module factory is initialized");
}

ModuleFactory ::~ModuleFactory()
{
	DEBUG_DESTROY("ModuleFactory");
}

ModuleFactory *ModuleFactory::GetInstance(sInitializeModule iInitializeModule)
{
	DEBUG_METHOD_CALL("ModuleFactory::GetInstance");

	gInstance = (gInstance == nullptr) ? new ModuleFactory(iInitializeModule) : gInstance;
	return gInstance;
}

void ModuleFactory::loop()
{
	DEBUG_METHOD_CALL("ModuleFactory::loop");

	// check the state of the key of all modules
	if (mModuleTTLCMOS->I2EIsKeySelected())
	{
		if (mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleTTLCMOS)
		{
			DEBUG_PRINT_LN("\nModule TTL/CMOS key pressed");
			I2ESelectModule(ModuleBase::eModuleCode::TModuleTTLCMOS);
			delay(100);
		}
	};

	if (mModuleAnalog->I2EIsKeySelected())
	{
		if (mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleAnalog)
		{
			DEBUG_PRINT_LN("\nModule Analog key pressed");
			I2ESelectModule(ModuleBase::eModuleCode::TModuleAnalog);
			delay(100);
		}
	};

	if (mModuleHF->I2EIsKeySelected())
	{
		if (mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleHF)
		{
			DEBUG_PRINT_LN("\nModule HF key pressed");
			I2ESelectModule(ModuleBase::eModuleCode::TModuleHF);
			delay(100);
		}
	};

	if (mTriggerLampTestOff)
	{
		mModuleTTLCMOS->I2ESwitchLamp(false); // 100 MHz TTL / CMOS
		mModuleAnalog->I2ESwitchLamp(false);  // 100 MHz Analog
		mModuleHF->I2ESwitchLamp(false);	   // 10GHz
		mTriggerLampTestOff = false;
		I2ESelectModule(mSelectedModule->GetModuleCode());
	}
}

#if DEBUG_APPLICATION == 0
String ModuleFactory::DispatchSerial(char iModuleIdentifyer, char iParameter)
{
	String lReturn = "";

	switch (iModuleIdentifyer)
	{
	case TName:

		switch (iParameter)
		{

		case ModuleBase::eModuleCode::TModuleTTLCMOS:
		case ModuleBase::eModuleCode::TModuleAnalog:
		case ModuleBase::eModuleCode::TModuleHF:
		case ModuleBase::eModuleCode::TModuleNone:
			_I2ESelectModule(iParameter);
			return String(iParameter);

		case ProjectBase::eFunctionCode::TParameterGetAll:
			if (_mModuleTTLCMOS->IsModuleInitialized())
			{
				lReturn += ProjectBase::GetVerboseMode() ? _mModuleTTLCMOS->GetName() : String((char)ModuleBase::eModuleCode::TModuleTTLCMOS);
			}

			if (_mModuleAnalog->IsModuleInitialized())
			{
				lReturn += ProjectBase::GetVerboseMode() ? (lReturn != "" ? "," : "") + _mModuleAnalog->GetName() : String((char)ModuleBase::eModuleCode::TModuleAnalog);
			}

			if (_mModuleHF->IsModuleInitialized())
			{
				lReturn += ProjectBase::GetVerboseMode() ? (lReturn != "" ? "," : "") + _mModuleHF->GetName() : String((char)ModuleBase::eModuleCode::TModuleHF);
			}

			if (_mModuleNone->IsModuleInitialized())
			{
				lReturn += ProjectBase::GetVerboseMode() ? (lReturn != "" ? "," : "") + _mModuleNone->GetName() : String((char)ModuleBase::eModuleCode::TModuleNone);
			}

			return lReturn;

		case ProjectBase::eFunctionCode::TParameterGetCurrent:
			char lModuleCode = _mSelectedModule->GetModuleCode();
			switch (lModuleCode)
			{
			case ModuleBase::eModuleCode::TModuleTTLCMOS:
			case ModuleBase::eModuleCode::TModuleAnalog:
			case ModuleBase::eModuleCode::TModuleHF:
			case ModuleBase::eModuleCode::TModuleNone:
				if (ProjectBase::GetVerboseMode())
				{
					return _mSelectedModule->GetName();
				}
				else
				{
					return String((char)lModuleCode);
				}
			}
			return "";
		}
		return _mText->Unknown();
	}
	return "";
}
#endif

void ModuleFactory::I2ELampTestOn()
{
	DEBUG_METHOD_CALL("ModuleFactory::I2ELampTestOn");

	mModuleTTLCMOS->I2ESwitchLamp(true); // 100 MHz TTL / CMOS
	mModuleAnalog->I2ESwitchLamp(true);  // 100 MHz Analog
	mModuleHF->I2ESwitchLamp(true);	  // 10GHz
}

String ModuleFactory::GetName()
{
	DEBUG_METHOD_CALL("ModuleFactory::GetName");

	return mText->GetObjectName();
}

String ModuleFactory::GetStatus()
{
	DEBUG_METHOD_CALL("ModuleFactory::GetStatus");

	String lReturn;

	lReturn = mModuleTTLCMOS->GetStatus();
	lReturn += mModuleAnalog->GetStatus();
	lReturn += mModuleHF->GetStatus();
	lReturn += mModuleNone->GetStatus();

	return lReturn;
}

void ModuleFactory::TriggerLampTestOff()
{
	DEBUG_METHOD_CALL("ModuleFactory::TriggerLampTestOff");

	mTriggerLampTestOff = true;
}

ModuleBase *ModuleFactory::GetSelectedModule()
{
	DEBUG_METHOD_CALL("ModuleFactory::GetSelectedModule");

	return mSelectedModule;
}

void ModuleFactory::I2ESelectModule(ModuleBase::eModuleCode iModuleCode)
{
	DEBUG_METHOD_CALL("ModuleFactory::_I2ESelectModule");

	// switch 1st all modules off, before a new one is switched on
	switch (iModuleCode)
	{
	case ModuleBase::eModuleCode::TModuleTTLCMOS:
		mModuleNone->I2ESelectModule(false);
		mModuleAnalog->I2ESelectModule(false);
		mModuleHF->I2ESelectModule(false);
		delay(10);
		mModuleTTLCMOS->I2ESelectModule(true);
		mSelectedModule = mModuleTTLCMOS;
		break;

	case ModuleBase::eModuleCode::TModuleAnalog:
		mModuleNone->I2ESelectModule(false);
		mModuleTTLCMOS->I2ESelectModule(false);
		mModuleHF->I2ESelectModule(false);
		delay(10);
		mModuleAnalog->I2ESelectModule(true);
		mSelectedModule = mModuleAnalog;
		break;

	case ModuleBase::eModuleCode::TModuleHF:
		mModuleNone->I2ESelectModule(false);
		mModuleTTLCMOS->I2ESelectModule(false);
		mModuleAnalog->I2ESelectModule(false);
		delay(10);
		mModuleHF->I2ESelectModule(true);
		mSelectedModule = mModuleHF;
		break;

	case ModuleBase::eModuleCode::TModuleNone:
		mModuleTTLCMOS->I2ESelectModule(false);
		mModuleAnalog->I2ESelectModule(false);
		mModuleHF->I2ESelectModule(false);
		delay(10);
		mModuleNone->I2ESelectModule(true);
		mSelectedModule = mModuleNone;
		break;
	}
}
