// Arduino Frequency Counter
// 19.10.2021
// Stefan Rau
// History
// 19.10.2021: 1st version - Stefan Rau
// 21.10.2021: Dispatcher has now chars as input - Stefan Rau
// 15.11.2021: Extended menu output by up/down buttons - Stefan Rau
// 05.12.2021: Initialization shows compile date - Stefan Rau
// 05.12.2021: Show arrows in line 2 - Stefan Rau
// 20.06.2022: Debug instantiation of classes - Stefan Rau
// 24.08.2022: Use new LCD library - Stefan Rau
// 06.09.2022: Singleton instantiation - Stefan Rau

#include "LCDHandler.h"
#include "ErrorHandler.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextLCDHandler::TextLCDHandler() : TextBase(-1)
{
    DebugInstantiation("New TextLCDHandler");
}

TextLCDHandler::~TextLCDHandler()
{
}

String TextLCDHandler::GetObjectName()
{
    switch (GetLanguage())
    {
        TextLangE("LCD");
        TextLangD("LCD");
    }
}

String TextLCDHandler::FrequencyCounter()
{
    switch (GetLanguage())
    {
        TextLangE("Frequencycounter");
        TextLangD("Frequenzzaehler");
    }
}

String TextLCDHandler::Selection()
{
    switch (GetLanguage())
    {
        TextLangE("Selection");
        TextLangD("Auswahl");
    }
}

String TextLCDHandler::Error()
{
    switch (GetLanguage())
    {
        TextLangE("Error");
        TextLangD("Fehler");
    }
}

String TextLCDHandler::InitError()
{
    switch (GetLanguage())
    {
        TextLangE("Can't init LCD");
        TextLangD("LCD kann nicht initialisiert werden");
    }
}

/////////////////////////////////////////////////////////////

// Module implementation

static LCDHandler::_eStateCode _mStateCode; // State of LCD handler for synchronization
static LCDHandler *gLCDHandler = nullptr;

LCDHandler::LCDHandler(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
    DebugInstantiation("New LCDHandler: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

    uint8_t lUp[8] = {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t lDown[8] = {0x00, 0x00, 0x00, 0x00, 0x11, 0x0A, 0x04, 0x00};
    uint8_t lUpDown[8] = {0x04, 0x0A, 0x11, 0x00, 0x11, 0x0A, 0x04, 0x00};

    _mText = new TextLCDHandler();

    // Initialize hardware
    _mI2ELCD = new hd44780_I2Cexp(mI2CAddress);

    if (_mI2ELCD->begin(16, 2) != 0)
    {
        DebugPrint("LCD can't be initialized");
        ErrorPrint(Error::eSeverity::TFatal, _mText->InitError());
        return;
    }

    DebugPrint("LCD is initialized at address: " + String(mI2CAddress));

    _mI2ELCD->noCursor();
    _mI2ELCD->noBlink();

    // Create special characters for scrolling the menu
    _mI2ELCD->createChar(0, lUp);
    _mI2ELCD->createChar(1, lDown);
    _mI2ELCD->createChar(2, lUpDown);

    mModuleIsInitialized = true;

    _mStateCode = TInitialize;
}

LCDHandler::~LCDHandler()
{
}

LCDHandler *LCDHandler::GetLCDHandler(sInitializeModule iInitializeModule)
{
	gLCDHandler = (gLCDHandler == nullptr) ? new LCDHandler(iInitializeModule) : gLCDHandler;
	return gLCDHandler;
}

void LCDHandler::loop()
{
    if (!mModuleIsInitialized)
    {
        return;
    }

    // the LCD must not be updated from a timer interrupt
    switch (_mStateCode)
    {
    case TInitialize:
        // Initialize LCD
        _mI2ELCD->backlight();
        _mI2ELCD->clear();
        _mI2ELCD->home();
        _mI2ELCD->print(_TrimLine(_mText->FrequencyCounter()));
        _mI2ELCD->setCursor(0, 1);
        _mI2ELCD->print(_TrimLine(String(__DATE__)));

        if (_mInputError == "")
        {
            _mStateCode = TDone;
        }
        else
        {
            _mStateCode = TShowError;
        }
        break;

    case TShowMenu:
        // show the menu
        _mI2ELCD->home();
        _mI2ELCD->print(_TrimLine(_mText->Selection()));
        _mI2ELCD->setCursor(0, 1);
        _mI2ELCD->print(_TrimLine(_mMenuSelectedFunction));
        _I2EWriteMenuNavigator();
        _mStateCode = TShowMenuDone;
        break;

    case TShowCounter:
        // show the value of the counter
        _mI2ELCD->home();
        _mI2ELCD->print(_TrimLine(_mInputSelectedFunction));
        _mI2ELCD->setCursor(0, 1);
        _mI2ELCD->print(_TrimLine(_mInputCurrentValue));
        _I2EWriteMenuNavigator();
        _mStateCode = TShowCounterDone;
        break;

    case TShowError:
        // output error message
        _mI2ELCD->home();
        _mI2ELCD->print(_TrimLine(_mText->Error()));
        _mI2ELCD->setCursor(0, 1);
        _mI2ELCD->print(_TrimLine(_mInputError));
        break;

    default:
        break;
    }
}

String LCDHandler::GetName()
{
    return _mText->GetObjectName();
}

#ifndef _DebugApplication
String LCDHandler::Dispatch(char iModuleIdentifyer, char iParameter)
{
    return String("");
}
#endif

// This function limits the maximum size of a text to 16
String LCDHandler::_TrimLine(String iText)
{
    String lLine;

    lLine = iText + "                ";
    lLine = lLine.substring(0, 16);
    return lLine;
}

void LCDHandler::_I2EWriteMenuNavigator()
{
    if (!mModuleIsInitialized)
    {
        return;
    }

    _mI2ELCD->setCursor(15, 1);

    if (_mLastMenuEntryNumber > 0)
    {
        if (_mCurrentMenuEntryNumber < 1)
        {
            // Scrolling up symbol
            _mI2ELCD->print((char)0);
        }
        else if (_mCurrentMenuEntryNumber == (_mLastMenuEntryNumber - 1))
        {
            // Scrolling down symbol
            _mI2ELCD->print((char)1);
        }
        else
        {
            // scrolling both directions
            _mI2ELCD->print((char)2);
        }
    }
    else
    {
        _mI2ELCD->print(' ');
    }
}

void LCDHandler::SetSelectedFunction(String iText)
{
    _mInputSelectedFunction = iText;
}

void LCDHandler::SetMeasurementValue(String iText)
{
    _mInputCurrentValue = iText;
}

void LCDHandler::SetErrorText(String iText)
{
    _mInputError = iText;
    _mStateCode = TShowError;
}

void LCDHandler::TriggerMenuSelectedFunction(String iText, int iCurrentMenuEntryNumber, int iLastMenuEntryNumber)
{
    _mMenuSelectedFunction = iText;
    _mCurrentMenuEntryNumber = iCurrentMenuEntryNumber;
    _mLastMenuEntryNumber = iLastMenuEntryNumber;
    if (_mStateCode != TInitialize)
    {
        _mStateCode = TShowMenu;
    }
}

void LCDHandler::TriggerShowRefresh()
{
    switch (_mStateCode)
    {
    case TShowMenuDone:
        _mStateCode = TShowMenu;
        break;
    case TShowCounterDone:
        _mStateCode = TShowCounter;
        break;
    default:
        break;
    }
}

void LCDHandler::TriggerShowCounter()
{
    switch (_mStateCode)
    {
    case TInitialize:
    case TShowError:
        break;
    default:
        _mStateCode = TShowCounter;
        break;
    }
}
