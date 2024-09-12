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

String getConnectionStatus() {
    // 연결 상태를 반환하는 로직을 구현하세요.
    return "Connected"; // 예시로 "Connected" 반환
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("Setting soft access point mode");
  WiFi.softAP(ssid, password);
  WiFi.mode(WIFI_AP);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);



  // 루트 경로에 대한 요청 처리
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", "<div id='responseBox' style='border:1px solid #ccc; height:200px; overflow-y:scroll;'><p id='responseText'>Connection Status</p></div>");
  });

  // 연결 상태를 비동기적으로 가져오는 엔드포인트
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
      String connectionStatus = getConnectionStatus(); // 연결 상태를 가져오는 함수
      request->send(200, "text/plain", connectionStatus);
  });

  server.begin();
}

void loop() {
    // 주기적으로 상태를 업데이트하려면 추가 로직을 여기에 작성할 수 있습니다.
}


