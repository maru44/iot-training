#define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_DEBUG Serial
#define SerialMon Serial
#define SerialAT Serial2

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

const char apn[] = "soracom.io";
const char lte_user[] = "sora";
const char lte_pass[] = "sora";
//---

const int PIN_TX = 17;
const int PIN_RX = 16;
const int PIN_POWER = 4;

const int sleep_sec = 60;

const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int port = 443;

// Global
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

// #define AC_UART_INSTANCE 2
// #define AC_TX_PIN GPIO_NUM_4;
// #define AC_RX_PIN GPIO_NUM_0;

/*
  setup関数
*/
void setup()
{
    delay(500);
    SerialMon.begin(115200);
    while (!SerialMon)
        ;
    SerialMon.println("start");

    // pinMode(PIN_TX, OUTPUT);
    // pinMode(PIN_RX, INPUT);

    delay(5000);

    pinMode(4, OUTPUT);
    // digitalWrite(4, LOW);
    // delay(500);
    // digitalWrite(4, HIGH);
    // delay(500);
    // digitalWrite(4, LOW);
    // delay(500);
    digitalWrite(4, HIGH);
    delay(500);

    SerialMon.println("start2");

    SerialAT.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
    while (!SerialAT)
        ;

    SerialMon.printf("available: %d", SerialAT.available());

    delay(3000);

    pinMode(4, OUTPUT);
    // digitalWrite(4, LOW);
    // delay(500);
    // digitalWrite(4, HIGH);
    // delay(500);
    // digitalWrite(4, LOW);
    // delay(500);
    digitalWrite(4, HIGH);
    delay(500);

    delay(3000);

    SerialMon.println("Wait...");
    // モデム初期化
    SerialMon.println("Initializing modem...");

    bool initOK = modem.init();

    SerialMon.printf("init: %d\n", initOK);
    String modemInfo = modem.getModemInfo();
    SerialMon.println("Modem Info: " + modemInfo);
    // 接続
    SerialMon.println("Connecting to " + String(apn));
    while (!modem.gprsConnect(apn, lte_user, lte_pass))
    {
        SerialMon.println("failed to connect.");
        SerialMon.println("retry");
    }
    SerialMon.println("OK.");
    // 接続まち
    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork())
    {
        SerialMon.println(" fail");
        return;
    }
    SerialMon.println(" success");
    bool res = modem.isGprsConnected();
    SerialMon.printf("GPRS status: %s\n", res ? "connected" : "not connected");

    // 各種情報を出力
    String ccid = modem.getSimCCID();
    SerialMon.println("CCID: " + ccid);
    String imei = modem.getIMEI();
    SerialMon.println("IMEI: " + imei);
    String cop = modem.getOperator();
    SerialMon.println("Operator: " + cop);
    IPAddress local = modem.localIP();
    SerialMon.print("Local IP: ");
    SerialMon.println(local);
    int csq = modem.getSignalQuality();
    SerialMon.print("Signal quality: ");
    SerialMon.println(csq);

    // 乱数を初期化
    randomSeed(analogRead(25));
}

bool send_data()
{
    HttpClient http(client, server, port);
    http.beginRequest();
    http.get(resource);
    http.endRequest();
    int status_code = http.responseStatusCode();
    return status_code >= 200 && status_code < 300;
}

/*
  loop関数
*/
void loop()
{
    // データ送信する
    bool ok = send_data();
    SerialMon.println(ok ? "OK" : "NG");
    // 待つ
    delay(sleep_sec * 1000L);
}
