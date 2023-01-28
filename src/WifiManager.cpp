#include "WifiManager.h"

WifiManager::WifiManager(
    const char *wifiSSID,
    const char *wifiKey,
    const char *hostName)
    : _wifiSSID(wifiSSID),
      _wifiKey(wifiKey),
      _hostName(hostName)
{
}

WifiManager::~WifiManager()
{
    Disconnect();
}

bool WifiManager::Connect(const int timeoutInMilis)
{
    if (_eventIds.empty())
    {
        _eventIds.push_back(
            WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
            {
                Serial.print("Obtained IP address: ");
                Serial.println(WiFi.localIP());
            }, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP));

        _eventIds.push_back(
            WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
            {
                WiFi.reconnect();
            }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED));

        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(_hostName.c_str());
        WiFi.begin(_wifiSSID.c_str(), _wifiKey.c_str());
    }

    for (int i = 0; i < timeoutInMilis && !Connected(); i += 100)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    return Connected();
}

void WifiManager::Disconnect()
{
    for (auto &eventId : _eventIds)
    {
        WiFi.removeEvent(eventId);
    }
    _eventIds.clear();
    WiFi.disconnect();
}

bool WifiManager::Connected()
{
    return WiFi.isConnected();
}