#include "mywifi.h"

extern HTTPClientManager *httpClient;
void startup_wifi_task(void *pV)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    WiFi.begin(WIFISSID, WIFIPWD);
    WiFi.setSleep(false);
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
    http.addHeader("Content-Type", "application/json");
    http.setReuse(false);
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

int HTTPClientManager::postRAW(String rawData) {
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGE(TAG, "WiFi is not connected");
        return -1;
    }
    if (!this->ready) {
        ESP_LOGE(TAG, "HTTP client is not ready");
        return -1;
    }
    
    int httpResponseCode = http.POST(rawData);
    // // Handle the response as needed
    // if (httpResponseCode > 0) {
    // } else {
    //     // Error
    //     ESP_LOGE(TAG, "Error code: %d", httpResponseCode);
    // }
    return httpResponseCode;
}

int HTTPClientManager::postRAWC(String rawData) {
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGE(TAG, "WiFi is not connected");
        return -1;
    }
    if (!this->ready) {
        ESP_LOGE(TAG, "HTTP client is not ready");
        return -1;
    }
    const char * cstr=rawData.c_str();
    int httpResponseCode = http.POST((uint8_t*)cstr,rawData.length());
    // // Handle the response as needed
    // if (httpResponseCode > 0) {
    // } else {
    //     // Error
    //     ESP_LOGE(TAG, "Error code: %d", httpResponseCode);
    // }
    // ESP_LOGE(TAG, "Error code: %d", httpResponseCode);
    vTaskDelay(pdMS_TO_TICKS(1000));
    return httpResponseCode;
}

int HTTPClientManager::postinfo(const int id,const int event_ts, const size_t sync_ts[TOTAL_NODES])
{
    if (!this->ready) {
        ESP_LOGE(TAG, "HTTP client is not ready");
        return -1;
    }
    // Format the JSON payload
    DynamicJsonDocument doc(200);
    doc.clear();
    doc["id"] = id;
    doc["event_ts"] = event_ts;
    JsonArray syncArray = doc.createNestedArray("sync_ts");
    for (int i = 0; i < TOTAL_NODES; i++)
    {
        syncArray.add(sync_ts[i]);
    }
    String payload;
    serializeJson(doc, payload);
    // ESP_LOGE(TAG, "Payload: %s", payload.c_str());
    int result=postRAW(payload);
    
    return result;
}