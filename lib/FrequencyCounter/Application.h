// Arduino Frequency Counter
// 31.10.2022
// Stefan Rau
// Main program

#pragma once
#ifndef _Application_h
#define _Application_h

#include <Arduino.h>
#include <Wire.h>

#include "Debug.h"
#include "Taskhandler.h"
#include "ModuleFactory.h"
#include "FrontPlate.h"
#include "Counter.h"
#include "LCDHandler.h"
#include "RemoteControl.h"
#include "ErrorHandler.h"

#define VERSION "V 1"
#define DEVICENAME "Frequenzzaehler 1"

/// <summary>
/// Local text class of the module
/// </summary>
class TextMain : public TextBase
{
public:
    TextMain(int iSettingsAddress);
    ~TextMain();

    String GetObjectName() override;
#ifndef DEBUG_APPLICATION
    String FreeMemory(int iFreeMemory);
#endif
    String ErrorInSetup();
    String ErrorInLoop();
};

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char *sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif // __arm__

class Application
{
public:
    // Hardware

    // Period detected
    const uint8_t cIDone = 6;
    // Input 0.5Hz counter
    const uint8_t cI0_5Hz = 7;
    // Reset counter input driver
    const uint8_t cOResetFF = 10;
    // Reset counter
    const uint8_t cOResetCounter = 11;
    // Reset period detection
    const uint8_t cONotResetPeriod = 12;
    // Reset 0.5Hz counter
    const uint8_t cOReset0_5Hz = 13;

    struct sInitializeSystem
    {
        I2CBase::sInitializeModule EEPROM = {-1, 0, 0x50};          // EEPROM is not used, I2C uses 0x50
        I2CBase::sInitializeModule Text = {0x00, 1, -1};            // EEPROM uses 0x00, I2C is not used
        I2CBase::sInitializeModule Counter = {-1, 0, 0x20};         // EEPROM is not used, I2C uses 2 addresses 0x20 .. 0x21
        I2CBase::sInitializeModule ModuleFactory = {0x02, 8, 0x22}; // EEPROM uses 4 addresses 0x02 .. 0x05, I2C uses 3 addresses 0x22 .. 0x25
        I2CBase::sInitializeModule LCDHandler = {0x06, 0, 0x26};    // EEPROM and I2C is used
        I2CBase::sInitializeModule FrontPlate = {0x07, 1, 0x27};    // EEPROM and I2C is used
        I2CBase::sInitializeModule ErrorLogger = {-1, 0, -1};       // EEPROM is not used
    } mInitializeSystem;

private:
    TextMain *mText = nullptr;      // Pointer to current text objekt of main
    bool mErrorPrinted;             // Signals than an error in the main loop is outputted
    bool mEventCountingInitialized; // Event counting shall be initialized only once after selected
    long mFreeMemory;

#if DEBUG_APPLICATION == 0
    RemoteControl *mRemoteControl = nullptr;
#define RemoteControlBufferSize 80
    char mRemoteControlBuffer[RemoteControlBufferSize];
#endif

    // Tasks
    Task *mLampTestTime = nullptr;
    Task *mMenuSwitchOfTime = nullptr;
    Task *mLCDRefreshCycleTime = nullptr;

    String mMeasurementValue = "";

    /// <summary>
    /// Constructor
    /// </summary>
    Application();
    ~Application();

public:
    /// <summary>
    /// Gets a singleton.
    /// </summary>
    /// <returns>Instance of this class</returns>
    static Application *GetInstance();

    /// <summary>
    /// Called by framework periodically
    /// </summary>
    void loop();

#if DEBUG_APPLICATION == 0
    /// <summary>
    /// Dispatches commands got from an serial input
    /// </summary>
    void DispatchSerial();
#endif

    /// <summary>
    /// Task that stops initialization phase: switches off LEDs, switches off initialization message on LCD
    /// </summary>
    static void TaskLampTestEnd();

    /// <summary>
    /// Task that switches off menu message on LCD after a menu button was pressed
    /// </summary>
    static void TaskMenuSwitchOff();

    /// <summary>
    /// Task that refreshs the LCD periodically
    /// </summary>
    static void TaskLCDRefresh();

    /// <summary>
    /// Get the really free RAM of the processor
    /// </summary>
    /// <returns>Free RAM in byte</returns>
    long GetFreeRAM();

    /// <summary>
    /// Resets the counters
    /// </summary>
    void ResetCounters();

    /// <summary>
    /// Resets pulse recognition
    /// </summary>
    void RestartPulsDetection();

    /// <summary>
    /// Restart 0.5Hz
    /// </summary>
    void RestartGateTimer();
};

#endif