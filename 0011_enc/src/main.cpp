#include <Arduino.h>
#include <cstdlib>

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.printf("%s - run\n", __func__);
}

void loop()
{
    delay(1000);
    Serial.printf("%s - run\n", __func__);
    delay(1000);
}
