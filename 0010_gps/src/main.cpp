// ref: https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE/blob/master/examples/SIM7080/Platformio/src/main.cpp
#define TINY_GSM_MODEM_SIM7080

#include "HardwareSerial.h"

// // See all AT commands, if wanted 如果需要，请查看所有AT命令
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed 如果需要，为调试打印定义串行控制台
#define TINY_GSM_DEBUG Serial

/*
   Tests enabled
*/
#define TINY_GSM_TEST_GPRS true
#define TINY_GSM_TEST_WIFI false
#define TINY_GSM_TEST_CALL false
#define TINY_GSM_TEST_SMS false
#define TINY_GSM_TEST_USSD false
#define TINY_GSM_TEST_BATTERY false
#define TINY_GSM_TEST_GPS true
// powerdown modem after tests
#define TINY_GSM_POWERDOWN true

// set GSM PIN, if any 如果有GSM PIN，请设置
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "soracom.io";
const char gprsUser[] = "sora";
const char gprsPass[] = "sora";

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial2, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(Serial2);
#endif

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60          /*  Time ESP32 will go to sleep (in seconds) */

#define UART_BAUD 115200

#define PIN_SIM 25

void setup()
{
    // Set console baud rate
    Serial.begin(115200);
    delay(10);

    Serial2.begin(UART_BAUD);
    delay(100);
}

void loop()
{
    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    DBG("Initializing modem...");
    if (!modem.init())
    {
        DBG("Failed to restart modem, delaying 10s and retrying");
        return;
    }

    String name = modem.getModemName();
    DBG("Modem Name:", name);

    String modemInfo = modem.getModemInfo();
    DBG("Modem Info:", modemInfo);

    DBG("Waiting for network...");
    if (!modem.waitForNetwork())
    {
        delay(10000);
        return;
    }

    if (modem.isNetworkConnected())
    {
        DBG("Network connected");
    }

#if TINY_GSM_TEST_GPRS
    DBG("Connecting to", apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        delay(10000);
        return;
    }

    bool res = modem.isGprsConnected();
    DBG("GPRS status:", res ? "connected" : "not connected");

    String ccid = modem.getSimCCID();
    DBG("CCID:", ccid);

    String imei = modem.getIMEI();
    DBG("IMEI:", imei);

    String cop = modem.getOperator();
    DBG("Operator:", cop);

    IPAddress local = modem.localIP();
    DBG("Local IP:", local);

    int csq = modem.getSignalQuality();
    DBG("Signal quality:", csq);

#endif

#if TINY_GSM_TEST_GPS
    modem.disableGPS();
    float lat, lon;

    while (1)
    {
        if (!modem.enableGPS())
        {
            Serial.println("gps not enabled");
            delay(200);
            continue;
        }
        if (modem.getGPS(&lat, &lon))
        {
            Serial.printf("lat:%f lon:%f\n", lat, lon);
            // tick.attach_ms(200, []()
            //                { digitalWrite(LED_PIN, !digitalRead(LED_PIN)); });
            break;
        }
        else
        {
            Serial.print("getGPS ");
            Serial.println(millis());
        }
        delay(2000);
    }
    modem.disableGPS();

    modem.sendAT("+CGPIO=0,58,0,0");
    modem.sendAT("+CGPIO=0,57,0,0");
#endif

#if TINY_GSM_TEST_GPRS
    modem.gprsDisconnect();
    if (!modem.isGprsConnected())
    {
        DBG("GPRS disconnected");
    }
    else
    {
        DBG("GPRS disconnect: Failed.");
    }
#endif

#if TINY_GSM_POWERDOWN
    // Try to power-off (modem may decide to restart automatically)
    // To turn off modem completely, please use Reset/Enable pins
    modem.poweroff();
    DBG("Poweroff.");
#endif

    // Test is complete Set it to sleep mode
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    delay(200);
    esp_deep_sleep_start();
}
