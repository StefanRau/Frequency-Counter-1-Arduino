// Arduino Frequency Counter
// 31.10.2022
// Stefan Rau
// History
// 31.10.2022: 1st version - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 20.01.2023: Improve debug handling - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "Application.h"

static Application *gInstance = nullptr;

// Module
static Counter *gCounter = nullptr;
static LCDHandler *gLCDHandler = nullptr;
static FrontPlate *gFrontPlate = nullptr;
static ModuleFactory *gModuleFactory = nullptr;

static bool gIsInitialized = false;
static bool gReadEventCounter;

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextMain::TextMain() : TextBase()
{
    DEBUG_INSTANTIATION("TextMain");
}

TextMain::~TextMain()
{
    DEBUG_DESTROY("TextMain");
}

String TextMain::GetObjectName()
{
    DEBUG_METHOD_CALL("TextMain::GetObjectName");

    return "Main";
}

#if DEBUG_APPLICATION == 0
String TextMain::FreeMemory(int iFreeMemory)
{
    DEBUG_METHOD_CALL("TextMain::FreeMemory");

    switch (GetLanguage())
    {
        TextLangE("Free memory: " + String(iFreeMemory) + " byte");
        TextLangD("Freier Speicher: " + String(iFreeMemory) + " Byte");
    }
}
#endif

String TextMain::ErrorInSetup()
{
    DEBUG_METHOD_CALL("TextMain::ErrorInSetup");

    switch (GetLanguage())
    {
        TextLangE("Error in setup");
        TextLangD("Fehler im setup");
    }
}

String TextMain::ErrorInLoop()
{
    DEBUG_METHOD_CALL("TextMain::ErrorInLoop");

    switch (GetLanguage())
    {
        TextLangE("Error in loop");
        TextLangD("Fehler in loop");
    }
}

/////////////////////////////////////////////////////////////

Application::Application()
{
    DEBUG_INSTANTIATION("Application");

    // Set clock frequency of I2C to 100kHz
    Wire.begin();
    delay(10);

#ifdef EXTERNAL_EEPROM
    ProjectBase::SetI2CAddressGlobalEEPROM(mInitializeSystem.EEPROM.I2CAddress);
#endif
    mTextWrapper = new TextWrapper(mInitializeSystem.Text.SettingsAddress);
    mText = new TextMain();

    //// CPU board

    // Input 0.5 Hz
    pinMode(cI0_5Hz, INPUT);
    // Output Reset 0.5 Hz counter
    pinMode(cOReset0_5Hz, OUTPUT);
    // Input period measurement done
    pinMode(cIDone, INPUT);
    // Output reset period measurement
    pinMode(cONotResetPeriod, OUTPUT);
    // Output reset counter
    pinMode(cOResetCounter, OUTPUT);
    // Output reset input driver of 0.5 Hz counter
    pinMode(cOResetFF, OUTPUT);

    // LCD
    if (!ERROR_DETECTED())
    {
        gLCDHandler = LCDHandler::GetInstance(mInitializeSystem.LCDHandler);
    }

    // Reset input modules and start lamp test
    if (!ERROR_DETECTED())
    {
        gModuleFactory = ModuleFactory::GetInstance(mInitializeSystem.ModuleFactory);
        gModuleFactory->I2ELampTestOn();
    }

    // Initialize main counter
    if (!ERROR_DETECTED())
    {
        gCounter = Counter::GetInstance(mInitializeSystem.Counter);
        gCounter->I2ESetFunctionCode(Counter::eFunctionCode::TFrequency);
    }

    // Initialize front plate
    if (!ERROR_DETECTED())
    {
        // Reset selection and start
        gFrontPlate = FrontPlate::GetInstance(mInitializeSystem.FrontPlate, gLCDHandler, gModuleFactory, gCounter);
    }

    // Initialize task management and all tasks
    DEBUG_PRINT_LN("Initialize tasks");

    // Task for lamp test end
    mLampTestTime = Task::GetNewTask(Task::eTaskType::TOneTime, 20, Application::TaskLampTestEnd);

    // Task timer for switching off the menue in LCD
    mMenuSwitchOfTime = Task::GetNewTask(Task::eTaskType::TTriggerOneTime, 20, Application::TaskMenuSwitchOff);
    mMenuSwitchOfTime->DefinePrevious(mLampTestTime);

    // Task timer LCD refresh
    mLCDRefreshCycleTime = Task::GetNewTask(Task::eTaskType::TFollowUpCyclic, 10, Application::TaskLCDRefresh);
    mLCDRefreshCycleTime->DefinePrevious(mLampTestTime);

    // Initialize task handler
    TaskHandler::GetInstance()->SetCycleTimeInMs(100);

    // Reset
    ResetCounters();
    RestartGateTimer();
    RestartPulsDetection();

#if DEBUG_APPLICATION == 0
    // Initialize remote control
    if (!ErrorDetected())
    {
        mRemoteControl = RemoteControl::GetInstance(mRemoteControlBuffer, 80);
    }
#endif

    // Output potential errors
    if (ERROR_DETECTED())
    {
        if (gLCDHandler != nullptr)
        {
            gLCDHandler->SetErrorText(mText->ErrorInSetup());
        }
        DEBUG_PRINT_LN("Error in setup");
    }

    mFreeMemory = 0;
    DEBUG_PRINT_LN("End setup");
}

Application::~Application()
{
    DEBUG_DESTROY("Application");
}

Application *Application::GetInstance()
{
    DEBUG_METHOD_CALL("Application::GetInstance");

    gInstance = (gInstance == nullptr) ? new Application() : gInstance;
    return gInstance;
}

void Application::loop()
{
    // DEBUG_METHOD_CALL("Application::loop");

    // String lCommand = "";
    long lFreeMemory;

    lFreeMemory = GetFreeRAM();
    if (mFreeMemory != lFreeMemory)
    {
        mFreeMemory = lFreeMemory;
        DEBUG_PRINT_LN("Free Memory: " + String(mFreeMemory));
    }

    if (gLCDHandler != nullptr)
    {
        gLCDHandler->loop(); // LCD must be called before the hardware is initialized to show potential errors
    }

    if (!gIsInitialized)
    {
        return;
    }

#if DEBUG_APPLICATION == 0
    DispatchSerial();
#endif

    // In case of an error, the program stops here
    if (ErrorHandler::GetInstance()->ContainsErrors())
    {
        if (!mErrorPrinted)
        {
            gLCDHandler->SetErrorText(mText->ErrorInLoop());
            DEBUG_PRINT_LN("Error in runtime => processing stopped");
            mErrorPrinted = true;
        }
        return;
    }

    DEBUG_LOOP();
    gFrontPlate->loop();
    gModuleFactory->loop();
    gCounter->loop();

    // Reset I2C after an error was detected
    if (Wire.getWriteError() != 0)
    {
        Wire.end();
        Wire.begin();
        DEBUG_PRINT_LN("Reset I2C");
    }

    // Get current menu item if a new one was selected
    if (gFrontPlate->IsNewMenuSelected())
    {
        DEBUG_PRINT_LN("Trigger TaskMenuSwitchOff");
        mMenuSwitchOfTime->Restart();
        ModuleBase *lModuleBase = gModuleFactory->GetSelectedModule();
        gLCDHandler->TriggerMenuSelectedFunction(lModuleBase->GetCurrentMenuEntry(-1), lModuleBase->GetCurrentMenuEntryNumber(), lModuleBase->GetLastMenuEntryNumber());
        DEBUG_PRINT_LN("Selected menu entry: " + lModuleBase->GetCurrentMenuEntry(-1));
    }

    // Reset counter if a new function is selected
    if (gFrontPlate->IsNewFunctionSelected())
    {
        ResetCounters();
        mMeasurementValue = gCounter->I2EGetCounterValue();
        RestartPulsDetection();
        RestartGateTimer();
        mEventCountingInitialized = false;
        gReadEventCounter = false;
    }

    if (gCounter->GetFunctionCode() == Counter::eFunctionCode::TFrequency)
    {
        // When frequency is selected
        if (digitalRead(cI0_5Hz) == LOW) // check if 10.000.000 pulses were counted
        {
            // DebugPrint("0.5 Hz signal detected");
            //  Wait a bit until the value is read
            //  => the counters are connected in a chain, it may happen some micro seconds until the last pulse reaches the last counter
            delayMicroseconds(10);
            mMeasurementValue = gCounter->I2EGetCounterValue();

            digitalWrite(cOResetCounter, HIGH);
            delayMicroseconds(10);

            digitalWrite(cOResetFF, HIGH);
            delayMicroseconds(10);

            digitalWrite(cOReset0_5Hz, HIGH);
            delayMicroseconds(10);

            digitalWrite(cOReset0_5Hz, LOW);
            delayMicroseconds(10);

            digitalWrite(cOResetCounter, LOW);
            delayMicroseconds(10);

            digitalWrite(cOResetFF, LOW);
            delayMicroseconds(10);
        }
    }
    else if (gCounter->GetFunctionCode() == Counter::eFunctionCode::TEventCounting)
    {
        // Event counting used frequency counter input, but does not use 0.5Hz => that is set permanently to 1
        // DebugPrint("Count events");
        if (!mEventCountingInitialized)
        {
            digitalWrite(cOReset0_5Hz, HIGH);
            delayMicroseconds(10);
            digitalWrite(cOResetFF, LOW);
            delayMicroseconds(10);
            mEventCountingInitialized = true;
        }
        if (gReadEventCounter)
        {
            mMeasurementValue = gCounter->I2EGetCounterValue();
            gReadEventCounter = false;
        }
    }
    else
    {
        // Pulse length
        if (digitalRead(cIDone) == HIGH) // check if pulse end is detected
        {
            // DebugPrint("Trigger detected");
            //   Wait a bit until the value is read
            delayMicroseconds(100);
            mMeasurementValue = gCounter->I2EGetCounterValue();
            ResetCounters();
            RestartPulsDetection();
        }
    }

    gLCDHandler->SetMeasurementValue(mMeasurementValue);
}

#if DEBUG_APPLICATION == 0
void Application::DispatchSerial()
{
    String lCommand;
    int lSeparatorIndex;
    char lModule;
    char lParameter;
    String lReturn = "";

    // dispatch the different modules
    if (mRemoteControl->Available())
    {
        lCommand = String(mRemoteControlBuffer);
        mRemoteControl->Read();
        if (lCommand != "")
        {
            lSeparatorIndex = lCommand.indexOf(':');

            if (lSeparatorIndex == 1)
            {
                lModule = lCommand[0];
                lParameter = lCommand[2];

                switch (lModule)
                {
                case 'S':

                    switch (lParameter)
                    {

                    case 'N':
                        // Reads the name of the device
                        lReturn = String(DEVICENAME);
                        break;

                    case 'V':
                        // Reads version and lists all hardware components and their state
                        lReturn = String(VERSION) + ", compiled at: " + String(__DATE__) + "\n";
                        lReturn += ErrorHandler::GetInstance()->GetStatus();
                        lReturn += gLCDHandler->GetStatus();
                        lReturn += gFrontPlate->GetStatus();
                        lReturn += gCounter->GetStatus();
                        lReturn += gModuleFactory->GetStatus();
                        lReturn += mText->FreeMemory(GetFreeRAM());
                        break;

                    case 'v':
                        // verbose mode
                        ProjectBase::SetVerboseMode(true);
                        lReturn = String(lParameter);
                        break;

                    case 's':
                        // short mode
                        ProjectBase::SetVerboseMode(false);
                        lReturn = String(lParameter);
                        break;
                    }
                    break;

                case 'D':

                    // Reads the current measurement value
                    lReturn = mMeasurementValue;
                    break;
                }

#ifdef EXTERNAL_EEPROM
                if (lReturn == "")
                {
                    // Manage error handling
                    // lModule = 'E'	: Code for this class, if controlled remotely
                    // lParameter = 'F' : Formatting EEPROM
                    // lParameter = '0' : Reset Log Pointer
                    // lParameter = 'R' : Read log
                    // lParameter = 'S' : Read log size
                    lReturn = ErrorHandler::GetInstance()->DispatchSerial(lModule, lParameter);
                }
#endif

                if (lReturn == "")
                {
                    // Select or get the current input module
                    // lModule = 'M'	: Code for this class, if controlled remotely
                    // lParameter = 'T' : 100 MHz TTL / CMOS module
                    // lParameter = 'A' : 100 MHz analog module
                    // lParameter = 'H' : 10GHz module
                    // lParameter = 'N' : no module - dummymodule for test purposes and fallback if no module is installed
                    // lParameter = '*' : codes of all installed modules "TAHN" - returns comma separated, readable text in verbose mode
                    // lParameter = '?' : returns the code of the selected module: 'T', 'A', 'H' or 'N' - returns readable text in verbose mode
                    lReturn = gModuleFactory->DispatchSerial(lModule, lParameter);
                }

                if (lReturn == "")
                {
                    // Select / display function
                    // lModule = 'F'	: Code for this class, if controlled remotely
                    // lParameter = 'f' : Frequenz
                    // lParameter = 'P' : Duration of positive level
                    // lParameter = 'N' : Duration of negative level
                    // lParameter = 'p' : Period triggered by positive edge
                    // lParameter = 'n' : Period triggered by negative edge
                    // lParameter = 'C' : Event counting
                    // lParameter = '*' : codes of all functions supported by the module "fnpNP" - returns comma separated, readable text in verbose mode
                    // lParameter = '?' : returns the code of the selected function: 'f', 'n', 'p', 'N', 'P', 'C' or '-' (if no function is selected)

                    // Select / display menue entry
                    // lModule = 'K'
                    // lParameter = '0' .. '9'	: Number of the menu item to select
                    // lParameter = '*' 		: codes of all menu items "123..." - returns comma separated, readable text in verbose mode
                    // lParameter = '?'			: returns the number of the currently selected menu item - returK:0ns readable text in verbose mode
                    lReturn = gFrontPlate->DispatchSerial(lModule, lParameter);
                }

                if (lReturn == "")
                {
                    // Select or get the current language
                    // lModule = 'L'			: Code for this class, if controlled remotely
                    // lParameter = 'D', 'E'	: Select language
                    // lParameter = '*'			: Lists all installed language as string of language codes - todo: im verbose mode komagetrennte Texte
                    // lParameter = '?'			: Shows the current language code: 'D', 'E' - todo: im verbose mode den Klartext
                    lReturn = mTextWrapper->DispatchSerial(lModule, lParameter);
                    if ((lReturn != "") && (lParameter != '*') && (lParameter != '?'))
                    {
                        gFrontPlate->TriggerLampTestOff(); // Reload displayed texts
                    }
                }

                if (lReturn != "")
                {
                    Serial.println(lReturn + "#");
                }
            }
        }
    }
}
#endif

void Application::TaskLampTestEnd()
{
    DEBUG_PRINT_FROM_TASK("TaskLampTestEnd\n");

    // Switch off initialization message of LCD
    if (gLCDHandler != nullptr)
    {
        gLCDHandler->TriggerShowCounter();
    }

    // End lamp test at front plate and get stored function
    if (gFrontPlate != nullptr)
    {
        gFrontPlate->TriggerLampTestOff();
        DEBUG_PRINT_FROM_TASK("Initial function: " + gCounter->GetSelectedFunctionName() + "\n");
    }

    // End lamp test at all modules
    if (gModuleFactory != nullptr)
    {
        gModuleFactory->TriggerLampTestOff();
        DEBUG_PRINT_FROM_TASK("Initial module: " + gModuleFactory->GetSelectedModule()->GetName() + "\n");
        DEBUG_PRINT_FROM_TASK("Initial menu entry: " + gModuleFactory->GetSelectedModule()->GetCurrentMenuEntry(-1) + "\n");
    }

    gIsInitialized = true;
}

void Application::TaskMenuSwitchOff()
{
    // Switch off menu message on display
    // DEBUG_PRINT_FROM_TASK("TaskMenuSwitchOff\n");
    if (gLCDHandler != nullptr)
    {
        gLCDHandler->TriggerShowCounter();
    }
}

void Application::TaskLCDRefresh()
{
    // DEBUG_PRINT_FROM_TASK("LCDRefresh");
    gReadEventCounter = true;
    if (gLCDHandler != nullptr)
    {
        gLCDHandler->TriggerShowRefresh();
    }
}

long Application::GetFreeRAM()
{
    DEBUG_METHOD_CALL("Application::GetFreeRAM");

    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char *>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
    return 0;
}

void Application::ResetCounters()
{
    DEBUG_METHOD_CALL("Application::ResetCounters");

    // reset counters
    digitalWrite(cOResetCounter, HIGH);
    delayMicroseconds(10);
    digitalWrite(cOResetCounter, LOW);
    delayMicroseconds(10);
}

void Application::RestartPulsDetection()
{
    DEBUG_METHOD_CALL("Application::RestartPulsDetection");

    // Restart puls detection
    digitalWrite(cONotResetPeriod, LOW);
    delayMicroseconds(10);
    digitalWrite(cONotResetPeriod, HIGH);
    delayMicroseconds(10);
}

void Application::RestartGateTimer()
{
    DEBUG_METHOD_CALL("Application::RestartGateTimer");

    // restart 0.5Hz
    digitalWrite(cOReset0_5Hz, HIGH);
    delayMicroseconds(10);
    digitalWrite(cOReset0_5Hz, LOW);
    delayMicroseconds(10);
}
