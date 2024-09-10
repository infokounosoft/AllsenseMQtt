#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// WiFi 설정
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// 웹서버 초기화
AsyncWebServer server(80);

// 센서 값 변수
float sensorValue = 0.0;

// 센서 값을 읽는 함수 (가상의 센서 예시)
float readSensor() {
    // 실제 센서 데이터를 읽는 코드로 대체하세요
    return random(0, 100) / 10.0; // 0.0 ~ 10.0 사이의 랜덤 값
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    // WiFi 연결 대기
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // HTML 페이지 서빙
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<html><body><h1>센서 값</h1>";
        html += "<p id='sensorValue'>값: " + String(sensorValue) + "</p>";
        html += "<script>setInterval(function(){fetch('/sensor').then(response => response.text()).then(data => document.getElementById('sensorValue').innerHTML = '값: ' + data);}, 500);</script>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });

    // 센서 값 요청 처리
    server.on("/sensor", HTTP_GET, [](AsyncWebServerRequest *request){
        sensorValue = readSensor();
        request->send(200, "text/plain", String(sensorValue));
    });

    server.begin();
}

void loop() {
    // 주기적으로 센서 값을 읽고 업데이트
    sensorValue = readSensor();
    delay(500); // 0.5초 대기
}
