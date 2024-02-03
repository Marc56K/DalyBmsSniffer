
#include <Arduino.h>
#include <ArduinoOTA.h>

namespace ota
{
    void init(const char* hostname, const char* password)
    {
        ArduinoOTA.onStart(
            []()
            {
                Serial.println("OTA: Begin");
            });
        ArduinoOTA.onEnd(
            []()
            {
                Serial.println("OTA: End");
            });
        ArduinoOTA.onProgress(
            [](unsigned int progress, unsigned int total)
            {
                Serial.print("OTA: Download: ");
                Serial.print(float(progress) / (total / 100.0f));
                Serial.println("%");
            });
        ArduinoOTA.onError(
            [](ota_error_t error)
            {
                Serial.printf("OTA: Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR)
                    Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR)
                    Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR)
                    Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR)
                    Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR)
                    Serial.println("End Failed");
            });

        ArduinoOTA.setHostname(hostname);
        ArduinoOTA.setPassword(password);
        ArduinoOTA.begin();
    }

    void update(unsigned long timeout)
    {
        auto start = millis();
        auto end = start + timeout;
        while (end > start && millis() < end)
        {
            ArduinoOTA.handle();
            delay(1);
        }
    }
}