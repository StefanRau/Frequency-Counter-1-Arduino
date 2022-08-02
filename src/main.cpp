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

/////////////////////////////////////////////////////////////

void setup()
{
	// Set clock frequency of I2C to 100kHz
	Wire.begin();
	//Wire.setClock(100000);

#ifdef ARDUINO_SAMD_NANO_33_IOT
	delay(5000);
#endif

	ProjectBase::SetI2CAddressGlobalEEPROM(mInitializeSystem.EEPROM.I2CAddress);
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
	if (!ErrorHandler::GetErrorHandler()->ContainsErrors())
	{
		mLCDHandler = new LCDHandler(mInitializeSystem.LCDHandler);
	}

	// Reset input modules and start lamp test
	if (!ErrorHandler::GetErrorHandler()->ContainsErrors())
	{
		mModuleFactory = new ModuleFactory(mInitializeSystem.ModuleFactory);
		mModuleFactory->I2ELampTestOn();
	}

	// Initialize main counter
	if (!ErrorHandler::GetErrorHandler()->ContainsErrors())
	{
		mCounter = new Counter(mInitializeSystem.Counter);
		mCounter->I2ESetFunctionCode(Counter::eFunctionCode::TFrequency);
	}

	// Initialize front plate
	if (!ErrorHandler::GetErrorHandler()->ContainsErrors())
	{
		// Reset selection and start
		mFrontPlate = new FrontPlate(mInitializeSystem.FrontPlate, mLCDHandler, mModuleFactory, mCounter);
	}

	// Initialize task management and all tasks
	DebugPrint("Initialize tasks");

	// Task for lamp test end
	mLampTestTime = Task::GetNewTask(Task::TOneTime, 30, TaskLampTestEnd);

	// Task timer for switching off the menue in LCD
	mMenuSwitchOfTime = Task::GetNewTask(Task::TTriggerOneTime, 20, TaskMenuSwitchOff);
	mMenuSwitchOfTime->DefinePrevious(mLampTestTime);

	// Task timer LCD refresh
	mLCDRefreshCycleTime = Task::GetNewTask(Task::TFollowUpCyclic, 10, TaskLCDRefresh);
	mLCDRefreshCycleTime->DefinePrevious(mLampTestTime);

	// Initialize task handler
	TaskHandler::GetTaskHandler()->SetCycleTimeInMs(100);

	// Reset
	ResetCounters();
	RestartGateTimer();
	RestartPulsDetection();

	// Initialize remote control
	if (!ErrorHandler::GetErrorHandler()->ContainsErrors())
	{
		RemoteControlInstance();
	}

	// Output potential errors
	if (ErrorHandler::GetErrorHandler()->ContainsErrors())
	{
		if (mLCDHandler != nullptr)
		{
			mLCDHandler->SetErrorText(ErrorHandler::GetErrorHandler()->GetRootCause()->GetErrorEntry().ErrorMessage);
		}
		DebugPrint("Error when setup");
		DebugPrint(ErrorHandler::GetErrorHandler()->GetRootCause()->GetErrorEntry().ErrorMessage);
	}

	DebugPrint("End setup");
}

void loop()
{
	String lCommand = "";

	// DebugPrint("Loop");

	if (mLCDHandler != nullptr)
	{
		mLCDHandler->loop(); // LCD must be called before the hardware is initialized to show potential errors
	}

	//DebugLoop();

	if (!mIsInitialized)
	{
		
		// DebugPrint("Not Initialized");
		return;
	}

	//delay(10);

	// In case of an error, the program stops here
	if (ErrorHandler::GetErrorHandler()->ContainsErrors())
	{
		if (!mErrorPrinted)
		{
			DebugPrint("Error: " + ErrorHandler::GetErrorHandler()->GetRootCause()->GetErrorEntry().ErrorMessage + " - processing stopped");
			mErrorPrinted = true;
		}
		return;
	}

#ifndef _DebugApplication
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
					lReturn += ErrorHandler::GetErrorHandler()->GetStatus();
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

			if (lReturn == "")
			{
				// Manage error handling
				// lModule = 'E'	: Code for this class, if controlled remotely
				// lParameter = 'F' : Formatting EEPROM
				// lParameter = '0' : Reset Log Pointer
				// lParameter = 'R' : Read log
				// lParameter = 'S' : Read log size
				lReturn = ErrorHandler::GetErrorHandler()->Dispatch(lModule, lParameter);
			}

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
				// lParameter = '?'			: returns the number of the currently selected menu item - returns readable text in verbose mode
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
		}

		RemoteControlPrint(lReturn);
	}
#endif

	mFrontPlate->loop();
	mModuleFactory->loop();
	mCounter->loop();

	// Get current menu item if a new one was selected
	if (mFrontPlate->IsNewMenuSelected())
	{
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
	}

	if (mFrontPlate->GetSelectedFunctionCode() == Counter::eFunctionCode::TFrequency)
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
	else if (mFrontPlate->GetSelectedFunctionCode() == Counter::eFunctionCode::TEventCounting)
	{
		// Event counting used frequency counter input, but does not use 0.5Hz => that is set permanently to 1
		// DebugPrint("Count events");
		digitalWrite(cOReset0_5Hz, HIGH);
		digitalWrite(cOResetFF, LOW);
		delayMicroseconds(10);
		mMeasurementValue = mCounter->I2EGetCounterValue();
	}
	else
	{
		// Pulse length
		if (digitalRead(cIDone) == HIGH) // check if pulse end is detected
		{
			//DebugPrint("Trigger detected");
			//  Wait a bit until the value is read
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
		DebugPrintFromTask("Initial function: " + mFrontPlate->GetSelectedFunctionName() + "\n");
	}

	// End lamp test at all modues
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
	if (mLCDHandler != nullptr)
	{
		mLCDHandler->TriggerShowRefresh();
	}
}

int GetFreeRAM()
{
#ifdef ARDUINO_AVR_NANO_EVERY
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
#endif
#ifdef ARDUINO_SAMD_NANO_33_IOT
	return 0;
#endif
}

void ResetCounters()
{
	// reset counters
	//DebugPrint("Reset counters");
	digitalWrite(cOResetCounter, HIGH);
	delayMicroseconds(10);
	digitalWrite(cOResetCounter, LOW);
	delayMicroseconds(10);
}

void RestartPulsDetection()
{
	// Restart puls detection
	//DebugPrint("Restart pulse detection");
	digitalWrite(cONotResetPeriod, LOW);
	delayMicroseconds(10);
	digitalWrite(cONotResetPeriod, HIGH);
	delayMicroseconds(10);
}

void RestartGateTimer()
{
	// restart 0.5Hz
	//DebugPrint("Restart 0.5Hz");
	digitalWrite(cOReset0_5Hz, HIGH);
	delayMicroseconds(10);
	digitalWrite(cOReset0_5Hz, LOW);
	delayMicroseconds(10);
}
