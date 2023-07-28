#define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_DEBUG Serial
#define SerialMon Serial
#define SerialAT Serial1

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

//--- TODO: 使用するSIMのAPN情報に書き換えてください。
const char apn[] = "soracom.io"; // "h.iijmobile.biz";
const char lte_user[] = "sora";
const char lte_pass[] = "sora";
//---

const int rx_pin = 26;
const int tx_pin = 27;

const int sleep_sec = 60;

const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int port = 443;

// Global
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

/*
  setup関数
*/
void setup()
{
    delay(500);
    SerialMon.begin(115200);

    delay(3000);

    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);
    delay(500);
    // digitalWrite(25, LOW);
    // delay(500);

    SerialAT.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
    delay(3000);

    SerialMon.println("Wait...");
    // モデム初期化
    SerialMon.println("Initializing modem...");

    bool initOK = modem.init();
    // bool initOK = modem.testAT();

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

/*
  IIJ IoTサービスにデータ送信する
    引数
      const char* ns     : 送信する namespace値
      const char* name   : 送信する name値
      const double value : 送信する value値
    戻り値
      bool true:送信成功 false:送信失敗
*/
bool send_data()
{
    // JSON文字列をIIJ IoTサービスにPOST
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
