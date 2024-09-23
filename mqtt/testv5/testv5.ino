//ap를 열고 ap에 접속하여 wifi정보를 입력하면 wifi연결 및 mqtt 연결 발행
//wifi scan 추가 
//webserver를 활용하여 여러페이지 구현 시도 -> 비동기 웹서버로 구현 시도
//연결 상태 값을 비동기로 수신하여 웹페이지에 출력
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi 설정
#define port 80
const char* ssid = "Kouno_Allsense";
const char* password = "kouno1092724855";

WiFiClient espClient;
PubSubClient client(espClient);

// 웹서버 초기화
AsyncWebServer server(port);

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

String getConnectionStatus() {
  String status;
  
  if (WiFi.status() == WL_CONNECTED) {
    status += "WiFi connected<br>";
  } else {
    status += "WiFi not connected<br>";
  }

  if (mqtt_server.length() != 0 && client.connected()) {
    status += "MQTT connected<br>";
  } else {
    status += "MQTT not connect<br>";
  }

  return status;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("Setting soft access point mode");
  WiFi.softAP(ssid, password);
  WiFi.mode(WIFI_MODE_APSTA);
  // WiFi.mode(WIFI_AP_STA);
  // WiFi.mode(WIFI_AP);
  //ap모드로 ap를 열고 값을 전달받은후 sta모드로 mqtt연결??
  // WiFi.mode(WIFI_STA);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  scanNetworks();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // 쿼리 파라미터를 가져옵니다.
    if (request->hasParam("wifissid")) {
      wifissid = request->getParam("wifissid")->value();
    }
    if (request->hasParam("wifipassword")) {
      wifipassword = request->getParam("wifipassword")->value();
    }
    if (request->hasParam("topic")) {
      topic = request->getParam("topic")->value();
    }
    if (request->hasParam("mqtt_server")) {
      mqtt_server = request->getParam("mqtt_server")->value();
    }
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
    <form id=\"wifiForm\">\
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
    <div id=\"responseBox\" style=\"border:1px solid #ccc; height:200px; overflow-y:scroll;\">\
      <p id='wifiSsid'>Wi-Fi SSID: " + wifissid + "</p>\
      <p id='wifiPassword'>Wi-Fi password: " + wifipassword + "</p>\
      <p id='topic'>topic: " + topic + "</p>\
      <p id='mqttServer'>MQTT server: " + mqtt_server + "</p>\
      <p id='connectionStatus'>" + getConnectionStatus() + "</p>\
    </div>\
    <script>\
    function updateStatus() {\
      fetch('/config')\
        .then(response => response.json())\
        .then(data => {\
          document.getElementById('connectionStatus').innerHTML = data.connectionStatus;\
        });\
    }\
    setInterval(updateStatus, 5000); // 5초마다 상태 업데이트\
    </script>\
    ";
      
    html += "</body>\
    </html>";

    request->send(200, "text/html", html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
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
    ";

    dataHtml += "<center><h1>Allsense Data</h1></center>\
    <center><h2>Web Server</h2></center>\
    <div id=\"responseBox\" style=\"border:1px solid #ccc; height:200px; overflow-y:scroll;\">\
      <p id='wifiSsid'>Wi-Fi SSID: " + wifissid + "</p>\
      <p id='wifiPassword'>Wi-Fi password: " + wifipassword + "</p>\
      <p id='topic'>topic: " + topic + "</p>\
      <p id='mqttServer'>MQTT server: " + mqtt_server + "</p>\
      <p id='connectionStatus'>" + getConnectionStatus() + "</p>\
    </div>\
    <script>\
    function updateStatus() {\
      fetch('/config')\
        .then(response => response.json())\
        .then(data => {\
          document.getElementById('connectionStatus').innerHTML = data.connectionStatus;\
        });\
    }\
    setInterval(updateStatus, 5000); // 5초마다 상태 업데이트\
    </script>\
    ";
      
    dataHtml += "</body>\
    </html>";

    request->send(200, "text/html", dataHtml);
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    String jsonResponse = "{";
    jsonResponse += "\"connectionStatus\": \"" + getConnectionStatus() + "\"";
    jsonResponse += "}";
    request->send(200, "application/json", jsonResponse);
  });

  server.begin();

}

void loop() {
  if ((wifissid.length() != 0) && (wifipassword.length() != 0) && (WiFi.status() != WL_CONNECTED)) {
    Serial.println("\nSetting Station configuration ... ");
    WiFi.begin(wifissid.c_str(), wifipassword.c_str());
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
    Serial.println(mqtt_server.c_str());
    if (!client.connected()) {
      reconnect();
    }
    String messageChar = "hellow";
    client.publish(topic.c_str(), messageChar.c_str());
    Serial.print("hellow");
  }
}


    // float o3 = taskParm2.lpPktData->mq131;
    // float co = taskParm2.lpPktData->co;
    // float no2 = taskParm2.lpPktData->no2;
    // float nh3 = taskParm2.lpPktData->nh3;
    // float c3h8 = taskParm2.lpPktData->c3h8;
    // float c4h10 = taskParm2.lpPktData->c4h10;
    // float ch4 = taskParm2.lpPktData->ch4;
    // float h2 = taskParm2.lpPktData->h2;
    // float c2h5oh = taskParm2.lpPktData->c2h5oh;
    // float ch2o = taskParm2.lpPktData->ch2o;
    // uint16_t srawVoc = taskParm2.lpPktData->voc;
    // float co2 = taskParm2.lpPktData->co2;
    // float temperature = taskParm2.lpPktData->temp;
    // float humidity = taskParm2.lpPktData->humi;