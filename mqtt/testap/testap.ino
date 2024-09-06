#include "WiFi.h"
#include <WiFiClient.h>
#include <WebServer.h>

#define port 80
const char* ssid = "Kouno_Allsense";
const char* password = "kouno1092724855";
WiFiServer server(port);
String wifissid = ""; // WiFi SSID를 저장할 변수
String wifipassword = ""; // WiFi 비밀번호를 저장할 변수

void Connect_WiFi(){
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("Setting soft access point mode");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
}

void loop() {
  WiFiClient client=server.available();
  if(client){
    client.print(generateHtml());
  }
}

String generateHtml() {
  String html = "<!DOCTYPE html>\
  <html>\
  <head>\
  <style>\
  .scrollbox {\
    width: 750px;\
    height: 550px;\
    overflow: auto;\
    border: 1px solid #ccc;\
    padding: 10px;\
  }\
  </style>\
  </head>\
  <body>\
  <center><h1>WiFi Networks</h1></center>\
  <div class=\"scrollbox\">";

  // WiFi 네트워크 스캔
  int n = WiFi.scanNetworks();
  if (n == 0) {
    html += "<p>No networks found</p>";
  } else {
    html += "<table>\
    <tr><th>Nr</th><th>SSID</th><th>RSSI</th><th>Channel</th><th>Encryption</th></tr>";
    for (int i = 0; i < n; ++i) {
      html += "<tr>";
      html += "<td>" + String(i + 1) + "</td>";
      html += "<td>" + WiFi.SSID(i) + "</td>";
      html += "<td>" + String(WiFi.RSSI(i)) + "</td>";
      html += "<td>" + String(WiFi.channel(i)) + "</td>";
      html += "<td>" + getEncryptionType(WiFi.encryptionType(i)) + "</td>";
      html += "</tr>";
      delay(10); // 스캔 속도를 늦추기 위해 추가
    }
    html += "</table>";
  }

  html += "</div>\
  <br>\
  <button onclick=\"location.reload();\">Refresh</button>\
  </body>\
  </html>";

  // 스캔 결과 메모리 해제
  WiFi.scanDelete();

  return html;
}

String getEncryptionType(int type) {
  switch (type) {
    case WIFI_AUTH_OPEN: return "open";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA+WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-EAP";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2+WPA3";
    case WIFI_AUTH_WAPI_PSK: return "WAPI";
    default: return "unknown";
  }
}
