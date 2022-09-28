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
// 09.08.2022: add ARDUINO NANO 33 BLE - Stefan Rau
// 26.09.2022: EXTERNAL_EEPROM defined in platform.ini - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau

#include "ProjectBase.h"

#ifndef DEBUG_APPLICATION
static bool _gVerboseMode = false; // returns results of dispatcher - true: in details, false: as single letter code
#endif

#ifdef EXTERNAL_EEPROM
static short gI2CAddressGlobalEEPROM = -1;
static I2C_eeprom *gI2CGlobalEEPROM = nullptr;
static bool gGlobalEEPROMIsInitialized = false;
#endif

ProjectBase::ProjectBase(int iSettingsAddress, int iNumberOfSettings)
{
    DebugInstantiation("New ProjectBase: iInitializeModule[SettingsAddress, NumberOfSettings]=[" + String(iSettingsAddress) + ", " + String(iNumberOfSettings) + "]");

#ifdef EXTERNAL_EEPROM
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
#endif

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

#ifdef EXTERNAL_EEPROM
void ProjectBase::SetI2CAddressGlobalEEPROM(short iI2CAddress)
{
    gI2CAddressGlobalEEPROM = iI2CAddress;
}

I2C_eeprom *ProjectBase::GetI2CGlobalEEPROM()
{
    return gI2CGlobalEEPROM;
}
#endif

#ifndef DEBUG_APPLICATION
void ProjectBase::SetVerboseMode(bool iVerboseMode)
{
    _gVerboseMode = iVerboseMode;
}

bool ProjectBase::GetVerboseMode()
{
    return _gVerboseMode;
}
#endif

//#ifndef NO_EEPROM
void ProjectBase::SetSetting(int iSettingNumber, char iValue)
{
#ifndef NO_EEPROM
    // Settings address and value must be valid
    if ((_mSettingAdddress >= 0) && (iSettingNumber > 0) && (iSettingNumber <= _mNumberOfSettings) && (iValue != cNullSetting))
    {
#ifdef ARDUINO_AVR_NANO_EVERY
        EEPROM.update(_mSettingAdddress + iSettingNumber - 1, iValue);
#endif
#ifdef EXTERNAL_EEPROM
#if defined(ARDUINO_SAMD_NANO_33_IOT) or defined(ARDUINO_ARDUINO_NANO33BLE)
        if (gI2CGlobalEEPROM != nullptr)
        {
            gI2CGlobalEEPROM->updateByte(_mSettingAdddress + iSettingNumber - 1, iValue);
        }
#endif
#endif
    }
    DebugPrint("Set Setting: " + String(_mSettingAdddress) + ", " + String(iValue));
#endif
}

char ProjectBase::GetSetting(int iSettingNumber)
{
    char lSetting = cNullSetting;

#ifndef NO_EEPROM
    if ((_mSettingAdddress >= 0) && (iSettingNumber > 0) && (iSettingNumber <= _mNumberOfSettings))
    {
#ifdef ARDUINO_AVR_NANO_EVERY
        lSetting = EEPROM.read(_mSettingAdddress + iSettingNumber - 1);
#endif
#ifdef EXTERNAL_EEPROM
#if defined(ARDUINO_SAMD_NANO_33_IOT) or defined(ARDUINO_ARDUINO_NANO33BLE)
        if (gI2CGlobalEEPROM != nullptr)
        {
            lSetting = gI2CGlobalEEPROM->readByte(_mSettingAdddress + iSettingNumber - 1);
        }
#endif
#endif
    }

    DebugPrint("Get Setting: " + String(_mSettingAdddress) + ", " + String(lSetting));

#endif

    return lSetting;
}
