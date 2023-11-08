// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 27.10.2021: Constructor requires structure - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 21.12.2022: Extend destructor - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "ModuleHF.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleHF::TextModuleHF() : TextBase()
{
    DebugInstantiation("TextModuleHF");
}

TextModuleHF::~TextModuleHF()
{
    DebugDestroy("TextModuleHF");
}

String TextModuleHF::GetObjectName()
{
    DebugMethodCalls("TextModuleHF::GetObjectName");

    switch (GetLanguage())
    {
        TextLangE("HF Module");
        TextLangD("HF Modul");
    }
}

/////////////////////////////////////////////////////////////

ModuleHF::ModuleHF(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
    DebugInstantiation("ModuleHF: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

    _mText = new TextModuleHF();
    mLastMenuEntryNumber = _cNumberOfMenuEntries;

    // Initialize hardware
    if (I2EInitialize())
    {
        // initialize specific part
    }
}

ModuleHF::~ModuleHF()
{
    DebugDestroy("ModuleHF");
}

void ModuleHF::I2EActivate()
{
    DebugMethodCalls("ModuleHF::I2EActivate");
}

void ModuleHF::I2EDeactivate()
{
    DebugMethodCalls("ModuleHF::I2EDeactivate");
}

void ModuleHF::I2ESelectFunction()
{
    DebugMethodCalls("ModuleHF::I2ESelectFunction");

    // Beim HF Modul gibt es keine Auswahlmöglichkeiten
}

String ModuleHF::GetName()
{
    DebugMethodCalls("ModuleHF::GetName");

    return _mText->GetObjectName();
}

bool ModuleHF::IsPeriodMeasurementPossible()
{
    DebugMethodCalls("ModuleHF::IsPeriodMeasurementPossible");

    return false;
}

bool ModuleHF::IsEventCountingPossible()
{
    DebugMethodCalls("ModuleHF::IsEventCountingPossible");

    return false;
}

String ModuleHF::GetCurrentMenuEntry(int iMenuEntry)
{
    DebugMethodCalls("ModuleHF::GetCurrentMenuEntry");

    return "";
}

ModuleBase::eModuleCode ModuleHF::GetModuleCode()
{
    DebugMethodCalls("ModuleHF::GetModuleCode");

    return ModuleBase::eModuleCode::TModuleHF;
}
