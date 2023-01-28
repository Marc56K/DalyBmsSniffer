#pragma once
#include <string>
#include <vector>
#include <WiFi.h>

class WifiManager
{
public:
    WifiManager(const char* wifiSSID, const char* wifiKey, const char* hostName);
    ~WifiManager();

    bool Connect(const int timeoutInMilis);
    void Disconnect();
    bool Connected();

private:
    std::string _wifiSSID;
    std::string _wifiKey;
    std::string _hostName;
    std::vector<WiFiEventId_t> _eventIds;
};