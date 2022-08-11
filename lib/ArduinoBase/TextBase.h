// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Manages multi language texts for Arduino projects

#pragma once
#ifndef _TextBase_h
#define _TextBase_h

//#include <Arduino.h>
#include "Debug.h"
#include "ProjectBase.h"

#define TextLangD(Text) \
	default:            \
		return Text;    \
		break
#define TextLangE(Text) \
	case 'E':           \
		return Text;    \
		break

/// <summary>
/// This class manages the text handling and languages. Possible is currently German as default language and English optionally.
/// Reserved language codes are:
/// D: German
/// E: English
/// </summary>
class TextBase : public ProjectBase
{
private:
	const char _cDefaultLanguage = 'D';	  // Defines default langauge
	const String _cValidLanguages = "DE"; // List of all avallable languages
	char _mLanguage = _cDefaultLanguage;  // Current language
	const int _cEepromIndexLanguage = 1;  // Entry used for language

#ifndef _DebugApplication
										 // Remote commands
	enum eFunctionCode : char
	{
		TName = 'L' // Code, if controlled remotely
	};
#endif

protected:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="iSettingsAddress">Address of the EEPROM where language code is stored</param>
	TextBase(int iSettingsAddress);
	~TextBase();

public:
	/// <summary>
	/// Returns a list of all installed languages
	/// </summary>
	/// <returns>String with single character language codes</returns>
	String GetValidLanguages();

	/// <summary>
	/// Sets a language
	/// </summary>
	/// <param name="iLanguage">One character with language code</param>
	void SetLanguage(char iLanguage);

	/// <summary>
	/// Returns the current language
	/// </summary>
	/// <returns>One character with language code</returns>
	char GetLanguage();

	/// <summary>
	/// Returns the name of the currently selected language
	/// </summary>
	/// <returns>Readable name</returns>
	String GetSelectedLanguageName();

#ifndef _DebugApplication
	/// <summary>
	/// Dispatches commands got from en external input, e.g. a serial interface
	/// </summary>
	/// <param name="iModuleIdentifyer">If this matches with the identifyer of this module, then iParameter is analyzed:
	/// 'L' : Command for language operations</param>
	/// <param name="iParameter">Parameter or command that is to be analyzed:
	/// 'D' : Selects German => returns "D"
	/// 'E' : Selects English => returns "E"
	/// '*' : Returns a list of all installed languages => "DE"
	/// '?' : Returns the current language e.g. "D"
	/// </param>
	/// <returns>Reaction of dispatching</returns>
	String Dispatch(char iModuleIdentifyer, char iParameter) override;
#endif

	/// <summary>
	/// Returns the readable name of the object where the derivation of this text object is implemented
	/// </summary>
	/// <returns>Object name</returns>
	virtual String GetObjectName() = 0;

	String LanguageEnglish();
	String LanguageGerman();
};

#endif