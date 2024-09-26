#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* wifi_network_ssid = "Kouno_GIGA_2.4G";
const char* wifi_network_password =  "1092724855";
 
const char *soft_ap_ssid = "Kouno_Allsense";
const char *soft_ap_password = "1092724855";

AsyncWebServer server(80);

void setup() {
 
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_APSTA);
 
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
 
  WiFi.begin(wifi_network_ssid, wifi_network_password);
 
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());
 
  Serial.print("ESP32 IP on the WiFi network: ");
  Serial.println(WiFi.localIP());
 
 
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest * request) {
 
    if (ON_STA_FILTER(request)) {
      request->send(200, "text/plain", "Hello from STA");
      return;
 
    } else if (ON_AP_FILTER(request)) {
      request->send(200, "text/plain", "Hello from AP");
      return;
    }
 
    request->send(200, "text/plain", "Hello from undefined");
  });
 
  server.begin();
 
}
 
void loop() {}