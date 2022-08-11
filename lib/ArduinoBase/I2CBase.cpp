// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// History
// 18.10.2021: 1st version - Stefan Rau
// 27.10.2021: Constructor requires structure - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau

#include "I2CBase.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextI2CBase::TextI2CBase() : TextBase(-1)
{
    DebugInstantiation("New TextI2CBase");
}

TextI2CBase::~TextI2CBase()
{
}

String TextI2CBase::GetObjectName()
{
    return "";
}

String TextI2CBase::ModuleInitialized()
{
    switch (GetLanguage())
    {
        TextLangE(" is initialized");
        TextLangD(" ist initialisiert");
    }
}

String TextI2CBase::ModuleNotInitialized()
{
    switch (GetLanguage())
    {
        TextLangE(" is not initialized");
        TextLangD(" ist nicht initialisiert");
    }
}

/////////////////////////////////////////////////////////////

// Module implementation

I2CBase::I2CBase(sInitializeModule iInitializeModule) : ProjectBase(iInitializeModule.SettingsAddress, iInitializeModule.NumberOfSettings)
{
    DebugInstantiation("New I2CBase: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

    _mText = new TextI2CBase();

    if (iInitializeModule.I2CAddress >= 0)
    {
        mI2CAddress = iInitializeModule.I2CAddress;
    }
}

I2CBase::~I2CBase()
{
}

String I2CBase::GetStatus()
{
    // Concatenates name of the module with text, if initialized or not
    String lReturn = GetName();
    lReturn += (mModuleIsInitialized) ? _mText->ModuleInitialized() + "\n" : _mText->ModuleNotInitialized() + "\n";
    return lReturn;
}

uint8_t I2CBase::Bool2State(bool iValue)
{
    return (iValue) ? HIGH : LOW;
}
