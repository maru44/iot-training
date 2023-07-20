#include "Secrets.h"

#define TINY_GSM_MODEM_SIM7080

// Set serial for debug console (to the Serial Monitor, default speed 115200)为调试控制台设置串口(串口监视器，默认速度为115200)
#define SerialMon Serial

// Set serial for AT commands (to the module) 设置AT命令的串行(到模块)
// Use Hardware Serial on Mega, Leonardo, Micro 使用硬件系列上的Mega, Leonardo, Micro
#define SerialAT Serial1

// Define the serial console for debug prints, if needed 如果需要，为调试打印定义串行控制台
#define TINY_GSM_DEBUG SerialMon

// Define how you're planning to connect to the internet
// These defines are only for this example; they are not needed in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
// #define GSM_PIN "0000"

// Your GPRS credentials, if any
const char apn[] = "soracom.io";
const char gprsUser[] = "sora";
const char gprsPass[] = "sora";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// Server details
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int port = 443;

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <Ticker.h>

// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);
HttpClient http(client, server, port);

Ticker tick;

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60          /*  Time ESP32 will go to sleep (in seconds) */
#define PIN_TX 27
#define PIN_RX 26
// #define PIN_ANT 32
#define POWER_PIN 25
#define UART_BAUD 115200

void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);
    delay(10);

    // POWER_PIN : This pin controls the power supply of the Modem
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);

    // pinMode(PIN_ANT, OUTPUT);
    // digitalWrite(PIN_ANT, HIGH);

    // // PWR_PIN ： This Pin is the PWR-KEY of the Modem PWR_PIN
    // // The time of active low level impulse of PWRKEY pin to power on module , type 500 ms
    // pinMode(PWR_PIN, OUTPUT);
    // digitalWrite(PWR_PIN, HIGH);
    // delay(500);
    // digitalWrite(PWR_PIN, LOW);
    // // delay(500);
    // //digitalWrite(PWR_PIN, HIGH);

    DBG("Wait...");

    delay(3000);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    modem.setNetworkMode(2);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    DBG("Initializing modem...");
    if (!modem.init())
    {
        DBG("Failed to restart modem, delaying 10s and retrying");
        return;
    }

    //    DBG("AT+CGMR");
    //        modem.sendAT("+CGMR");
    //    DBG("END");
}

void loop()
{
    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    DBG("Initializing modem...");
    if (!modem.restart())
    {
        DBG("Failed to restart modem, delaying 10s and retrying");
        return;
    }

    String name = modem.getModemName();
    DBG("Modem Name:", name);

    String modemInfo = modem.getModemInfo();
    DBG("Modem Info:", modemInfo);

    // #if TINY_GSM_USE_GPRS
    //     // Unlock your SIM card with a PIN if needed
    //     DBG("Modem Status:", modem.getSimStatus());
    //     if (modem.getSimStatus() != 3)
    //     {
    //         modem.simUnlock();
    //     }
    // #endif

#if TINY_GSM_USE_WIFI
    // Wifi connection parameters must be set before waiting for the network
    SerialMon.print(F("Setting SSID/password..."));
    if (!modem.networkConnect(wifiSSID, wifiPass))
    {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");
#endif

#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork())
    {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");

    if (modem.isNetworkConnected())
    {
        SerialMon.println("Network connected");
    }

#if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");

    if (modem.isGprsConnected())
    {
        SerialMon.println("GPRS connected");
    }
#endif

    SerialMon.print(F("Performing HTTPS GET request... "));
    http.connectionKeepAlive(); // Currently, this is needed for HTTPS
    int err = http.get(resource);
    if (err != 0)
    {
        SerialMon.println(F("failed to connect"));
        delay(10000);
        return;
    }

    int status = http.responseStatusCode();
    SerialMon.print(F("Response status code: "));
    SerialMon.println(status);
    if (!status)
    {
        delay(10000);
        return;
    }

    SerialMon.println(F("Response Headers:"));
    while (http.headerAvailable())
    {
        String headerName = http.readHeaderName();
        String headerValue = http.readHeaderValue();
        SerialMon.println("    " + headerName + " : " + headerValue);
    }

    int length = http.contentLength();
    if (length >= 0)
    {
        SerialMon.print(F("Content length is: "));
        SerialMon.println(length);
    }
    if (http.isResponseChunked())
    {
        SerialMon.println(F("The response is chunked"));
    }

    String body = http.responseBody();
    SerialMon.println(F("Response:"));
    SerialMon.println(body);

    SerialMon.print(F("Body length is: "));
    SerialMon.println(body.length());

    // Shutdown

    http.stop();
    SerialMon.println(F("Server disconnected"));

#if TINY_GSM_USE_WIFI
    modem.networkDisconnect();
    SerialMon.println(F("WiFi disconnected"));
#endif
#if TINY_GSM_USE_GPRS
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
#endif

    // Do nothing forevermore
    while (true)
    {
        delay(1000);
    }
}