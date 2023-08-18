#define TINY_GSM_MODEM_SIM7080

// Set serial for debug console (to the Serial Monitor, default speed 115200)为调试控制台设置串口(串口监视器，默认速度为115200)
#define SerialMon Serial

#include "HardwareSerial.h"

// // See all AT commands, if wanted 如果需要，请查看所有AT命令
// #define DUMP_AT_COMMANDS

// // Define the serial console for debug prints, if needed 如果需要，为调试打印定义串行控制台
// #define TINY_GSM_DEBUG SerialMon

// set GSM PIN, if any 如果有GSM PIN，请设置
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "soracom.io";
const char gprsUser[] = "sora";
const char gprsPass[] = "sora";

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

#define PIN_TX 17
#define PIN_RX 16
#define PWR_PIN 25

#define SLEEP_TIME 3 * 60 * 1000000ul // sleep time (3min)

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial2, SerialMon);
#else
#include <TinyGSM.h>
TinyGsm modem(Serial2);
#endif

#include <ArduinoHttpClient.h>
TinyGsmClientSecure client(modem);
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int port = 443;
HttpClient http(client, server, port);

void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);
    delay(1000);

    SerialMon.println("init AT");
    Serial2.begin(115200);
    delay(1000);
}

void loop()
{
    // pinMode(PWR_PIN, OUTPUT);
    // digitalWrite(PWR_PIN, HIGH);
    // delay(1000);

    while (!modem.init())
    {
        SerialMon.println("failed");
        return;
    }

    SerialMon.println("success");

    SerialMon.println("@ Wait For Network @");
    if (!modem.waitForNetwork())
    {
        // TODO:エラー処理
        return;
    }

    SerialMon.println("@ Wait For Network Connected @");
    if (!modem.isNetworkConnected())
    {
        // TODO:エラー処理
        return;
    }

    SerialMon.println("@ Connect GPRS: " + String(apn) + " @");
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        // TODO:エラー処理
        return;
    }

    SerialMon.println("@ Connected GPRS @");
    if (!modem.isGprsConnected())
    {
        // TODO:エラー処理
        return;
    }

    SerialMon.println("@ HTTP Get @");
    int err = http.get(resource);
    if (err != 0)
    {
        // TODO:エラー処理
        return;
    }

    int status = http.responseStatusCode();
    SerialMon.println("@ Status Code: " + String(status) + " @");
    if (!status)
    {
        // TODO:エラー処理
        return;
    }

    SerialMon.print("@ Header @");
    while (http.headerAvailable())
    {
        String headerName = http.readHeaderName();
        String headerValue = http.readHeaderValue();
        SerialMon.println("    " + headerName + " : " + headerValue);
    }

    int length = http.contentLength();
    if (length >= 0)
    {
        SerialMon.println("@ Content length: " + String(length) + " @");
    }

    SerialMon.println("@ Body @");
    String body = http.responseBody();
    SerialMon.println(body);

    http.stop();
    SerialMon.println("@ Stop HTTP @");

    modem.gprsDisconnect();
    SerialMon.println("@ Disconnect GPRS @");

    delay(50);
    modem.poweroff();

    delay(50);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME);
    esp_deep_sleep_start();
}
