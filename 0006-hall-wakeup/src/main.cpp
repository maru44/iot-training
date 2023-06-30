#include <Arduino.h>

#define HALL_PIN 12
#define SLEEP_TIME 30 * 1000000ul // sleep time (2min)

RTC_DATA_ATTR int bootCount = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(HALL_PIN, INPUT);
  delay(50);
}

void processWakeup(esp_sleep_wakeup_cause_t cause)
{
  switch (cause)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
  case ESP_SLEEP_WAKEUP_TIMER:
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);
    return;
    // case ESP_SLEEP_WAKEUP_EXT0:
    //   esp_sleep_disable_wakeup_source(esp_sleep_source_t::ESP_SLEEP_WAKEUP_EXT0);
    //   return;
  }
}

void loop()
{
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  processWakeup(cause);

  bootCount++;
  Serial.printf("bootCount: %d\n", bootCount);
  Serial.printf("wake up cause: %d\n", cause);

  int digitalInput = digitalRead(HALL_PIN);
  Serial.printf("digital: %d\n", digitalInput);

  delay(50);
  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  esp_deep_sleep_start();
}
