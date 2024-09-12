#include "WiFi.h"

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

void setup(){
  Serial.begin(115200);
  delay(1000);
  Serial.print("Setting soft access point mode");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
}


void loop(){
  WiFiClient client=server.available();
  if(client){
    String request = client.readStringUntil('\r');
    Serial.println(request); // 요청을 출력하여 디버깅
    if (request.indexOf("wifissid=") != -1 && request.indexOf("wifipassword=") != -1) {
      int ssidStartIndex = request.indexOf("wifissid=") + 9; // '=' 이후에서 시작
      int ssidEndIndex = request.indexOf('&', ssidStartIndex);
      if (ssidEndIndex == -1) ssidEndIndex = request.length();
      wifissid = request.substring(ssidStartIndex, ssidEndIndex);

      int passwordStartIndex = request.indexOf("wifipassword=") + 13; // '=' 이후에서 시작
      int passwordEndIndex = request.indexOf(' ', passwordStartIndex);
      if (passwordEndIndex == -1) passwordEndIndex = request.length();
      wifipassword = request.substring(passwordStartIndex, passwordEndIndex);

      Serial.print("WiFi SSID: ");
      Serial.println(wifissid); // SSID 출력
      Serial.print("WiFi Password: ");
      Serial.println(wifipassword); // 비밀번호 출력
      // WiFi 재연결
      // Connect_WiFi();
    }
    String html ="<!DOCTYPE html> \
    <html> \
    <head> \
    <style> \
    .scrollbox { \
      width: 300px; \
      height: 150px; \
      overflow: auto; \
      border: 1px solid #ccc; \
      padding: 10px; \
    } \
    </style> \
    </head> \
    <body> \
    <center><h1>Konuno Allsense Soft access point</h1></center> \
    <center><h2>Web Server</h2></center> \
    <form action=\"/\" method=\"GET\"> \
    <input type=\"text\" name=\"wifissid\" placeholder=\"WiFi SSID\" required> \
    <br> \
    <input type=\"password\" name=\"wifipassword\" placeholder=\"WiFi Password\" required> \
    <br> \
    <button type=\"submit\">Change WiFi</button><br><br> \
    </form> \
    <div class=\"scrollbox\"> \
    <p>" + wifissid + "</p> \
    <p>" + wifipassword + "</p> \
    </div> \
    </body> \
    </html>";
    client.print(html);
    request="";
  }
}