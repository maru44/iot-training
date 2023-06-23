#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "Secrets.h"
#include <cstdlib>
#include "esp_wifi.h"

#define VOLT_PIN 32
#define SLEEP_TIME 600 * 1000000ul // sleep time (10min)
const int VOLTAGE_DIVISIONAL_COUNT = 3;
RTC_DATA_ATTR int bootCount = 0;

const char *AWS_IOT_PUBLISH_TOPIC = "test0001/topic0001";

// TODO modify
const int WIFI_TIMEOUT_MS = 2000;
const int MQTT_TIMEOUT_MS = 2000;

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

inline const char *toString(esp_sleep_wakeup_cause_t v)
{
  switch (v)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    return "ESP_SLEEP_WAKEUP_UNDEFINED";
  case ESP_SLEEP_WAKEUP_EXT0:
    return "ESP_SLEEP_WAKEUP_EXT0";
  case ESP_SLEEP_WAKEUP_EXT1:
    return "ESP_SLEEP_WAKEUP_EXT1";
  case ESP_SLEEP_WAKEUP_TIMER:
    return "ESP_SLEEP_WAKEUP_TIMER";
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    return "ESP_SLEEP_WAKEUP_TOUCHPAD";
  case ESP_SLEEP_WAKEUP_ULP:
    return "ESP_SLEEP_WAKEUP_ULP";
  case ESP_SLEEP_WAKEUP_GPIO:
    return "ESP_SLEEP_WAKEUP_GPIO";
  case ESP_SLEEP_WAKEUP_UART:
    return "ESP_SLEEP_WAKEUP_UART";
  case ESP_SLEEP_WAKEUP_COCPU:
    return "ESP_SLEEP_WAKEUP_COCPU";
  case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
    return "ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG";
  case ESP_SLEEP_WAKEUP_BT:
    return "ESP_SLEEP_WAKEUP_BT";
  default:
    /* must not reach here */
    // sprintf(ret, "ESP_SLEEP_UNKNOWN:%d", v);
    // return ret;
    return "ESP_SlEEP_UNKNOWN:" + v;
  }
}

void connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");

  int wifi_connecting = 0;
  while ((WiFi.status() != WL_CONNECTED) && (wifi_connecting <= WIFI_TIMEOUT_MS))
  {
    delay(500);
    wifi_connecting += 500;
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wi-Fi Timeout");
    WiFi.disconnect();

    delay(30000);
    ESP.restart();
    return;
  }

  Serial.println("success to connect Wi-Fi");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  return;
}

void connectAWS()
{
  if (client.connected())
  {
    return;
  }

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.setTimeout(1000);

  Serial.println("Connecting to AWS IOT");

  int client_connecting = 0;
  while ((!client.connect(THINGNAME)) && (client_connecting <= MQTT_TIMEOUT_MS))
  {
    Serial.print(".");
    delay(100);
    client_connecting += 100;
  }

  if (!client.connected())
  {
    WiFi.disconnect(true, true);

    Serial.println("AWS IoT Timeout!");

    // delay(300000);
    // ESP.restart();
    delay(50);
    esp_deep_sleep(SLEEP_TIME);
    return;
  }

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  int milliVoltage = analogReadMilliVolts(VOLT_PIN);

  StaticJsonDocument<200> doc;
  doc["device_id"] = DEVICE_ID;
  doc["kind"] = "measure-voltage";
  // voltage is divided
  // times 3.3 / (divisional count * 4096) in lambda function
  int voltage = (int)(milliVoltage * 3.3 * VOLTAGE_DIVISIONAL_COUNT * 1000 / 4096);
  doc["milli_voltage"] = voltage;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  bool published = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  if (published)
  {
    Serial.println("Publish Success");
  }
  else
  {
    Serial.println("Publish fail");
  }
}

void publishSetup()
{
  StaticJsonDocument<200> doc;
  doc["device_id"] = DEVICE_ID;
  doc["kind"] = "setup";

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  bool published = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  if (published)
  {
    Serial.println("Publish setup Success");
  }
  else
  {
    Serial.println("Publish setup fail");
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(VOLT_PIN, ANALOG);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(false);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  Serial.printf("wake up: %s\n", toString(cause)); // TODO delete

  switch (cause)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    connectWiFi();
    connectAWS();
    publishSetup();
    // wifi_disconnect();

    delay(50);
    esp_deep_sleep(SLEEP_TIME);
    break;
  }
}

void loop()
{
  bootCount++;
  // 初回起動時は飛ばす
  if (bootCount > 1)
  {
    connectWiFi();
    connectAWS();
    publishMessage();
  }

  delay(50);
  esp_deep_sleep(SLEEP_TIME);
}
