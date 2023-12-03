#ifndef MYWIFI
#define MYWIFI
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include <ArduinoJson.h>
#include <esp_log.h>

// Set the tag used in logging, you can set this to your class or file name
static const char* TAG = "HTTPClientManager";

void startup_wifi_task(void *pV);

class HTTPClientManager {
private:
    HTTPClient http;
    String serverUrl;

public:
    HTTPClientManager(const String& url);

    void begin();

    int postinfo(const int id, const int event_ts,const int sync_ts[TOTAL_NODES]);

    int postRAW(const String& jsonData);

    void end();

    ~HTTPClientManager();
};


#endif