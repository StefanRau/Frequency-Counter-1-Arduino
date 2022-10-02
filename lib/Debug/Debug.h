// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Enables output of text via serial interface if debugging is not simply possible on that device

#pragma once
#ifndef _Debug_h
#define _Debug_h

#ifdef DEBUG_APPLICATION

#define DebugPrint(Text) Debug::GetInstance()->Print(Text)
#define DebugMethodCalls(Text) Debug::GetInstance()->Print(Text)
#define DebugInstantiation(Text) Debug::GetInstance()->Print(Text)
#define DebugPrintFromTask(Text) Debug::GetInstance()->PrintFromTask(Text)
#define DebugLoop() Debug::GetInstance()->loop()

#else

#define DebugPrint(Text)
#define DebugMethodCalls(Text)
#define DebugInstantiation(Text)
#define DebugPrintFromTask(Text)
#define DebugLoop()

#endif

#ifdef DEBUG_APPLICATION
#include <Arduino.h>

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
    static Debug *GetInstance();

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
