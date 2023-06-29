#include <Arduino.h>

#define HALL_PIN 12
#define SLEEP_TIME 30 * 1000000ul // sleep time (1min)

RTC_DATA_ATTR int bootCount = 0;

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

void setup()
{
  Serial.begin(115200);
  pinMode(HALL_PIN, INPUT);

  if (bootCount == 0)
  {
    Serial.println("setup!!");
  }

  delay(50);
}

void loop()
{
  Serial.printf("wake up cause: %s\n", toString(esp_sleep_get_wakeup_cause()));
  bootCount++;

  int digitalInput = digitalRead(HALL_PIN);
  Serial.printf("digital: %d\n", digitalInput);

  delay(50);
  // esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 1);
  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  esp_deep_sleep_start();
}
