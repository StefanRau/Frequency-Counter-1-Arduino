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
// 26.09.2022: EXTERNAL_EEPROM defined in platform.ini - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau
// 31.10.2022: Switch to minimal main program - Stefan Rau
// 20.01.2023: Improve debug handling - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

// Nano 33 IOT
// https://docs.arduino.cc/hardware/nano-33-iot
// https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/DataSheets/SAM-D21DA1-Family-Data-Sheet-DS40001882G.pdf

// Todo: control I2C regarding crashes
// Todo: implement web interface: web UI only
// Todo: use internal RTC for counting time - get time from web
// Todo: use own timer implementation for cyclic task timer
// Todo: use watchdog timer raising a reset after 10s? without trigger

#include "Application.h"

Application *gApplication;

void setup()
{
	DebugPrintLn("Start Main");
	gApplication = Application::GetInstance();
}

void loop()
{
	gApplication->loop();
}
