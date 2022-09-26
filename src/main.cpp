// Arduino Frequency Counter
// 21.10.2021
// Stefan Rau
// History
// 21.10.2021: Dispatcher has now chars as input - Stefan Rau
// 27.10.2021: Use error handler with persistent logging - Stefan Rau
// 28.10.2021: Calculate free RAM - Stefan Rau
// 04.11.2021: Verbose mode implemented - Stefan Rau
// 04.11.2021: Local dispatching now done with select-case - Stefan Rau
// 15.11.2021: Extended menu output by up/down buttons - Stefan Rau
// 12.01.2022: Extended by ARDUINO_NANO_RP2040_CONNECT - Stefan Rau
// 14.01.2022: Using macros from timer interrupts for writing debugger output - Stefan Rau
// 12.03.2022: Speed up remote control - Stefan Rau
// 16.03.2022: ARDUINO_NANO_RP2040_CONNECT removed - Stefan Rau
// 21.03.2022: Event counting added - Stefan Rau
// 22.03.2022: Separate reset for counter - Stefan Rau
// 29.03.2022: Separate reset functions - Stefan Rau
// 29.03.2022: Get name of device - Stefan Rau
// 24.08.2022: Trigger reading of event counter in sync to display refresh - Stefan Rau
// 07.09.2022: Transient error log removed - Stefan Rau

#include "main.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextMain::TextMain(int iSettingsAddress) : TextBase(iSettingsAddress)
{
}

TextMain::~TextMain()
{
}

String TextMain::GetObjectName()
{
	return "Main";
}

String TextMain::FreeMemory(int iFreeMemory)
{
	switch (GetLanguage())
	{
		TextLangE("Free memory: " + String(iFreeMemory) + " byte");
		TextLangD("Freier Speicher: " + String(iFreeMemory) + " Byte");
	}
}

String TextMain::ErrorInSetup()
{
	switch (GetLanguage())
	{
		TextLangE("Error in setup");
		TextLangD("Fehler im setup");
	}
}

String TextMain::ErrorInLoop()
{
	switch (GetLanguage())
	{
		TextLangE("Error in loop");
		TextLangD("Fehler in loop");
	}
}

/////////////////////////////////////////////////////////////

void setup()
{
	// Set clock frequency of I2C to 100kHz
	Wire.begin();
	delay(10);

#ifdef EXTERNAL_EEPROM
	ProjectBase::SetI2CAddressGlobalEEPROM(mInitializeSystem.EEPROM.I2CAddress);
#endif
	mText = new TextMain(mInitializeSystem.Text.SettingsAddress);

	DebugPrint("Start Setup");

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
	if (!ErrorDetected())
	{
		mLCDHandler = LCDHandler::GetInstance(mInitializeSystem.LCDHandler);
	}

	// Reset input modules and start lamp test
	if (!ErrorDetected())
	{
		mModuleFactory = ModuleFactory::GetInstance(mInitializeSystem.ModuleFactory);
		mModuleFactory->I2ELampTestOn();
	}

	// Initialize main counter
	if (!ErrorDetected())
	{
		mCounter = Counter::GetInstance(mInitializeSystem.Counter);
		mCounter->I2ESetFunctionCode(Counter::eFunctionCode::TFrequency);
	}

	// Initialize front plate
	if (!ErrorDetected())
	{
		// Reset selection and start
		mFrontPlate = FrontPlate::GetInstance(mInitializeSystem.FrontPlate, mLCDHandler, mModuleFactory, mCounter);
	}

	// Initialize task management and all tasks
	DebugPrint("Initialize tasks");

	// Task for lamp test end
	mLampTestTime = Task::GetNewTask(Task::TOneTime, 20, TaskLampTestEnd);

	// Task timer for switching off the menue in LCD
	mMenuSwitchOfTime = Task::GetNewTask(Task::TTriggerOneTime, 20, TaskMenuSwitchOff);
	mMenuSwitchOfTime->DefinePrevious(mLampTestTime);

	// Task timer LCD refresh
	mLCDRefreshCycleTime = Task::GetNewTask(Task::TFollowUpCyclic, 10, TaskLCDRefresh);
	mLCDRefreshCycleTime->DefinePrevious(mLampTestTime);

	// Initialize task handler
	TaskHandler::GetInstance()->SetCycleTimeInMs(100);

	// Reset
	ResetCounters();
	RestartGateTimer();
	RestartPulsDetection();

	// Initialize remote control
	if (!ErrorDetected())
	{
		RemoteControlInstance();
	}

	// Output potential errors
	if (ErrorDetected())
	{
		if (mLCDHandler != nullptr)
		{
			mLCDHandler->SetErrorText(mText->ErrorInSetup());
		}
		DebugPrint("Error in setup");
	}

	DebugPrint("End setup");
}

void loop()
{
	String lCommand = "";
	long lFreeMemory;

	lFreeMemory = GetFreeRAM();
	if (mFreeMemory != lFreeMemory)
	{
		mFreeMemory = lFreeMemory;
		DebugPrint("Free Memory: " + String(mFreeMemory));
	}

	if (mLCDHandler != nullptr)
	{
		mLCDHandler->loop(); // LCD must be called before the hardware is initialized to show potential errors
	}

	if (!mIsInitialized)
	{
		return;
	}

#ifndef DEBUG_APPLICATION
	// dispatch the different modules
	lCommand = RemoteControlGetCommand();
	if (lCommand != "")
	{
		int lSeparatorIndex;
		char lModule;
		char lParameter;
		String lReturn = "";

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
					lReturn += mLCDHandler->GetStatus();
					lReturn += mFrontPlate->GetStatus();
					lReturn += mCounter->GetStatus();
					lReturn += mModuleFactory->GetStatus();
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
				lReturn = ErrorHandler::GetInstance()->Dispatch(lModule, lParameter);
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
				lReturn = mModuleFactory->Dispatch(lModule, lParameter);
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
				lReturn = mFrontPlate->Dispatch(lModule, lParameter);
			}

			if (lReturn == "")
			{
				// Select or get the current language
				// lModule = 'L'			: Code for this class, if controlled remotely
				// lParameter = 'D', 'E'	: Select language
				// lParameter = '*'			: Lists all installed language as string of language codes - todo: im verbose mode komagetrennte Texte
				// lParameter = '?'			: Shows the current language code: 'D', 'E' - todo: im verbose mode den Klartext
				lReturn = mText->Dispatch(lModule, lParameter);
				if ((lReturn != "") && (lParameter != '*') && (lParameter != '?'))
				{
					mFrontPlate->TriggerLampTestOff(); // Reload displayed texts
				}
			}

			if (lReturn != "")
			{
				Serial.println(lReturn + "#");
			}
		}
	}
#endif

	// In case of an error, the program stops here
	if (ErrorHandler::GetInstance()->ContainsErrors())
	{
		if (!mErrorPrinted)
		{
			mLCDHandler->SetErrorText(mText->ErrorInLoop());
			DebugPrint("Error in runtime => processing stopped");
			mErrorPrinted = true;
		}
		return;
	}

	DebugLoop();
	mFrontPlate->loop();
	mModuleFactory->loop();
	mCounter->loop();

	// Reset I2C after an error was detected
	if (Wire.getWriteError() != 0)
	{
		Wire.end();
		Wire.begin();
		DebugPrint("Reset I2C");
	}

	// Get current menu item if a new one was selected
	if (mFrontPlate->IsNewMenuSelected())
	{
		DebugPrint("Trigger TaskMenuSwitchOff");
		mMenuSwitchOfTime->Restart();
		ModuleBase *lModuleBase = mModuleFactory->GetSelectedModule();
		mLCDHandler->TriggerMenuSelectedFunction(lModuleBase->GetCurrentMenuEntry(-1), lModuleBase->GetCurrentMenuEntryNumber(), lModuleBase->GetLastMenuEntryNumber());
		DebugPrint("Selected menu entry: " + lModuleBase->GetCurrentMenuEntry(-1));
	}

	// Reset counter if a new function is selected
	if (mFrontPlate->IsNewFunctionSelected())
	{
		ResetCounters();
		mMeasurementValue = mCounter->I2EGetCounterValue();
		RestartPulsDetection();
		RestartGateTimer();
		mEventCountingInitialized = false;
		mReadEventCounter = false;
	}

	if (mCounter->GetFunctionCode() == Counter::eFunctionCode::TFrequency)
	{
		// When frequency is selected
		if (digitalRead(cI0_5Hz) == LOW) // check if 10.000.000 pulses were counted
		{
			// DebugPrint("0.5 Hz signal detected");
			//  Wait a bit until the value is read
			//  => the counters are connected in a chain, it may happen some micro seconds until the last pulse reaches the last counter
			delayMicroseconds(10);
			mMeasurementValue = mCounter->I2EGetCounterValue();

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
	else if (mCounter->GetFunctionCode() == Counter::eFunctionCode::TEventCounting)
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
		if (mReadEventCounter)
		{
			mMeasurementValue = mCounter->I2EGetCounterValue();
			mReadEventCounter = false;
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
			mMeasurementValue = mCounter->I2EGetCounterValue();
			ResetCounters();
			RestartPulsDetection();
		}
	}

	mLCDHandler->SetMeasurementValue(mMeasurementValue);
}

void TaskLampTestEnd()
{
	DebugPrintFromTask("TaskLampTestEnd\n");

	// Switch off initialization message of LCD
	if (mLCDHandler != nullptr)
	{
		mLCDHandler->TriggerShowCounter();
	}

	// End lamp test at front plate and get stored function
	if (mFrontPlate != nullptr)
	{
		mFrontPlate->TriggerLampTestOff();
		DebugPrintFromTask("Initial function: " + mCounter->GetSelectedFunctionName() + "\n");
	}

	// End lamp test at all modules
	if (mModuleFactory != nullptr)
	{
		mModuleFactory->TriggerLampTestOff();
		DebugPrintFromTask("Initial module: " + mModuleFactory->GetSelectedModule()->GetName() + "\n");
		DebugPrintFromTask("Initial menu entry: " + mModuleFactory->GetSelectedModule()->GetCurrentMenuEntry(-1) + "\n");
	}

	mIsInitialized = true;
}

void TaskMenuSwitchOff()
{
	// Switch off menu message on display
	// DebugPrintFromTask("TaskMenuSwitchOff\n");
	if (mLCDHandler != nullptr)
	{
		mLCDHandler->TriggerShowCounter();
	}
}

void TaskLCDRefresh()
{
	// DebugPrintFromTask("LCDRefresh");
	mReadEventCounter = true;
	if (mLCDHandler != nullptr)
	{
		mLCDHandler->TriggerShowRefresh();
	}
}

long GetFreeRAM()
{
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

void ResetCounters()
{
	// reset counters
	digitalWrite(cOResetCounter, HIGH);
	delayMicroseconds(10);
	digitalWrite(cOResetCounter, LOW);
	delayMicroseconds(10);
}

void RestartPulsDetection()
{
	// Restart puls detection
	digitalWrite(cONotResetPeriod, LOW);
	delayMicroseconds(10);
	digitalWrite(cONotResetPeriod, HIGH);
	delayMicroseconds(10);
}

void RestartGateTimer()
{
	// restart 0.5Hz
	digitalWrite(cOReset0_5Hz, HIGH);
	delayMicroseconds(10);
	digitalWrite(cOReset0_5Hz, LOW);
	delayMicroseconds(10);
}
