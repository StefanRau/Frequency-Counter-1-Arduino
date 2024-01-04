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

static LCDHandler::eStateCode _mStateCode; // State of LCD handler for synchronization
static LCDHandler *gInstance = nullptr;

LCDHandler::LCDHandler(sInitializeModule iInitializeModule) : I2CBase(iInitializeModule)
{
    DEBUG_INSTANTIATION("LCDHandler: iInitializeModule[SettingsAddress, NumberOfSettings, I2CAddress]=[" + String(iInitializeModule.SettingsAddress) + ", " + String(iInitializeModule.NumberOfSettings) + ", " + String(iInitializeModule.I2CAddress) + "]");

    uint8_t lUp[8] = {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t lDown[8] = {0x00, 0x00, 0x00, 0x00, 0x11, 0x0A, 0x04, 0x00};
    uint8_t lUpDown[8] = {0x04, 0x0A, 0x11, 0x00, 0x11, 0x0A, 0x04, 0x00};

    mText = new TextLCDHandler();

    // Initialize hardware
    mI2ELCD = new hd44780_I2Cexp(mI2CAddress);

    if (mI2ELCD->begin(16, 2) != 0)
    {
        DEBUG_PRINT_LN("LCD can't be initialized");
        ERROR_PRINT(Error::eSeverity::TFatal, mText->InitError());
        return;
    }

    DEBUG_PRINT_LN("LCD is initialized at address: " + String(mI2CAddress));

    mI2ELCD->noCursor();
    mI2ELCD->noBlink();

    // Create special characters for scrolling the menu
    mI2ELCD->createChar(0, lUp);
    mI2ELCD->createChar(1, lDown);
    mI2ELCD->createChar(2, lUpDown);

    mModuleIsInitialized = true;

    _mStateCode = eStateCode::TInitialize;
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
    case eStateCode::TInitialize:
        // Initialize LCD
        mI2ELCD->backlight();
        mI2ELCD->clear();
        mI2ELCD->home();
        mI2ELCD->print(TrimLine(mText->FrequencyCounter()));
        mI2ELCD->setCursor(0, 1);
        mI2ELCD->print(TrimLine(String(__DATE__)));

        if (mInputError == "")
        {
            _mStateCode = eStateCode::TDone;
        }
        else
        {
            _mStateCode = eStateCode::TShowError;
        }
        break;

    case eStateCode::TShowMenu:
        // show the menu
        mI2ELCD->home();
        mI2ELCD->print(TrimLine(mText->Selection()));
        mI2ELCD->setCursor(0, 1);
        mI2ELCD->print(TrimLine(mMenuSelectedFunction));
        I2EWriteMenuNavigator();
        _mStateCode = eStateCode::TShowMenuDone;
        break;

    case eStateCode::TShowCounter:
        // show the value of the counter
        mI2ELCD->home();
        mI2ELCD->print(TrimLine(mInputSelectedFunction));
        mI2ELCD->setCursor(0, 1);
        mI2ELCD->print(TrimLine(mInputCurrentValue));
        I2EWriteMenuNavigator();
        _mStateCode = eStateCode::TShowCounterDone;
        break;

    case eStateCode::TShowError:
        // output error message
        mI2ELCD->home();
        mI2ELCD->print(TrimLine(mText->Error()));
        mI2ELCD->setCursor(0, 1);
        mI2ELCD->print(TrimLine(mInputError));
        break;

    default:
        break;
    }
}

String LCDHandler::GetName()
{
    DEBUG_METHOD_CALL("LCDHandler::GetName");

    return mText->GetObjectName();
}

#if DEBUG_APPLICATION == 0
String LCDHandler::DispatchSerial(char iModuleIdentifyer, char iParameter)
{
    return String("");
}
#endif

// This function limits the maximum size of a text to 16
String LCDHandler::TrimLine(String iText)
{
    DEBUG_METHOD_CALL("LCDHandler::_TrimLine");

    String lLine;

    lLine = iText + "                ";
    lLine = lLine.substring(0, 16);
    return lLine;
}

void LCDHandler::I2EWriteMenuNavigator()
{
    DEBUG_METHOD_CALL("LCDHandler::_I2EWriteMenuNavigator");

    if (!mModuleIsInitialized)
    {
        return;
    }

    mI2ELCD->setCursor(15, 1);

    if (mLastMenuEntryNumber > 0)
    {
        if (mCurrentMenuEntryNumber < 1)
        {
            // Scrolling up symbol
            mI2ELCD->print((char)0);
        }
        else if (mCurrentMenuEntryNumber == (mLastMenuEntryNumber - 1))
        {
            // Scrolling down symbol
            mI2ELCD->print((char)1);
        }
        else
        {
            // scrolling both directions
            mI2ELCD->print((char)2);
        }
    }
    else
    {
        mI2ELCD->print(' ');
    }
}

void LCDHandler::SetSelectedFunction(String iText)
{
    DEBUG_METHOD_CALL("LCDHandler::SetSelectedFunction");

    mInputSelectedFunction = iText;
}

void LCDHandler::SetMeasurementValue(String iText)
{
    DEBUG_METHOD_CALL("LCDHandler::SetMeasurementValue");

    mInputCurrentValue = iText;
}

void LCDHandler::SetErrorText(String iText)
{
    DEBUG_METHOD_CALL("LCDHandler::SetErrorText");

    mInputError = iText;
    _mStateCode = eStateCode::TShowError;
}

void LCDHandler::TriggerMenuSelectedFunction(String iText, int iCurrentMenuEntryNumber, int iLastMenuEntryNumber)
{
    DEBUG_METHOD_CALL("LCDHandler::TriggerMenuSelectedFunction");

    mMenuSelectedFunction = iText;
    mCurrentMenuEntryNumber = iCurrentMenuEntryNumber;
    mLastMenuEntryNumber = iLastMenuEntryNumber;
    if (_mStateCode != eStateCode::TInitialize)
    {
        _mStateCode = eStateCode::TShowMenu;
    }
}

void LCDHandler::TriggerShowRefresh()
{
    DEBUG_METHOD_CALL("LCDHandler::TriggerShowRefresh");

    switch (_mStateCode)
    {
    case eStateCode::TShowMenuDone:
        _mStateCode = eStateCode::TShowMenu;
        break;
    case eStateCode::TShowCounterDone:
        _mStateCode = eStateCode::TShowCounter;
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
    case eStateCode::TInitialize:
    case eStateCode::TShowError:
        break;
    default:
        _mStateCode = eStateCode::TShowCounter;
        break;
    }
}
