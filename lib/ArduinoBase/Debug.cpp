// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// History
// 18.10.2021: 1st version - Stefan Rau
// 14.01.2022: using macros from timer interrupts for writing debugger output - Stefan Rau
// 21.09.2022: use GetInstance instead of Get<Typename> - Stefan Rau
// 23.09.2022: Get rid of TimerInterrupt_Generic_Debug - Stefan Rau

#include "Debug.h"

#ifdef DEBUG_APPLICATION

static Debug *gInstance = nullptr;

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

Debug *Debug::GetInstance()
{
	// returns a pointer to singleton instance
	gInstance = (gInstance == nullptr) ? new Debug : gInstance;
	return gInstance;
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
		//Serial.println();
		// Wait until buffer is empty
		Serial.flush();
		_mWriteBuffer = "";
		_mBufferContainsData = false;
	}
}

#endif