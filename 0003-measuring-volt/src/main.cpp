#include <Arduino.h>

#define AD_PIN 32

void setup()
{
  Serial.begin(115200);
  pinMode(AD_PIN, ANALOG);
}

void loop()
{
  Serial.printf("%d[mV]\n", analogReadMilliVolts(AD_PIN));
  delay(1000);
}
