// ref: https://github.com/Xinyuan-LilyGO/LilyGo-T-SIM7080G/blob/master/examples/SIM7080G-ATT-NB-IOT-AWS-MQTT/SIM7080G-ATT-NB-IOT-AWS-MQTT.ino
#define TINY_GSM_MODEM_SIM7080

// // Set serial for debug console (to the Serial Monitor, default speed 115200)为调试控制台设置串口(串口监视器，默认速度为115200)
// #define SerialMon Serial

#include "HardwareSerial.h"
#include <ArduinoJson.h>

// certs
#include "./certs/AWS_root_CA.h"
#include "./certs/AWS_Client_CRT.h"
#include "./certs/AWS_Client_PSK.h"

#include "Secrets.h"

// Your GPRS credentials, if any
const char apn[] = "soracom.io";
const char gprsUser[] = "sora";
const char gprsPass[] = "sora";

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

#define PIN_TX 17
#define PIN_RX 16
#define PWR_PIN 25

#define SLEEP_TIME 3 * 60 * 1000000ul // sleep time (3min)

#include <TinyGSM.h>
TinyGsm modem(Serial2);

const char *AWS_IOT_PUBLISH_TOPIC = "test0001/topic0001";

#include <ArduinoHttpClient.h>

char buffer[1024] = {0};
char jsonBuffer[512];

void writeCaFiles(int index, const char *filename, const char *data, size_t length)
{
    modem.sendAT("+CFSTERM");
    modem.waitResponse();

    modem.sendAT("+CFSINIT");
    if (modem.waitResponse() != 1)
    {
        Serial.println("INITFS FAILED");
        return;
    }
    // AT+CFSWFILE=<index>,<filename>,<mode>,<filesize>,<input time>
    // <index>
    //      Directory of AP filesystem:
    //      0 "/custapp/" 1 "/fota/" 2 "/datatx/" 3 "/customer/"
    // <mode>
    //      0 If the file already existed, write the data at the beginning of the
    //      file. 1 If the file already existed, add the data at the end o
    // <file size>
    //      File size should be less than 10240 bytes. <input time> Millisecond,
    //      should send file during this period or you can’t send file when
    //      timeout. The value should be less
    // <input time> Millisecond, should send file during this period or you can’t
    // send file when timeout. The value should be less than 10000 ms.

    size_t payloadLength = length;
    size_t totalSize = payloadLength;
    size_t alreadyWrite = 0;

    while (totalSize > 0)
    {
        size_t writeSize = totalSize > 10000 ? 10000 : totalSize;

        modem.sendAT("+CFSWFILE=", index, ",", "\"", filename, "\"", ",", !(totalSize == payloadLength), ",", writeSize, ",", 10000);
        modem.waitResponse(30000UL, "DOWNLOAD");
    REWRITE:
        modem.stream.write(data + alreadyWrite, writeSize);
        if (modem.waitResponse(30000UL) == 1)
        {
            alreadyWrite += writeSize;
            totalSize -= writeSize;
            Serial.printf("Writing:%d overage:%d\n", writeSize, totalSize);
        }
        else
        {
            Serial.printf("Write failed!: %s\n", filename);
            delay(1000);
            goto REWRITE;
        }
    }

    Serial.printf("Wirte done!!!: %s\n", filename);

    modem.sendAT("+CFSTERM");
    if (modem.waitResponse() != 1)
    {
        Serial.println("CFSTERM FAILED");
        return;
    }
}

char *setupPayload()
{
    StaticJsonDocument<200> doc;
    doc["device_id"] = DEVICE_ID;
    doc["kind"] = "setup";

    serializeJson(doc, jsonBuffer);
    return jsonBuffer;
}

void setup()
{
    // Set console baud rate
    Serial.begin(115200);
    delay(1000);

    Serial.println("init AT");
    Serial2.begin(115200);
    delay(1000);
}

void loop()
{
    // pinMode(PWR_PIN, OUTPUT);
    // digitalWrite(PWR_PIN, HIGH);
    // delay(1000);

    while (!modem.init())
    {
        return;
    }

    Serial.println("success");

    Serial.println("@ Wait For Network @");
    if (!modem.waitForNetwork())
    {
        // TODO:エラー処理
        Serial.println("failed to start network");
        modem.poweroff();
        return;
    }

    Serial.println("@ Wait For Network Connected @");
    if (!modem.isNetworkConnected())
    {
        // TODO:エラー処理
        return;
    }

    Serial.println("@ Connect GPRS: " + String(apn) + " @");
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        // TODO:エラー処理
        return;
    }

    Serial.println("@ Connected GPRS @");
    if (!modem.isGprsConnected())
    {
        // TODO:エラー処理
        return;
    }

    Serial.println("Step 6 done !");

    writeCaFiles(3, "rootCA.pem", AWS_ROOT_CA, strlen(AWS_ROOT_CA));                         // root_CA is retrieved from AWS_root_CA.h, which is the  "VeriSign Class 3 Public Primary G5 root CA certificate" from https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html"
    writeCaFiles(3, "deviceCert.crt", AWS_DEVICE_CERT, strlen(AWS_DEVICE_CERT));             // Client_CRT is retrieved from AWS_Client_CRT.h, please download the device certificate from AWS IoT Core when you create the thing
    writeCaFiles(3, "devicePrivateKey.pem", AWS_DEVICE_PRIVATE, strlen(AWS_DEVICE_PRIVATE)); // Client_PSK is retrieved from AWS_Client_PSK.h, please download the device private key from AWS IoT Core when you create the thing

    /***********************************
     * step 9 : Configure TLS/SSL before connecting to AWS IOT Core
     ***********************************/

    Serial.println("............................................................................Step 9");
    Serial.println("start to configure the TLS/SSL parameters ");

    // If it is already connected, disconnect it first
    modem.sendAT("+SMDISC");
    modem.waitResponse();

    snprintf(buffer, 1024, "+SMCONF=\"URL\",%s,%d", AWS_IOT_ENDPOINT, AWS_IOT_PORT);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1)
    {
        return;
    }

    // session clean
    modem.sendAT("+SMCONF=\"CLEANSS\",1");
    if (modem.waitResponse() != 1)
    {
        return;
    }

    // configure the socket keep-alive timer
    modem.sendAT("+SMCONF=\"KEEPTIME\",60");
    if (modem.waitResponse() != 1)
    {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"CLIENTID\",\"device-%s\"", DEVICE_ID);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1)
    {
        return;
    }

    // configure the QOS level for the MQTT connection
    modem.sendAT("+SMCONF=\"QOS\", 0");
    if (modem.waitResponse() != 1)
    {
        return;
    }

    // configure the SSL/TLS version for a secure socket connection
    modem.sendAT("+CSSLCFG=\"SSLVERSION\",0,3");
    if (modem.waitResponse() != 1)
    {
        return;
    }
    // convert the rootCA to the format required by the modem
    // <ssltype>
    //      1 QAPI_NET_SSL_CERTIFICATE_E
    //      2 QAPI_NET_SSL_CA_LIST_E
    //      3 QAPI_NET_SSL_PSK_TABLE_E
    // AT+CSSLCFG="CONVERT",2,"rootCA.crt"
    modem.sendAT("+CSSLCFG=\"CONVERT\",2,\"rootCA.pem\"");
    if (modem.waitResponse() != 1)
    {
        Serial.println("Convert rootCA.pem failed!");
    }

    // convert the deviceCert.crt and devicePrivateKey.pem to the format required by the modem
    modem.sendAT("+CSSLCFG=\"CONVERT\",1,\"deviceCert.crt\",\"devicePrivateKey.pem\"");
    if (modem.waitResponse() != 1)
    {
        Serial.println("Convert deviceCert.crt and devicePrivateKey.pem failed!");
    }

    /* enable SSL/TLS for a specific socket and set the root certificate authority (CA) and device certificate for secure communication.
    <index> SSL status, range: 0-6
            0 Not support SSL
            1-6 Corresponding to AT+CSSLCFG command parameter <ctindex>
            range 0-5
    <ca list> CA_LIST file name, Max length is 20 bytes
    <cert name> CERT_NAME file name, Max length is 20 bytes
    <len_calist> Integer type. Maximum length of parameter <ca list>.
    <len_certname> Integer type. Maximum length of parameter <cert name>. */
    modem.sendAT("+SMSSL=1,\"rootCA.pem\",\"deviceCert.crt\"");
    if (modem.waitResponse() != 1)
    {
        Serial.println("SSL with root CA and device certificate set up failed!");
    }
    else
    {
        Serial.println("SSL with root CA and device certificate set up successfully!");
    }
    Serial.println("Step 9 done !");

    /***********************************
     * step 10 : Connect AWS IOT Core, Publish to and Subscribe a topic from AWS IOT Core
     ***********************************/

    Serial.println("............................................................................Step 10");
    Serial.println("start to connect AWS IOT Core ");

    Serial.println("Connecting to AWS IOT Core ...");
    while (true)
    {
        modem.sendAT("+SMCONN");
        String response;
        int8_t ret = modem.waitResponse(60000UL, response);

        if (response.indexOf("ERROR") >= 0) // Check if the response contains "ERROR"
        {
            Serial.println("Connect failed");
            modem.poweroff();
            delay(50);
            esp_sleep_enable_timer_wakeup(SLEEP_TIME);
            esp_deep_sleep_start();
            return; // Stop attempting to connect
        }
        else if (response.indexOf("OK") >= 0) // Check if the response contains "OK"
        {
            Serial.println("Connect successfully");
            break; // Exit the loop
        }
        else
        {
            Serial.println("No valid response, retrying connect ...");
            delay(1000);
        }
    }

    // // Publish topic, below is an example, you need to create your own topic
    // String pub_topic = String(AWS_IOT_PUBLISH_TOPIC);
    // // String pub_topic = "$aws/things/" + String(AWS_IOT_PUBLISH_TOPIC);

    char buffer[1024];
    // AT+SMPUB=<topic>,<content length>,<qos>,<retain><CR>message is entered Quit edit mode if message length equals to <content length>
    snprintf(buffer, 1024, "+SMPUB=\"%s\",%d,0,0", AWS_IOT_PUBLISH_TOPIC, strlen(setupPayload())); // ! qos must be set to 0 since AWS IOT Core does not support QoS 1 SIM7080G

    modem.sendAT(buffer);
    if (modem.waitResponse(">") == 1)
    {
        modem.stream.write(setupPayload(), strlen(setupPayload()));
        Serial.println("");
        Serial.println(".......................................");
        Serial.println("Publishing below topic and payload now: ");
        Serial.println(AWS_IOT_PUBLISH_TOPIC);

        if (modem.waitResponse(3000))
        {
            Serial.println("Send Packet success!");
        }
        else
        {
            Serial.println("Send Packet failed!");
        }
    }

    delay(50);
    modem.poweroff();

    delay(50);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME);
    esp_deep_sleep_start();
}
