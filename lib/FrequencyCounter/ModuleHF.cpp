// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 27.10.2021: Constructor requires structure - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau

#include "ModuleHF.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextModuleHF::TextModuleHF() : TextBase(-1)
{
    DebugInstantiation("New TextModuleHF");
}

TextModuleHF::~TextModuleHF()
{
}

String TextModuleHF::GetObjectName()
{
    switch (GetLanguage())
    {
        TextLangE("HF Module");
        TextLangD("HF Modul");
    }
}

/////////////////////////////////////////////////////////////

ModuleHF::ModuleHF(sInitializeModule iInitializeModule) : ModuleBase(iInitializeModule)
{
    DebugInstantiation("New ModuleHF: iInitializeModule[SettingsAddress, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.I2CAddress) + "]");

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
}

void ModuleHF::I2EActivate()
{
}

void ModuleHF::I2EDeactivate()
{
}

void ModuleHF::I2ESelectFunction()
{
    // Beim HF Modul gibt es keine Auswahlmöglichkeiten
}

String ModuleHF::GetName()
{
    return _mText->GetObjectName();
}

bool ModuleHF::IsPeriodMeasurementPossible()
{
    return false;
}

bool ModuleHF::IsEventCountingPossible()
{
    return false;
}

String ModuleHF::GetCurrentMenuEntry(int iMenuEntry)
{
    return "";
}

char ModuleHF::GetModuleCode()
{
    return ModuleBase::eModuleCode::TModuleHF;
}
