#include <Arduino.h>
#include <WiFi.h>

const char *WIFI_NETWORK = "Kouno_GIGA_2.4G";
const char *WIFI_PASSWORD = "1092724855";
#define WIFI_TIMEOUT_MS 10000

void WiFiEvent(WiFiEvent_t event)
{
   Serial.println( "[WiFi-event] event: " + event );
  switch (event) {
        case SYSTEM_EVENT_WIFI_READY:
          Serial.println("WiFi interface ready");
          break;
        case SYSTEM_EVENT_SCAN_DONE:
          Serial.println("Completed scan for access points");
          break;
        case SYSTEM_EVENT_STA_START:
          Serial.println("WiFi client started");
          break;
        case SYSTEM_EVENT_STA_STOP:
          Serial.println("WiFi clients stopped");
          break;
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("Connected to access point");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Disconnected from WiFi access point");
      break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
          Serial.println("Authentication mode of access point has changed");
          break;
        case SYSTEM_EVENT_STA_GOT_IP:
          Serial.println("Obtained IP address: " + WiFi.localIP() );
          break;
        case SYSTEM_EVENT_STA_LOST_IP:
          Serial.println("Lost IP address and IP address is reset to 0");
          //      vTaskDelay( 5000 );
          //      ESP.restart();
          break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
          Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
          break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
          Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
          //      ESP.restart();
          break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
          Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
          break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
          Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
          break;
        case SYSTEM_EVENT_AP_START:
          Serial.println("WiFi access point started");
          break;
        case SYSTEM_EVENT_AP_STOP:
          Serial.println("WiFi access point  stopped");
          //      WiFi.mode( WIFI_OFF);
          //      esp_sleep_enable_timer_wakeup( 1000000 * 2 ); // 1 second times how many seconds wanted
          //      esp_deep_sleep_start();
          break;
        case SYSTEM_EVENT_AP_STACONNECTED:
          Serial.println("Client connected");
          break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("WiFi client disconnected");
          break;
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
          Serial.println("Assigned IP address to client");
          break;
        case SYSTEM_EVENT_AP_PROBEREQRECVED:
          Serial.println("Received probe request");
          break;
        case SYSTEM_EVENT_GOT_IP6:
          Serial.println("IPv6 is preferred");
          break;
        case SYSTEM_EVENT_ETH_GOT_IP:
          Serial.println("Obtained IP address");
          break;
    default: break;
  }
}

void connectToWiFi()
{
  int TryCount = 0;
  Serial.println( "connect to wifi" );
  WiFi.onEvent(WiFiEvent);
  while ( WiFi.status() != WL_CONNECTED )
  {
    TryCount++;
    WiFi.disconnect();
    WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
    Serial.println( TryCount);
    delay(4000);
    if ( TryCount == 10 )
    {
      ESP.restart();
    }
  }
  WiFi.onEvent(WiFiEvent);
}

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
}

void loop()
{
}