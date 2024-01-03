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
// 21.09.2022: use GetInstance instead of Get<Typename> - Stefan Rau
// 26.09.2022: DEBUG_APPLICATION defined in platform.ini - Stefan Rau
// 21.12.2022: extend destructor - Stefan Rau
// 20.01.2023: Improve debug handling - Stefan Rau
// 16.07.2023: Debugging of method calls is now possible - Stefan Rau

#include "LCDHandler.h"
#include "ErrorHandler.h"

// Text definitions

/// <summary>
/// There is no new EEPROM address required
/// </summary>
TextLCDHandler::TextLCDHandler() : TextBase()
{
    DEBUG_INSTANTIATION("TextLCDHandler");
}

TextLCDHandler::~TextLCDHandler()
{
    DEBUG_DESTROY("TextLCDHandler");
}

String TextLCDHandler::GetObjectName()
{
    DEBUG_METHOD_CALL("TextLCDHandler::GetObjectName");

    switch (GetLanguage())
    {
        TextLangE("LCD");
        TextLangD("LCD");
    }
}

String TextLCDHandler::FrequencyCounter()
{
    DEBUG_METHOD_CALL("TextLCDHandler::FrequencyCounter");

    switch (GetLanguage())
    {
        TextLangE("Frequencycounter");
        TextLangD("Frequenzzaehler");
    }
}

String TextLCDHandler::Selection()
{
    DEBUG_METHOD_CALL("TextLCDHandler::Selection");

    switch (GetLanguage())
    {
        TextLangE("Selection");
        TextLangD("Auswahl");
    }
}

String TextLCDHandler::Error()
{
    DEBUG_METHOD_CALL("TextLCDHandler::Error");

    switch (GetLanguage())
    {
        TextLangE("Error");
        TextLangD("Fehler");
    }
}

String TextLCDHandler::InitError()
{
    DEBUG_METHOD_CALL("TextLCDHandler::InitError");

    switch (GetLanguage())
    {
        TextLangE("Can't init LCD");
        TextLangD("LCD kann nicht initialisiert werden");
    }
}

/////////////////////////////////////////////////////////////

// Module implementation

static LCDHandler::_eStateCode _mStateCode; // State of LCD handler for synchronization
static LCDHandler *gInstance = nullptr;

LCDHandler::LCDHandler(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
    DEBUG_INSTANTIATION("LCDHandler: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

    uint8_t lUp[8] = {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t lDown[8] = {0x00, 0x00, 0x00, 0x00, 0x11, 0x0A, 0x04, 0x00};
    uint8_t lUpDown[8] = {0x04, 0x0A, 0x11, 0x00, 0x11, 0x0A, 0x04, 0x00};

    _mText = new TextLCDHandler();

    // Initialize hardware
    _mI2ELCD = new hd44780_I2Cexp(mI2CAddress);

    if (_mI2ELCD->begin(16, 2) != 0)
    {
        DEBUG_PRINT_LN("LCD can't be initialized");
        ErrorPrint(Error::eSeverity::TFatal, _mText->InitError());
        return;
    }

    DEBUG_PRINT_LN("LCD is initialized at address: " + String(mI2CAddress));

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
    DEBUG_DESTROY("LCDHandler");
}

LCDHandler *LCDHandler::GetInstance(sInitializeModule iInitializeModule)
{
    DEBUG_METHOD_CALL("LCDHandler::GetInstance");

    gInstance = (gInstance == nullptr) ? new LCDHandler(iInitializeModule) : gInstance;
    return gInstance;
}

void LCDHandler::loop()
{
    DEBUG_METHOD_CALL("LCDHandler::loop");

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
    DEBUG_METHOD_CALL("LCDHandler::GetName");

    return _mText->GetObjectName();
}

#if DEBUG_APPLICATION == 0
String LCDHandler::DispatchSerial(char iModuleIdentifyer, char iParameter)
{
    return String("");
}
#endif

// This function limits the maximum size of a text to 16
String LCDHandler::_TrimLine(String iText)
{
    DEBUG_METHOD_CALL("LCDHandler::_TrimLine");

    String lLine;

    lLine = iText + "                ";
    lLine = lLine.substring(0, 16);
    return lLine;
}

void LCDHandler::_I2EWriteMenuNavigator()
{
    DEBUG_METHOD_CALL("LCDHandler::_I2EWriteMenuNavigator");

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
    DEBUG_METHOD_CALL("LCDHandler::SetSelectedFunction");

    _mInputSelectedFunction = iText;
}

void LCDHandler::SetMeasurementValue(String iText)
{
    DEBUG_METHOD_CALL("LCDHandler::SetMeasurementValue");

    _mInputCurrentValue = iText;
}

void LCDHandler::SetErrorText(String iText)
{
    DEBUG_METHOD_CALL("LCDHandler::SetErrorText");

    _mInputError = iText;
    _mStateCode = TShowError;
}

void LCDHandler::TriggerMenuSelectedFunction(String iText, int iCurrentMenuEntryNumber, int iLastMenuEntryNumber)
{
    DEBUG_METHOD_CALL("LCDHandler::TriggerMenuSelectedFunction");

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
    DEBUG_METHOD_CALL("LCDHandler::TriggerShowRefresh");

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
    DEBUG_METHOD_CALL("LCDHandler::TriggerShowCounter");

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
