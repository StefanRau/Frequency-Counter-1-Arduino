// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Enables output of text via serial interface if debugging is not simply possible on that device

#pragma once
#ifndef _Debug_h
#define _Debug_h

//#define _DebugApplication

#ifdef _DebugApplication
#define _DebugMethodCalls
#define _DebugInstantiation

#define DebugPrint(Text) Debug::GetDebug()->Print(Text)
#define DebugPrintFromTask(Text) Debug::GetDebug()->PrintFromTask(Text)
#define DebugLoop() Debug::GetDebug()->loop()

#ifdef _DebugMethodCalls
#define DebugMethodCalls(Text) Debug::GetDebug()->Print(Text)
#else
#define DebugMethodCalls(Text)
#endif

#ifdef _DebugInstantiation
#define DebugInstantiation(Text) Debug::GetDebug()->Print(Text)
#else
#define DebugInstantiation(Text)
#endif

#else

#define DebugPrint(Text)
#define DebugMethodCalls(Text)
#define DebugInstantiation(Text)
#define DebugPrintFromTask(Text)
#define DebugLoop()

#endif

#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0

#ifdef _DebugApplication
#include <Arduino.h>
#include "TimerInterrupt_Generic_Debug.h"

/// <summary>
/// Enables output of text via serial interface if debugging is not simply possible on that device
/// Debugger shall not use text objects - write debugger texts always in English
/// </summary>
class Debug
{
private:
    Debug();
    ~Debug();

    String _mWriteBuffer = "";
    bool _mBufferContainsData = false;

public:
    /// <summary>
    /// Gets a singleton
    /// </summary>
    /// <returns>Instance of debugger</returns>
    static Debug *GetDebug();

    /// <summary>
    /// Writes debugging text to output
    /// </summary>
    /// <param name="iOutput">Text to write</param>
    void Print(String iOutput);

    /// <summary>
    /// Writes debugging text into buffer
    /// </summary>
    /// <param name="iOutput">Text to write</param>
    void PrintFromTask(String iOutput);

    /// <summary>
    /// Is called periodically from main loop
    /// </summary>
    void loop();
};
#endif

#endif
