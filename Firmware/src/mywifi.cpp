#include "mywifi.h"

extern HTTPClientManager *httpClient;
void startup_wifi_task(void *pV)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    WiFi.begin(WIFISSID, WIFIPWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    httpClient = new HTTPClientManager(SERVER_URL);
    vTaskDelete(NULL);
}

HTTPClientManager::HTTPClientManager(const String &url) : serverUrl(url) {}

void HTTPClientManager::begin()
{
    http.begin(serverUrl);
}

void HTTPClientManager::end()
{
    http.end();
}

HTTPClientManager::~HTTPClientManager()
{
    this->end();
}

int HTTPClientManager::postRAW(const String& rawData) {
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGE(TAG, "WiFi is not connected");
        return -1;
    }
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(rawData);
    // Handle the response as needed
    if (httpResponseCode > 0) {
        // Success, process response
        String payload = http.getString();
        ESP_LOGI(TAG, "Response: %s", payload.c_str());
    } else {
        // Error
        ESP_LOGE(TAG, "Error code: %d", httpResponseCode);
    }
    return httpResponseCode;
}

int HTTPClientManager::postinfo(const int id,const int event_ts, const size_t sync_ts[TOTAL_NODES])
{
    // Format the JSON payload
    static StaticJsonDocument<200> doc;
    doc["event_ts"] = event_ts;
    JsonArray syncArray = doc.createNestedArray("sync_ts");
    for (int i = 0; i < TOTAL_NODES; i++)
    {
        syncArray.add(sync_ts[i]);
    }
    String payload;
    serializeJson(doc, payload);

    // Call postRAW with the formatted JSON payload
    return postRAW(payload);
}