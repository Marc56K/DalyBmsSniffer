#include <Arduino.h>
#include <MQTT.h>
#include <daly-bms-uart.h>
#include "WifiManager.h"
#include "Config.h"

#define BMS_SERIAL Serial2 // ESP32: TX 17 RX 16
Daly_BMS_UART bms(BMS_SERIAL);

WifiManager wifi(WIFI_SSID, WIFI_PASS, HOSTNAME);
WiFiClient wifi_client;
MQTTClient mqtt_client;

void MqttPublish(const char *topic, const String &value)
{
  if (!mqtt_client.publish(topic, value, true, 1))
  {
    Serial.println(String("MQTT publish to ") + topic + " failed!");
  }
}

void MqttPublish(const char *topic, const float value)
{
  MqttPublish(topic, String(value));
}

void MqttPublish(const char *topic, const int value)
{
  MqttPublish(topic, String(value));
}


bool MqttConnect(int timeout)
{
  if (!mqtt_client.connected())
  {
    auto end = millis() + timeout;
    mqtt_client.setWill(MQTT_TOPIC_PREFIX "LWT", "Offline", true, 1);
    while (!mqtt_client.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_KEY) && millis() < end)
    {
      vTaskDelay(1000);
    }

    if (!mqtt_client.connected())
    {
      Serial.println("Can't connect to MQTT!");
    }
    else
    {
      MqttPublish(MQTT_TOPIC_PREFIX "LWT", "Online");
    }
  }
  return mqtt_client.connected();
}

void setup()
{
  Serial.begin(115200);
  bms.Init();

  mqtt_client.begin(MQTT_SERVER, MQTT_SERVERPORT, wifi_client);

  // connect to internet
  if (wifi.Connect(10000))
  {
    MqttConnect(3000);
  }
  else
  {
    Serial.println("Can't connect to WiFi!");
    Serial.flush();
    ESP.restart();
  }
}

void loop()
{
  mqtt_client.loop();
  if (bms.update())
  {
    MqttPublish(MQTT_TOPIC_PREFIX "uart_conected", 1);

    // And print them out!
    Serial.println("basic BMS Data:              " + (String)bms.get.packVoltage + "V " + (String)bms.get.packCurrent + "I " + (String)bms.get.packSOC + "\% ");
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/voltage", bms.get.packVoltage);
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/current", bms.get.packCurrent);
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/soc", bms.get.packSOC);

    Serial.println("Package Temperature:         " + (String)bms.get.tempAverage);
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/temperature", bms.get.tempAverage);

    Serial.println("Charge Status:               " + bms.get.chargeDischargeStatus);
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/charge", bms.get.chargeDischargeStatus);
    
    Serial.println("BMS Chrg / Dischrg Cycles:   " + (String)bms.get.bmsCycles);
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/cycles", bms.get.bmsCycles);
    Serial.println("BMS Heartbeat:               " + (String)bms.get.bmsHeartBeat); // cycle 0-255
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/heartbeat", bms.get.bmsHeartBeat);
    Serial.println("Remaining Capacity mAh:      " + (String)bms.get.resCapacitymAh);
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/capacity", bms.get.resCapacitymAh);

    Serial.println("Number of Cells:             " + (String)bms.get.numberOfCells);
    MqttPublish(MQTT_TOPIC_PREFIX "sensor/cells", bms.get.numberOfCells);
    for (size_t i = 0; i < size_t(bms.get.numberOfCells); i++)
    {
      String prefix = String(MQTT_TOPIC_PREFIX) + "sensor/cell" + String(i) + "/";
      Serial.println("Cell Voltage mV:             " + (String)bms.get.cellVmV[i]);
      MqttPublish((prefix + "voltage").c_str(), bms.get.cellVmV[i]);
      Serial.println("Cell Balanced:               " + (String)bms.get.cellBalanceState[i]);
      MqttPublish((prefix + "balanced").c_str(), (int)bms.get.cellBalanceState[i]);
    }
  }
  else
  {
    Serial.println("Can't connect to BMS!");
    MqttPublish(MQTT_TOPIC_PREFIX "uart_conected", 0);
  }

  if (!wifi.Connected() || !mqtt_client.connected() || millis() > 24 * 60 * 60 * 1000) // 24h
  {
    Serial.flush();
    ESP.restart();
  }

  vTaskDelay(5000);
}