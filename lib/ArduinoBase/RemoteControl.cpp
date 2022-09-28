// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// History
// 18.10.2021: 1st version - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 21.09.2022: use GetInstance instead of Get<Typename> - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau

#ifdef ARDUINO_AVR_NANO_EVERY
#include <SoftwareSerial.h>
#endif
#include "RemoteControl.h"

#ifndef DEBUG_APPLICATION

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextRemoteControl::TextRemoteControl() : TextBase(-1)
{
    DebugInstantiation("New TextRemoteControl");
}

TextRemoteControl::~TextRemoteControl()
{
}

String TextRemoteControl::GetObjectName()
{
    switch (GetLanguage())
    {
        TextLangE("Remote control");
        TextLangD("Fernsteuerung");
    }
}

String TextRemoteControl::Start()
{
    switch (GetLanguage())
    {
        TextLangE("Start remote control");
        TextLangD("Fernsteuerung starten");
    }
}

/////////////////////////////////////////////////////////////

static RemoteControl *gInstance = nullptr;

RemoteControl::RemoteControl()
{
    DebugInstantiation("New RemoteControl");

    _mText = new TextRemoteControl();

    //Serial.begin(115200);
    Serial.begin(9600);
    delay(10);
    Serial.println(_mText->Start());
    Serial.flush();
    _mBuffer = "";
}

RemoteControl::~RemoteControl()
{
}

RemoteControl *RemoteControl::GetInstance()
{
    // returns a pointer to singleton instance
    gInstance = (gInstance == nullptr) ? new RemoteControl : gInstance;
    return gInstance;
}

String RemoteControl::GetCommand()
{
    // Characters received by input stream are collected in an internal buffer and returned after CR or LF is detected.
    // Method must be called from main loop and reads only single characters, that processing is not slowed down.
    int lChar;
    String lBuffer;

    lChar = Serial.read();

    if (lChar >= 32)
    {
        Serial.print((char)lChar);
        Serial.flush();
        _mBuffer = _mBuffer + (char)lChar;
    }
    else if (lChar == '\n')
    {
        Serial.println('#');
        Serial.flush();
        lBuffer = _mBuffer;
        _mBuffer = "";
        return lBuffer;
    }

    return "";
}

void RemoteControl::Print(String iOutput)
{
    Serial.println(iOutput + String('#'));
    Serial.flush();
}

#endif