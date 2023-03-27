#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "Secrets.h"
#include <cstdlib>

#define SLEEP_TIME 60 * 1000000ul // sleep time (1min)

const char *AWS_IOT_PUBLISH_TOPIC = "test0001/topic0001";

const int WIFI_TIMEOUT_MS = 20000;
const int MQTT_TIMEOUT_MS = 5000;

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

void delay_with_client_loop(unsigned long ms)
{
  unsigned long start = millis();
  while ((millis() - start) < ms)
  {
    client.loop();
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

    delay(30000);
    ESP.restart();
    return;
  }

  Serial.println("AWS IoT Connected!");

  return;
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["message"] = "ok";

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

void notifySetup()
{
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  switch (cause)
  {
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("skip: wake up by timer"); // TODO delete
    break;

  default:
    // Serial.printf('wake up: %d\n', cause); // TODO delete
    Serial.printf("wu: %d\n", cause);
    // TODO: send topic to notify start
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(false);

  notifySetup();
}

void loop()
{
  connectWiFi();
  connectAWS();
  publishMessage();

  // delay_with_client_loop(60 * 1000);
  // delay(60000);

  delay(100);
  esp_deep_sleep(SLEEP_TIME);
}
