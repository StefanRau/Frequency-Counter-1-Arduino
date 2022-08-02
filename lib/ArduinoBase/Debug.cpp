// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// History
// 18.10.2021: 1st version - Stefan Rau
// 14.01.2022: using macros from timer interrupts for writing debugger output - Stefan Rau

#ifdef ARDUINO_AVR_NANO_EVERY
#include <SoftwareSerial.h> // For debug reasons
#endif

#include "Debug.h"

#ifdef _DebugApplication

static Debug *gDebug = nullptr;

Debug::Debug()
{
	Serial.begin(9600);
	delay(10);
	// That does not work using text objects, because we would get a circular reference there
	Serial.println("Start Debugger");
	// Wait until buffer is empty
	Serial.flush();
}

Debug::~Debug()
{
}

Debug *Debug::GetDebug()
{
	// returns a pointer to singleton instance
	gDebug = (gDebug == nullptr) ? new Debug : gDebug;
	return gDebug;
}

void Debug::Print(String iOutput)
{
	Serial.print(iOutput);
	Serial.println();
	// Wait until buffer is empty
	Serial.flush();
}

void Debug::PrintFromTask(String iOutput)
{
	// Write into a string buffer
	_mWriteBuffer += iOutput;
	_mBufferContainsData = true;
}

void Debug::loop()
{
	if (_mBufferContainsData)
	{
		Serial.print(_mWriteBuffer);
		Serial.println();
		// Wait until buffer is empty
		Serial.flush();
		_mWriteBuffer = "";
		_mBufferContainsData = false;
	}
}

#endif