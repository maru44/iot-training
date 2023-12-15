#include <Arduino.h>
#include <cstdlib>
#include "HardwareSerial.h"

#define VOLT_PIN 32
#define HALL_GPIO GPIO_NUM_34
#define SIM_PWR_PIN 22
#define SIM_KEY_PIN 21

#define DBGSERIAL

#define SLEEP_TIME 1 * 60 * 1000000ull // sleep time (1min)

#define TINY_GSM_MODEM_SIM7080
#include <TinyGSM.h>
// TinyGsm modem(Serial2);

#include <StreamDebugger.h>
StreamDebugger debugger(Serial2, Serial);
TinyGsm modem(debugger);

void startSleep()
{
    modem.poweroff();
#ifdef DBGSERIAL
    Serial.println("sleep");
#endif
    delay(10);
    digitalWrite(SIM_KEY_PIN, LOW);
    digitalWrite(SIM_PWR_PIN, LOW);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME);
    delay(50);
    esp_deep_sleep_start();
}

void connectGPS()
{
    // pull SIM7080G(BK)'s K pin to ground
    digitalWrite(SIM_KEY_PIN, LOW);
    delay(1000);
    digitalWrite(SIM_KEY_PIN, HIGH);

    if (!modem.init())
    {
#ifdef DBGSERIAL
        Serial.println("failed to init modem");
        Serial.println("sleep");
#endif
        delay(10);
        digitalWrite(SIM_PWR_PIN, LOW);
        esp_sleep_enable_timer_wakeup(SLEEP_TIME);
        delay(50);
        esp_deep_sleep_start();
        return;
    }

    if (!modem.enableGPS())
    {
#ifdef DBGSERIAL
        Serial.println("failed to enable GPS");
#endif
        startSleep();
    }
}

void setup()
{
#ifdef DBGSERIAL
    Serial.begin(115200);
#endif
    pinMode(VOLT_PIN, ANALOG);
    pinMode(SIM_PWR_PIN, OUTPUT);
    pinMode(SIM_KEY_PIN, OUTPUT);

    Serial2.begin(115200);
    digitalWrite(SIM_KEY_PIN, HIGH);
    digitalWrite(SIM_PWR_PIN, HIGH);
    delay(20);
}

void loop()
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    connectGPS();

    float lat2 = 0;
    float lon2 = 0;
    float speed2 = 0;
    float alt2 = 0;
    int vsat2 = 0;
    int usat2 = 0;
    float accuracy2 = 0;
    int year2 = 0;
    int month2 = 0;
    int day2 = 0;
    int hour2 = 0;
    int min2 = 0;
    int sec2 = 0;
    for (int8_t i = 15; i; i--)
    {
        delay(5000L);
        modem.sendAT("+CGNSINF");
        if (modem.getGPS(&lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                         &year2, &month2, &day2, &hour2, &min2, &sec2))
        {
            Serial.printf("lat %g\n", lat2);
            Serial.printf("lon %g\n", lon2);

            String gps_str = modem.getGPSraw();
            const char *raw = NULL;
            raw = gps_str.c_str();
            Serial.printf("raw %s\n", raw);
            break;
        }
        else
        {
            Serial.printf("failed to get GPS: %d\n", i);
        }
    }
    modem.disableGPS();

    Serial.println("loop finished");

    startSleep();
}
