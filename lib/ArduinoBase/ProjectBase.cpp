// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// History
// 18.10.2021: 1st version - Stefan Rau
// 21.10.2021: Dispatcher has now chars as input - Stefan Rau
// 22.10.2021: global constants for dispatcher - Stefan Rau
// 04.11.2021: Verbose mode implemented - Stefan Rau
// 11.11.2021: Use separate EEPROM for Arduino Nano IoT - Stefan Rau
// 12.01.2022: extended by ARDUINO_NANO_RP2040_CONNECT - Stefan Rau
// 16.03.2022: ARDUINO_NANO_RP2040_CONNECT removed - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 09.08.2022: Switch to ARDUINO NANO IOT due to memory issues - Stefan Rau
// 09.08.2022: add ARDUINO NANO 33 BLE

#include "ProjectBase.h"

#ifndef _DebugApplication
static bool _mVerboseMode = false; // returns results of dispatcher - true: in details, false: as single letter code
#endif

// // Text definitions

// /// <summary>
// /// There is no new EEPROM address required
// /// </summary>
// TextProjectBase::TextProjectBase() : TextBase(-1)
// {
//     DebugInstantiation("New TextProjectBase");
// }

// TextProjectBase::~TextProjectBase()
// {
// }

// String TextProjectBase::GetObjectName()
// {
//     switch (GetLanguage())
//     {
//         TextLangE("Project base class");
//         TextLangD("Projekt Basis Klasse");
//     }
// }

// String TextProjectBase::InconsistentParameters()
// {
//     switch (GetLanguage())
//     {
//         TextLangE("Implementation error: parameter iNumberOfSettings must be set to a value > 0");
//         TextLangD("Implementierungsfehler: Parameter iNumberOfSettings muss größer 0 sein");
//     }
// }

/////////////////////////////////////////////////////////////

static short gI2CAddressGlobalEEPROM = -1;
static I2C_eeprom *gI2CGlobalEEPROM = nullptr;
static bool gGlobalEEPROMIsInitialized = false;

ProjectBase::ProjectBase(int iSettingsAddress, int iNumberOfSettings)
{
    DebugInstantiation("New ProjectBase: iInitializeModule[SettingsAddress, NumberOfSettings]=[" + String(iSettingsAddress) + ", " + String(iNumberOfSettings) + "]");

    // _mText = new TextProjectBase();

    // try only once to instantiate the EEPROM: with the 1st call of this constructor
    if (!gGlobalEEPROMIsInitialized)
    {
        if ((gI2CGlobalEEPROM == nullptr) && (gI2CAddressGlobalEEPROM >= 0))
        {
            gI2CGlobalEEPROM = new I2C_eeprom(gI2CAddressGlobalEEPROM, I2C_DEVICESIZE_24LC256);
            if (gI2CGlobalEEPROM->begin())
            {
                if (!gI2CGlobalEEPROM->isConnected())
                {
                    // if the EEPROM is not connected, destroy the class
                    gI2CGlobalEEPROM = nullptr;
                }
            }
            else
            {
                // if the EEPROM is not connected, destroy the class
                gI2CGlobalEEPROM = nullptr;
            }
        }

        if (gI2CGlobalEEPROM != nullptr)
        {
            DebugPrint("EEPROM is initialized");
        }
        else
        {
            DebugPrint("EEPROM is not initialized");
        }

        gGlobalEEPROMIsInitialized = true;
    }

    // Stores settings address of the object only if address is valid
    if (iSettingsAddress >= 0)
    {
        _mSettingAdddress = iSettingsAddress;

        if (iNumberOfSettings > 0)
        {
            _mNumberOfSettings = iNumberOfSettings;
        }
        else
        {
            // ErrorPrint(Error::eSeverity::TFatal, _mText->InconsistentParameters());
            DebugPrint("Implementation error: parameter iNumberOfSettings must be set to a value > 0");
            return;
        }
    }
}

ProjectBase::ProjectBase()
{
    DebugInstantiation("New ProjectBase");
}

ProjectBase::~ProjectBase()
{
}

void ProjectBase::SetI2CAddressGlobalEEPROM(short iI2CAddress)
{
    gI2CAddressGlobalEEPROM = iI2CAddress;
}

I2C_eeprom *ProjectBase::GetI2CGlobalEEPROM()
{
    return gI2CGlobalEEPROM;
}

#ifndef _DebugApplication
void ProjectBase::SetVerboseMode(bool iVerboseMode)
{
    _mVerboseMode = iVerboseMode;
}

bool ProjectBase::GetVerboseMode()
{
    return _mVerboseMode;
}
#endif

void ProjectBase::SetSetting(int iNumberOfSetting, char iValue)
{
    // Settings address and value must be valid
    if ((_mSettingAdddress >= 0) && (iNumberOfSetting > 0) && (iNumberOfSetting <= _mNumberOfSettings) && (iValue != cNullSetting))
    {
#ifdef ARDUINO_AVR_NANO_EVERY
        EEPROM.update(_mSettingAdddress + iNumberOfSetting - 1, iValue);
#endif
#if defined(ARDUINO_SAMD_NANO_33_IOT) or defined(ARDUINO_ARDUINO_NANO33BLE)
        if (gI2CGlobalEEPROM != nullptr)
        {
            gI2CGlobalEEPROM->updateByte(_mSettingAdddress + iNumberOfSetting - 1, iValue);
        }
#endif
    }
    DebugPrint("Set Setting: " + String(_mSettingAdddress) + ", " + String((int)iValue));
}

char ProjectBase::GetSetting(int iNumberOfSetting)
{
    char lSetting = cNullSetting;

    if ((_mSettingAdddress >= 0) && (iNumberOfSetting > 0) && (iNumberOfSetting <= _mNumberOfSettings))
    {
#ifdef ARDUINO_AVR_NANO_EVERY
        lSetting = EEPROM.read(_mSettingAdddress + iNumberOfSetting - 1);
#endif
#if defined(ARDUINO_SAMD_NANO_33_IOT) or defined(ARDUINO_ARDUINO_NANO33BLE)
        if (gI2CGlobalEEPROM != nullptr)
        {
            lSetting = gI2CGlobalEEPROM->readByte(_mSettingAdddress + iNumberOfSetting - 1);
        }
#endif
    }

    DebugPrint("Get Setting: " + String(_mSettingAdddress) + ", " + String((int)lSetting));
    return lSetting;
}
