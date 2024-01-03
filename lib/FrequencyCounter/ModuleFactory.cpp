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

	_mText = new TextModuleFactory();

	// Initialize the hardware of all modules
	// 100 MHz TTL / CMOS
	_mModuleTTLCMOS = new ModuleTTLCMOS(lInitializeModule);

	// 100 MHz analog
	lInitializeModule.I2CAddress += 1;
	lInitializeModule.SettingsAddress += iInitializeModule.NumberOfSettings;
	_mModuleAnalog = new ModuleAnalog(lInitializeModule);

	// 10GHz
	lInitializeModule.I2CAddress += 1;
	lInitializeModule.SettingsAddress += iInitializeModule.NumberOfSettings;
	_mModuleHF = new ModuleHF(lInitializeModule);

	// Dummy module
	lInitializeModule.I2CAddress = -1;
	lInitializeModule.SettingsAddress += iInitializeModule.NumberOfSettings;
	_mModuleNone = new ModuleNone(lInitializeModule);

	if (_mModuleTTLCMOS->IsModuleInitialized())
	{
		_mSelectedModule = _mModuleTTLCMOS;
	}
	else
	{
		if (_mModuleAnalog->IsModuleInitialized())
		{
			_mSelectedModule = _mModuleAnalog;
		}
		else
		{
			if (_mModuleHF->IsModuleInitialized())
			{
				_mSelectedModule = _mModuleHF;
			}
			else
			{
				_mSelectedModule = _mModuleNone;
			}
		}
	}

	_mSelectedModule->I2ESelectFunction();
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
	if (_mModuleTTLCMOS->I2EIsKeySelected())
	{
		if (_mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleTTLCMOS)
		{
			DEBUG_PRINT_LN("\nModule TTL/CMOS key pressed");
			_I2ESelectModule(ModuleBase::eModuleCode::TModuleTTLCMOS);
			delay(100);
		}
	};

	if (_mModuleAnalog->I2EIsKeySelected())
	{
		if (_mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleAnalog)
		{
			DEBUG_PRINT_LN("\nModule Analog key pressed");
			_I2ESelectModule(ModuleBase::eModuleCode::TModuleAnalog);
			delay(100);
		}
	};

	if (_mModuleHF->I2EIsKeySelected())
	{
		if (_mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleHF)
		{
			DEBUG_PRINT_LN("\nModule HF key pressed");
			_I2ESelectModule(ModuleBase::eModuleCode::TModuleHF);
			delay(100);
		}
	};

	if (_mTriggerLampTestOff)
	{
		_mModuleTTLCMOS->I2ESwitchLamp(false); // 100 MHz TTL / CMOS
		_mModuleAnalog->I2ESwitchLamp(false);  // 100 MHz Analog
		_mModuleHF->I2ESwitchLamp(false);	   // 10GHz
		_mTriggerLampTestOff = false;
		_I2ESelectModule(_mSelectedModule->GetModuleCode());
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

	_mModuleTTLCMOS->I2ESwitchLamp(true); // 100 MHz TTL / CMOS
	_mModuleAnalog->I2ESwitchLamp(true);  // 100 MHz Analog
	_mModuleHF->I2ESwitchLamp(true);	  // 10GHz
}

String ModuleFactory::GetName()
{
	DEBUG_METHOD_CALL("ModuleFactory::GetName");

	return _mText->GetObjectName();
}

String ModuleFactory::GetStatus()
{
	DEBUG_METHOD_CALL("ModuleFactory::GetStatus");

	String lReturn;

	lReturn = _mModuleTTLCMOS->GetStatus();
	lReturn += _mModuleAnalog->GetStatus();
	lReturn += _mModuleHF->GetStatus();
	lReturn += _mModuleNone->GetStatus();

	return lReturn;
}

void ModuleFactory::TriggerLampTestOff()
{
	DEBUG_METHOD_CALL("ModuleFactory::TriggerLampTestOff");

	_mTriggerLampTestOff = true;
}

ModuleBase *ModuleFactory::GetSelectedModule()
{
	DEBUG_METHOD_CALL("ModuleFactory::GetSelectedModule");

	return _mSelectedModule;
}

void ModuleFactory::_I2ESelectModule(char iModuleCode)
{
	DEBUG_METHOD_CALL("ModuleFactory::_I2ESelectModule");

	// switch 1st all modules off, before a new one is switched on
	switch (iModuleCode)
	{
	case ModuleBase::eModuleCode::TModuleTTLCMOS:
		_mModuleNone->I2ESelectModule(false);
		_mModuleAnalog->I2ESelectModule(false);
		_mModuleHF->I2ESelectModule(false);
		delay(10);
		_mModuleTTLCMOS->I2ESelectModule(true);
		_mSelectedModule = _mModuleTTLCMOS;
		break;

	case ModuleBase::eModuleCode::TModuleAnalog:
		_mModuleNone->I2ESelectModule(false);
		_mModuleTTLCMOS->I2ESelectModule(false);
		_mModuleHF->I2ESelectModule(false);
		delay(10);
		_mModuleAnalog->I2ESelectModule(true);
		_mSelectedModule = _mModuleAnalog;
		break;

	case ModuleBase::eModuleCode::TModuleHF:
		_mModuleNone->I2ESelectModule(false);
		_mModuleTTLCMOS->I2ESelectModule(false);
		_mModuleAnalog->I2ESelectModule(false);
		delay(10);
		_mModuleHF->I2ESelectModule(true);
		_mSelectedModule = _mModuleHF;
		break;

	case ModuleBase::eModuleCode::TModuleNone:
		_mModuleTTLCMOS->I2ESelectModule(false);
		_mModuleAnalog->I2ESelectModule(false);
		_mModuleHF->I2ESelectModule(false);
		delay(10);
		_mModuleNone->I2ESelectModule(true);
		_mSelectedModule = _mModuleNone;
		break;
	}
}
