// #include "HardwareSerial.h"
// HardwareSerial Serial2(2);

// #define SerialMon Serial
// #define SerialAT Serial2

// #define GSM_AUTOBAUD_MIN 9600
// #define GSM_AUTOBAUD_MAX 115200
// #define PIN_TX 17
// #define PIN_RX 16
// #define POWER_PIN 25

// // const int sleep_sec = 60;

// #include <StreamDebugger.h>
// StreamDebugger debugger(SerialAT, SerialMon);

// /*
//   setup関数
// */
// void setup()
// {
//     SerialMon.begin(GSM_AUTOBAUD_MAX);
//     SerialAT.begin(GSM_AUTOBAUD_MAX, SERIAL_8N1);

//     pinMode(POWER_PIN, OUTPUT);
//     digitalWrite(POWER_PIN, HIGH);
// }

// void loop()
// {
//     // SerialMon.println(SerialMon.available());
//     // // SerialMon.println(SerialAT.available());
//     // while (SerialAT.available())
//     // {
//     //     SerialMon.println("ok");
//     //     int data = SerialAT.read();
//     //     SerialMon.write(data);
//     // }
//     SerialMon.println("ok");
//     // int data = SerialAT.read();
//     SerialAT.write("okdesuka?");

//     delay(1000);
//     SerialMon.println(SerialAT.read());
// }

#include "HardwareSerial.h"
// HardwareSerial Serial2(2);

#define RXD2 16
#define TXD2 17
void setup()
{
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    delay(1000);
    Serial.println("Loopback program started");
}
void loop()
{
    // Serial.println("AT: ");
    // Serial.println(Serial2);
    // Serial.println("ATA: ");
    // Serial.println(Serial2.available());
    // if (Serial.available())
    // {
    //     Serial.println("XXXX");
    //     Serial.write("-");
    //     Serial2.write(Serial.read());
    // }
    if (Serial2.available())
    {
        Serial.write(".");
        Serial.write(Serial2.read());
    }
    delay(2000);
    Serial2.write("write");
}