//ap를 열고 ap에 접속하여 wifi정보를 입력하면 wifi연결 및 mqtt 연결 발행
//wifi scan 추가 
//webserver를 활용하여 여러페이지 구현 시도 및 handle처리
#include "WiFi.h"
#include <WebServer.h>
#include <PubSubClient.h>

#define port 80
const char* ssid = "Kouno_Allsense";
const char* password = "kouno1092724855";

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(port);

int n = 0; // WiFi 네트워크 수
String ssidList[100]; // 최대 20개의 SSID 저장
String rssiList[100]; // RSSI 값 저장
String channelList[100]; // 채널 저장
String encryptionList[100]; // 암호화 타입 저장

String wifissid; // WiFi SSID를 저장할 변수
String wifipassword; // WiFi 비밀번호를 저장할 변수
String topic;
String mqtt_server;

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

void handleRoot() {
  String html = "<!DOCTYPE html>\
  <html>\
  <head>\
  <style>\
  .scrollbox {\
    width: 750px;\
    height: 250px;\
    overflow: auto;\
    border: 1px solid #ccc;\
    padding: 10px;\
    margin: 0 auto;\ 
  }\
  body {\
    text-align: center;\
  }\
  .header {\
    text-align: center;\
  }\
  .button-container {\
    text-align: right;\
    margin-top: 10px;\
  }\
  button {\
    margin: 10px;\
  }\
  </style>\
  </head>\
  <body>\
    <div class=\"header\">\
      <h1>WiFi Networks</h1>\
    </div>\
    <div class=\"button-container\">\
      <button onclick=\"location.href='/'\">Main Page</button>\
      <button onclick=\"location.href='/data'\">Sensor Data</button>\
    </div>\
    <div class=\"scrollbox\">\
  ";

  if (n == 0) {
    html += "<p>No networks found</p>";
  } else {
    html += "<table>\
    <tr><th>Nr</th><th>SSID</th><th>RSSI</th><th>Channel</th><th>Encryption</th></tr>";
    for (int i = 0; i < n; ++i) {
      html += "<tr>";
      html += "<td>" + String(i + 1) + "</td>";
      html += "<td>" + ssidList[i] + "</td>";
      html += "<td>" + rssiList[i] + "</td>";
      html += "<td>" + channelList[i] + "</td>";
      html += "<td>" + encryptionList[i] + "</td>";
      html += "</tr>";
    }
    html += "</table>";
  }

  html += "</div>\
  <br>\
  <center><h1>Kouno Allsense Soft access point</h1></center>\
  <center><h2>Web Server</h2></center>\
  <form action=\"/change\" method=\"GET\">\
    <input type=\"text\" name=\"wifissid\" placeholder=\"WiFi SSID\" required>\
    <br>\
    <input type=\"text\" name=\"wifipassword\" placeholder=\"WiFi Password\" required>\
    <br>\
    <input type=\"text\" name=\"topic\" placeholder=\"Topic\" required>\
    <br>\
    <input type=\"text\" name=\"mqtt_server\" placeholder=\"Mqtt Server\" required>\
    <br>\
    <button type=\"submit\">Change WiFi</button>\
    <br><br>\
  </form>\
  ";
  
  html += "</body>\
  </html>";

  server.send(200, "text/html", html);
}

void handleData() {
  String dataHtml = "<!DOCTYPE html>\
  <html>\
  <head>\
  <style>\
  .scrollbox {\
    width: 750px;\
    height: 250px;\
    overflow: auto;\
    border: 1px solid #ccc;\
    padding: 10px;\
    margin: 0 auto;\ 
  }\
  body {\
    text-align: center;\
  }\
  .header {\
    text-align: center;\
  }\
  .button-container {\
    text-align: right;\
    margin-top: 10px;\
  }\
  button {\
    margin: 10px;\
  }\
  </style>\
  </head>\
  <body>\
    <div class=\"header\">\
      <h1>WiFi Networks</h1>\
    </div>\
    <div class=\"button-container\">\
      <button onclick=\"location.href='/'\">Main Page</button>\
      <button onclick=\"location.href='/data'\">Sensor Data</button>\
    </div>\
    <div class=\"scrollbox\">\
      <h1>Received Data</h1>\
      <p>wifissid: " + wifissid + "</p>\
      <p>wifipassword: " + wifipassword + "</p>\
      <p>topic: " + topic + "</p>\
      <p>Mqtt Server: " + mqtt_server + "</p>\
    </div>\
  </body>\
  </html>";

  server.send(200, "text/html", dataHtml);
}

void handleChange() {
  wifissid = server.arg("wifissid");
  wifipassword = server.arg("wifipassword");
  topic = server.arg("topic");
  mqtt_server = server.arg("mqtt_server");

  String response = "<!DOCTYPE html>\
  <html>\
  <head>\
  <style>\
  .scrollbox {\
    width: 750px;\
    height: 250px;\
    overflow: auto;\
    border: 1px solid #ccc;\
    padding: 10px;\
    margin: 0 auto;\
  }\
  body {\
    text-align: center;\
  }\
  .header {\
    text-align: center;\
  }\
  .button-container {\
    text-align: right;\
    margin-top: 10px;\
  }\
  button {\
    margin: 10px;\
  }\
  </style>\
  </head>\
  <body>\
    <div class=\"header\">\
      <h1>WiFi Networks</h1>\
    </div>\
    <div class=\"button-container\">\
      <button onclick=\"location.href='/'\">Main Page</button>\
      <button onclick=\"location.href='/data'\">Sensor Data</button>\
    </div>\
    <div class=\"scrollbox\">\
      <h1>Received Data</h1>\
      <p>wifissid: " + wifissid + "</p>\
      <p>wifipassword: " + wifipassword + "</p>\
      <p>topic: " + topic + "</p>\
      <p>Mqtt Server: " + mqtt_server + "</p>\
    </div>\
  </body>\
  </html>";
  
  server.send(200, "text/html", response);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    
    if (client.connect("ESP32Client")) { // 고유 id 아무거나 해도됌
      Serial.println("connected");
      // Subscribe
      client.subscribe(topic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void scanNetworks() {
  n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    ssidList[i] = WiFi.SSID(i);
    rssiList[i] = String(WiFi.RSSI(i));
    channelList[i] = String(WiFi.channel(i));
    encryptionList[i] = getEncryptionType(WiFi.encryptionType(i));
  }
  // 메모리 해제
  WiFi.scanDelete();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("Setting soft access point mode");
  WiFi.softAP(ssid, password);
  // wifi_ap모드가 wifi 와 ap연결을 동시에 할 수 있도록 함.
  WiFi.mode(WIFI_AP);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/change", HTTP_GET, handleChange);
  server.begin();
  scanNetworks();
}

void loop() {
  server.handleClient();

  if ((wifissid.length() != 0) && (wifipassword.length() != 0) && (WiFi.status() != WL_CONNECTED)) {
    Serial.println("\nSetting Station configuration ... ");
    WiFi.begin(wifissid, wifipassword);
    Serial.println(String("Connecting to ")+ wifissid);
    int attemptCount = 0; // 시도 횟수 카운터
    while (WiFi.status() != WL_CONNECTED && attemptCount < 10){
      delay(500);
      Serial.print(".");
      attemptCount++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected, IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect after 10 attempts.");
    }
  }

  if(mqtt_server.length()!=0 && !client.connected()){
    // && !client.connected()
    client.setServer(mqtt_server.c_str(), 1883);
    if (!client.connected()) {
      reconnect();
    }
    String messageChar = "hellow";
    client.publish(topic.c_str(), messageChar.c_str());
    Serial.print("hellow");
  }
  
  // "<!DOCTYPE html>\
  // <html>\
  // <head>\
  // <style>\
  // .scrollbox {\
  //   width: 750px;\
  //   height: 550px;\
  //   overflow: auto;\
  //   border: 1px solid #ccc;\
  //   padding: 10px;\
  //   margin: 0 auto; /* 중앙 정렬을 위한 자동 마진 */\
  // }\
  // body {\
  //   text-align: center; /* 모든 텍스트를 가운데 정렬 */\
  // }\
  // form {\
  //   display: inline-block; /* 폼을 인라인 블록으로 설정하여 가운데 정렬 */\
  // }\
  // button {\
  //   margin: 10px; /* 버튼 간격 조정 */\
  // }\
  // </style>\
  // </head>\
  // <body>\
  // <center><h1>WiFi Networks</h1></center>\
  // <center>\
  //   <button onclick=\"location.href='/'\">현재 페이지</button>\
  //   <button onclick=\"location.href='/graph.html'\">그래프 창</button>\
  // </center>\
  // <div class=\"scrollbox\">";

  // // WiFi 네트워크 스캔
  // if (n == 0) {
  //   html += "<p>No networks found</p>";
  // } else {
  //   html += "<table>\
  //   <tr><th>Nr</th><th>SSID</th><th>RSSI</th><th>Channel</th><th>Encryption</th></tr>";
  //   for (int i = 0; i < n; ++i) {
  //     html += "<tr>";
  //     html += "<td>" + String(i + 1) + "</td>";
  //     html += "<td>" + ssidList[i] + "</td>";
  //     html += "<td>" + rssiList[i] + "</td>";
  //     html += "<td>" + channelList[i] + "</td>";
  //     html += "<td>" + encryptionList[i] + "</td>";
  //     html += "</tr>";
  //   }
  //   html += "</table>";
  // }

  // html += "</div>\
  // <br>\
  // <center><h1>Kouno Allsense Soft access point</h1></center> \
  // <center><h2>Web Server</h2></center> \
  // <form action=\"/\" method=\"GET\"> \
  // <input type=\"text\" name=\"wifissid\" placeholder=\"WiFi SSID\" required> \
  // <br> \
  // <input type=\"text\" name=\"wifipassword\" placeholder=\"WiFi Password\" required> \
  // <br> \
  // <input type=\"text\" name=\"topic\" placeholder=\"Topic\" required> \
  // <br> \
  // <input type=\"text\" name=\"mqtt_server\" placeholder=\"Mqtt Server\" required> \
  // <br> \
  // <button type=\"submit\">Change WiFi</button><br><br> \
  // </form> \
  // <div class=\"scrollbox\"> \
  // <p>wifissid: " + wifissid + "</p> \
  // <p>wifipassword: " + wifipassword + "</p> \
  // <p>topic: " + topic + "</p> \
  // <p>Mqtt Server: " + mqtt_server + "</p> \
  // ";

  // // WiFi 연결 상태 확인
  // if (WiFi.status() == WL_CONNECTED) {
  //     html += "<p>WiFi connected</p>";
  // }

  // // MQTT 연결 상태 확인
  // if (client.connected()) {
  //     html += "<p>MQTT connected</p>";
  // }

  // html += "</div> \
  // </body>\
  // </html>";
  
}

