#include <Arduino.h>
#include <cstdlib>
#include <iostream>
// #include <string>

#define LED_PIN 2

// char *test_str;
// std::string str;

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.printf("%s - run\n", __func__);
  pinMode(LED_PIN, OUTPUT);

  // test_str = std::getenv("TEST_STRING");
}

void loop()
{
  digitalWrite(LED_PIN, HIGH);
  Serial.printf("%s - LED_PIN - HIGH\n", __func__);

#ifdef ENV_TEST_STRING
  Serial.printf("%s\n", ENV_TEST_STRING);
#endif

  delay(1000);
  digitalWrite(LED_PIN, LOW);
  Serial.printf("%s - LED_PIN - LOW\n", __func__);
  delay(1000);
}
