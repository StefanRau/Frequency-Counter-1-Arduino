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

#include "ModuleFactory.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleFactory::TextModuleFactory() : TextBase(-1)
{
	DebugInstantiation("New TextModuleFactory");
}

TextModuleFactory::~TextModuleFactory()
{
}

String TextModuleFactory::GetObjectName()
{
	switch (GetLanguage())
	{
		TextLangE("Module factory");
		TextLangD("Module Factory");
	}
}

String TextModuleFactory::Unknown()
{
	switch (GetLanguage())
	{
		TextLangE("Unknown function");
		TextLangD("Unbekannte Funktion");
	}
}

/////////////////////////////////////////////////////////////

ModuleFactory::ModuleFactory(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
	DebugInstantiation("New ModuleFactory: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

	sInitializeModule lInitializeModule = iInitializeModule;
	lInitializeModule.NumberOfSettings = 2;

	_mText = new TextModuleFactory();

	// Initialize the hardware of all modules
	// 100 MHz TTL / CMOS
	_mModuleTTLCMOS = new ModuleTTLCMOS(lInitializeModule);

	// 100 MHz analog
	lInitializeModule.I2CAddress += 1;
	lInitializeModule.SettingsAddress += 2;
	_mModuleAnalog = new ModuleAnalog(lInitializeModule);

	// 10GHz
	lInitializeModule.I2CAddress += 1;
	lInitializeModule.SettingsAddress += 2;
	_mModuleHF = new ModuleHF(lInitializeModule);

	// Dummy module
	lInitializeModule.I2CAddress = -1;
	lInitializeModule.SettingsAddress += 2;
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
	DebugPrint("Module factory is initialized");
}

ModuleFactory ::~ModuleFactory()
{
}

void ModuleFactory::loop()
{

	// check the state of the key of all modules
	if (_mModuleTTLCMOS->I2EIsKeySelected())
	{
		DebugPrint("Module TTL/CMOS key pressed");
		if (_mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleTTLCMOS)
		{
			_I2ESelectModule(ModuleBase::eModuleCode::TModuleTTLCMOS);
		}
	};

	if (_mModuleAnalog->I2EIsKeySelected())
	{
		DebugPrint("Module Analog key pressed");
		if (_mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleAnalog)
		{
			_I2ESelectModule(ModuleBase::eModuleCode::TModuleAnalog);
		}
	};

	if (_mModuleHF->I2EIsKeySelected())
	{
		DebugPrint("Module HF key pressed");
		if (_mSelectedModule->GetModuleCode() != ModuleBase::eModuleCode::TModuleHF)
		{
			_I2ESelectModule(ModuleBase::eModuleCode::TModuleHF);
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

#ifndef _DebugApplication
String ModuleFactory::Dispatch(char iModuleIdentifyer, char iParameter)
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
	_mModuleTTLCMOS->I2ESwitchLamp(true); // 100 MHz TTL / CMOS
	_mModuleAnalog->I2ESwitchLamp(true);  // 100 MHz Analog
	_mModuleHF->I2ESwitchLamp(true);	  // 10GHz
}

String ModuleFactory::GetName()
{
	return _mText->GetObjectName();
}

String ModuleFactory::GetStatus()
{
	String lReturn;

	lReturn = _mModuleTTLCMOS->GetStatus();
	lReturn += _mModuleAnalog->GetStatus();
	lReturn += _mModuleHF->GetStatus();
	lReturn += _mModuleNone->GetStatus();

	return lReturn;
}

void ModuleFactory::TriggerLampTestOff()
{
	_mTriggerLampTestOff = true;
}

ModuleBase *ModuleFactory::GetSelectedModule()
{
	return _mSelectedModule;
}

void ModuleFactory::_I2ESelectModule(char iModuleCode)
{
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
