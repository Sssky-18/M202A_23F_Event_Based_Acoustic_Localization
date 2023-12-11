#include "mywifi.h"

extern HTTPClientManager *httpClient;
void startup_wifi_task(void *pV)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    WiFi.begin(WIFISSID, WIFIPWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    httpClient = new HTTPClientManager(SERVER_URL);
    httpClient->begin();
    vTaskDelete(NULL);
}

HTTPClientManager::HTTPClientManager(const String &url) : serverUrl(url) {}

void HTTPClientManager::begin()
{
    bool result;
    result=http.begin(serverUrl);
    if(!result){
        ESP_LOGE(TAG, "HTTP begin failed");
    }
    this->ready=true;
}

void HTTPClientManager::end()
{
    http.end();
}

HTTPClientManager::~HTTPClientManager()
{
    this->end();
}

int HTTPClientManager::postRAW(const String rawData) {
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGE(TAG, "WiFi is not connected");
        return -1;
    }
    if (!this->ready) {
        ESP_LOGE(TAG, "HTTP client is not ready");
        return -1;
    }
    http.addHeader("Content-Type", "application/json");
    const char * payload = rawData.c_str();
    size_t size=rawData.length();
    int httpResponseCode = http.POST((uint8_t*)payload,size);
    // Handle the response as needed
    if (httpResponseCode > 0) {
    } else {
        // Error
        ESP_LOGE(TAG, "Error code: %d", httpResponseCode);
    }
    return httpResponseCode;
    return 1;
}

int HTTPClientManager::postinfo(const int id,const int event_ts, const size_t sync_ts[TOTAL_NODES])
{
    // Format the JSON payload
    static StaticJsonDocument<100> doc;
    doc.clear();
    doc["id"] = id;
    doc["event_ts"] = event_ts;
    JsonArray syncArray = doc.createNestedArray("sync_ts");
    // ESP_LOGI(TAG, "sync_ts: %d %d %d", sync_ts[0], sync_ts[1], sync_ts[2]);

    for (int i = 0; i < TOTAL_NODES; i++)
    {
        syncArray.add(sync_ts[i]);
    }
    String payload;
    serializeJson(doc, payload);
    // ESP_LOGI(TAG, "Payload: %s", payload.c_str());
    return postRAW(payload);
}