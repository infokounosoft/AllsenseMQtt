
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


#include <Arduino.h>
#include <i2c_adc_ads7828.h>
// #include <EthernetENC.h>
// #include <EthernetWebServer.h>      // Library: EthernetWebServer by Khoi Hoang  2.2.3

#include "global.h"
#include "debug.h"

#include "sensor.h"
#include "websocket.h"
#include "ble.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "remote_control.h"
#include "ui.h"
#include "utils.h"
#include "task.h"
#include "led.h"
#include <Wire.h>
#include "ze08_ch2o.h"
//#include "spk_test.h"
//#include "XT_DAC_Audio.h"

/* SIP 라이브러리 */
#include <mbedtls/md5.h>
#include <driver/dac.h>

// reset
#include <esp_system.h>

// mqtt
#include <WiFi.h>
#include <PubSubClient.h>
const char* ssid = "Kouno_Zone_2G";
const char* password = "1092724855";
const char* mqtt_server = "10.0.0.70";
WiFiClient espClient;
PubSubClient client(espClient);
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "jude") {
    // Serial.print("data recieved ");
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
      client.subscribe("jude");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

#if REL_EN_ETHER_ENC

// ...../esp32/include/nghttp/port/include/http_parser.h:102:6: error: 'HTTP_OPTIONS' conflicts with a previous declaration

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 3
#define _ETG_LOGLEVEL_ 1

#define SHIELD_TYPE "ENC28J60 using EthernetENC Libraryon SPI2"

// Only one if the following to be true
#define USE_UIP_ETHERNET false
#define USE_ETHERNET_PORTENTA_H7 false
#define USE_NATIVE_ETHERNET false
#define USE_QN_ETHERNET false
#define USE_ETHERNET_GENERIC false
#define USE_ETHERNET_ESP8266 false
#define USE_ETHERNET_ENC true
#define USE_CUSTOM_ETHERNET false

#define USE_THIS_SS_PIN 15 // GPIO15

#include <EthernetENC.h>
// #include <EthernetWebServer.h>      // Library: EthernetWebServer by Khoi Hoang  2.2.3  // <<---- Error: ..../esp32/include/tcpip_adapter/include/tcpip_adapter.h:62:72: error: 'ip6_addr_t' has not been declared

#endif // REL_EN_ETHER_ENC

// 2022.03.03  ???????????????????????????????????
// #include <WiFi.h>
// #include <WiFiMulti.h>
// #include <ArduinoJson.h>

#if DBG_WEBSOCKET_FUNC
// #include <WiFiMulti.h>
////#include <WiFiClientSecure.h>
// #include <ArduinoJson.h>
////#include <WebSocketsClient.h>
#endif // DBG_WEBSOCKET_FUNC

#if DBG_EN_SOCKETIO
#include <SocketIOclient.h>
#endif // DBG_EN_SOCKETIO

// #if DBG_EN_OTA
#include "HttpsOTAUpdate.h"
// #endif // DBG_EN_OTA

#if REL_EN_DISP_SSD
#include <SSD1306Wire.h>
#endif // REL_EN_DISP_SSD

// 2022.03.03  ???????????????????????????????????
// WiFiMulti WiFiMulti;
// SocketIOclient socketIO;
// bool isConnected = false;      // 2021.12.09 sjsim
// unsigned long messageTimestamp = 0;

// const uint16_t port = WEBSOCKET_PORT;
// const char *server = WEBSOCKET_SVRIP;

// void socketIOEventLocal0(socketIOmessageType_t type, uint8_t *payload, size_t length);


APPL_PARM applParm;
TASK_PARM taskParm1;
TASK_PARM taskParm2;
TaskHandle_t task[4];
PKT_DATA pktData;

boolean acState = false;
boolean isBLE = false;
boolean propBLE = false;

void taskSensorI2C(void *parameter);


#if REL_EN_ETHER_ENC
// byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01};
byte mac[] = {0x34, 0x94, 0x54, 0x49, 0x18, 0x1C};
// 34:94:54:49:18:34

#endif // REL_EN_ETHER_ENC

#if DBG_IR_FUNC
long lastDebounceTime = 0; // the last time the output pin was toggled
// long debounceDelay = 50;     // the debounce time; increase if the output flickers
long debounceDelay = 500; // the debounce time; increase if the output flickers
int bState = -1;
int bPreState = -1;

// int btnState = LOW;
// int btnPreState = LOW;
// char gcFi = ' ';
// char gcSe = ' ';
#endif // DBG_IR_FUNC

static const int fw_ver = FW_VERSION;

#if DBG_EN_OTA

static char buf_url[128];
// static /*const*/ char *upd_server_ip = (char *)DEFAULT_FWSERVER_IP;   // "203.251.137.122"     // "192.168.10.200"
static char upd_server_ip[20];
static /*const*/ int upd_server_port = DEFAULT_FWSERVER_PORT; // 49080;
static const char *md5_str = "kn_light";                      // "d594b23abec72a4262ba2a2e41b4db06";
static const char *model = "kn_light";

static const char *server_certificate = "-----BEGIN CERTIFICATE-----\n"
                                        "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
                                        "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
                                        "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
                                        "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
                                        "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
                                        "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
                                        "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
                                        "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
                                        "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
                                        "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
                                        "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
                                        "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
                                        "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
                                        "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
                                        "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
                                        "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
                                        "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
                                        "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
                                        "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
                                        "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
                                        "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
                                        "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
                                        "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
                                        "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
                                        "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
                                        "-----END CERTIFICATE-----";

static HttpsOTAStatus_t otastatus;

#endif // DBG_EN_OTA

// extern char buf_pkt[sizeof(PKT)];
char lBufPkt[sizeof(PKT)];
char buf_rcvpkt[sizeof(PKT)]; //

#if REL_EN_ETHER_ENC // 2022.09.21
IPAddress ip(192, 168, 10, 185);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

// EthernetServer server(PORT_WEBSERVER);

// Enter the IP address of the server you're connecting to:
IPAddress ip_server(192, 168, 10, 232);
EthernetClient client;

int connStatus = NET_DISCONNECTED;
#endif // REL_EN_ETHER_ENC

boolean isEthernetHW = false;

#if REL_EN_DISP_SSD
// SSD1306Wire display(0x3c, -1, -1, GEOMETRY_128_64, I2C_ONE, 400000);      // sjsim
//SSD1306Wire display(0x3c);
SSD1306Wire display(0x3c, 21, 22); // sjsim
#endif // REL_EN_DISP_SSD

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t value = 0;


#if DBG_I2S
    XT_Wav_Class Sound(rawData);                                          
    XT_DAC_Audio_Class DacAudio(25,0);    
    uint32_t DemoCounter=0;
#endif

#if DBG_TASK_QUEUE
// You can also use the pdMS_TO_TICKS() macro which will convert
// a delay interval in ms to the equivalent of FreeRTOS ticks.
// For example, calling vTaskDelay(pdMS_TO_TICKS(1500)) to delay
// for 1500ms will be equivalent to calling vTaskDelay(150).

// ## sjsim, 2021.11.26, Task communication
#define MS(x) ((unsigned long)(x) / portTICK_PERIOD_MS)

/* 500ms. */
const TickType_t xDelay_500ms = 500 / portTICK_PERIOD_MS;

QueueHandle_t queueRemote; // for Remote Control  // queue
int queueRemoteSize = 10;

#endif // DBG_TASK_QUEUE

#if DBG_TASK_QUEUE2
QueueHandle_t queuePkt; // for PKT
int queuePktSize = 10;
#endif // DBG_TASK_QUEUE2

// ######################################
// ## BUILDING_ID, ROOM_ID   -> apply NVS variable
int global_building_id = GLOBAL_BUILDING_ID; // 2022.02.28
// int global_room_id = GLOBAL_ROOM_ID;                   // 2022.02.28
// int global_room_id = DEFAULT_ROOMNUM;                  // 2022.03.29

#if DBG_IR_FUNC
// ##################################################################
// ##################################################################
// ## EVENT, BUTTON

/*
// 2021.10.07 sjsim, IR remocon test
void IRAM_ATTR Ext_INT1_ISR(void)
{
  //Serial.println("Ext_INT1_ISR!!!!!!");

  //## Button Chattering 제거 코드 반영
  if ( (millis() - lastDebounceTime) > debounceDelay) {
    if (bState < 0) {
      bState = 0;
    }
    if (bState != bPreState) {
      bPreState = bState;
      if (bState == 0) {
        //Serial.println("Ext_INT1_ISR HIGH!!!!!!");
        bState = 1;

        Serial.println("AIRCON ON!!!!!!");
        acState = true;
        led_on();
        RemoconQuery('0', '1');
      } else {
        //Serial.println("Ext_INT1_ISR LOW!!!!!!");
        bState = 0;

        Serial.println("AIRCON OFF!!!!!!");
        acState = false;
        led_off();
        RemoconQuery('1', '1');
      }
      Serial.println("Ext_INT1_ISR CLICK!!!!!!");
    }

    lastDebounceTime = millis();                  // set the current time
  }

}
*/
// ##################################################################
// ##################################################################
// ## EVENT, BUTTON
#endif // DBG_IR_FUNC

#if DBG_EN_OTA
void HttpEvent(HttpEvent_t *event)
{
  switch (event->event_id)
  {
  case HTTP_EVENT_ERROR:
    Serial.println("Http Event Error");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    Serial.println("Http Event On Connected");
    break;
  case HTTP_EVENT_HEADER_SENT:
    Serial.println("Http Event Header Sent");
    break;
  case HTTP_EVENT_ON_HEADER:
    Serial.printf("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    break;
  case HTTP_EVENT_ON_FINISH:
    Serial.println("Http Event On Finish");
    break;
  case HTTP_EVENT_DISCONNECTED:
    Serial.println("Http Event Disconnected");
    break;
  }
}
#endif // DBG_EN_OTA

#if DBG_WEBSOCKET_FUNC
// -> websocket.ino
// void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
//{
//  socketIOEventLocal(type, payload, length);
//}
#endif // DBG_WEBSOCKET_FUNC

// ##################################################################
// ##################################################################
// ## BLE

#if DBG_MULTI_PKT
void sendRAW(char *buf, int sz_buf, int inx, int tot)
{
  char sndBuf[MAX_RAW_DATA + 3];

  if (sz_buf > MAX_RAW_DATA)
  {
    Serial.println("sendRAW Parameter error!");
    return;
  }

  sndBuf[0] = MSTX;
  sndBuf[1] = ((inx << 4) & 0xf0 | (tot & 0x0f));
  memcpy(sndBuf + 2, buf, sz_buf);

  displayPktTitle((char *)"SENDRAW", sndBuf, sz_buf + 2);

  pCharacteristic->setValue((uint8_t *)sndBuf, sz_buf + 2);
  pCharacteristic->notify();
  // value++;
  //  bluetooth stack will go into congestion, if too many packets are sent
  // delay(2000);
  delay(100);
}

int sendBLE(char *buf, int sz_buf)
{
  int szRemain;
  int szSend;
  int inxStart;
  int inx;
  int tot;

  if (buf == NULL || sz_buf <= 0)
  {
    return MY_ERROR;
  }

  // Serial.println("sendBLE");
  // Serial.printf("sendBLE Multi : %d\r\n", sz_buf);

  tot = (sz_buf / MAX_RAW_DATA) + 1; // MAX_RAW_DATA = 18
  inx = 0;
  // Serial.printf("sendBLE Multi : %d, %d\r\n", sz_buf, tot);
  // displayPktTitle("SENDBLE ALL", buf, sz_buf);

  if (deviceConnected)
  {
    if (sz_buf > MAX_RAW_DATA)
    {
      szRemain = sz_buf;
      inxStart = 0;
      szSend = MAX_RAW_DATA;

      while (szSend > 0)
      {
        // Serial.printf("sendBLE Multi : %d, %d (%d)\r\n", inxStart, szSend, szRemain);
        // displayPktTitle("SENDBLE", buf+inxStart, szSend);
        sendRAW(buf + inxStart, szSend, inx, tot);
        inx++;
        szRemain -= szSend;
        inxStart += szSend;
        if (szRemain <= MAX_RAW_DATA)
        {
          szSend = szRemain;
          szRemain = 0;
        }
      } // while
    }
    else
    {
      sendRAW(buf, sz_buf, 0, 1);
    }
  }
  else
  {
    Serial.printf("sendBLE Multi Error\r\n");
    return MY_ERROR;
  }

  // Serial.printf("sendBLE Multi END : %d\r\n", sz_buf);

  return sz_buf;
}
#else  // DBG_MULTI_PKT

int sendBLE(char *buf, int sz_buf)
{
  // #if 1
  if (deviceConnected)
  {
    pCharacteristic->setValue((uint8_t *)buf, sz_buf);
    pCharacteristic->notify();
    // value++;

    // bluetooth stack will go into congestion, if too many packets are sent
    // delay(2000);     // 2000ms   // original
    delay(100); // 2000ms   // original
  }

  return sz_buf;
  // #else
  //   return 10;
  // #endif
}
#endif // DBG_MULTI_PKT

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    Serial.println("## CONNECT");
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("## DISCONNECT");
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  TASK_PARM *lpTaskParm = NULL; // 2022.05.27
  PKT_DATA *lpPktData = NULL;   // 2022.05.27

public:
  MyCallbacks(void *pvParameters)
  { // 2022.05.27
    if (pvParameters != NULL)
    {
      lpTaskParm = (TASK_PARM *)pvParameters;
      // isBLE1 = lpTaskParm->isBLE;
      lpPktData = (PKT_DATA *)lpTaskParm->lpPktData;
      //} else {
      // Serial.println("task1 parameter is NULL");
    }
  }

  void setParm(void *pvParameters)
  { // 2022.05.27
    if (pvParameters != NULL)
    {
      lpTaskParm = (TASK_PARM *)pvParameters;
      // isBLE1 = lpTaskParm->isBLE;
      lpPktData = (PKT_DATA *)lpTaskParm->lpPktData;
      //} else {
      // Serial.println("task1 parameter is NULL");
    }
  }

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

#if DBG_MULTI_PKT
    MPKT mPkt; // size : 131
    // char *lpPkt = NULL;
    static char lpPkt[SIZE_MPKT];
    char buf_dt[MAX_MDATA];
    // char pktBuf[SIZE_MPKT];
    char pktBuf[32]; // data max : 20
    int pg_inx;
    int pg_tot;
    int st_inx;
    static int pg_co = 0;
    static int rcv_len = 0;
    static int inx = 0;

#else  // DBG_MULTI_PKT
    char buf_dt[MAX_DATA];
    int lenPkt = 0;
#endif // DBG_MULTI_PKT
    char buf_cmd;
    char ch_data;
    unsigned char ckbyte;
    int ret;
    int lenBuf;
    int len;
    int i;

    // Serial.println("**");
    // Serial.printf("Received Value: %d\r\n" + rxValue.length());
    lenBuf = rxValue.length();

    if (rxValue.length() > 0)
    {
      // char pktBuf[lenBuf];
      // len = rxValue.length();

      if (lpTaskParm != NULL)
      { // 2022.05.27
        lpTaskParm->isBLEDataRecv = true;
      }

#if DBG_MULTI_PKT
      // rxValue.toCharArray(char_array, lenBuf);
      for (i = 0; i < lenBuf; i++)
      {
        pktBuf[i] = rxValue[i];
        pktBuf[i + 1] = 0x00;
      }

      // Serial.println("***");
      // displayPkt(pktBuf, lenBuf);
      displayPktTitle((char *)"Recv", pktBuf, lenBuf);

      // lpPkt = (char *)&mPkt;
      pg_inx = 0;
      pg_tot = 0;

      if (pktBuf[0] == MSTX)
      {
        pg_inx = (int)(pktBuf[1] >> 4 & 0x0f); // 0,1,2,....,pg_tot-1
        pg_tot = (int)(pktBuf[1] & 0x0f);
        if (pg_inx == 0)
        {
          pg_co = 1;
          rcv_len = 0;
          inx = 0;
          memset(lpPkt, 0x00, sizeof(lpPkt));
        }
        else
        {
          pg_co++;
        }
        st_inx = pg_inx * MAX_RAW_DATA;

        Serial.printf("Recv inx:%d, tot:%d, co:%d, st_inx:%d\r\n", pg_inx, pg_tot, pg_co, st_inx); // #### DBG

        ////memcpy(lpPkt + st_inx, &pktBuf[2], lenBuf-2);
        // memcpy(lpPkt+st_inx, pktBuf+2, lenBuf-2);
        for (i = 0; i < lenBuf - 2; i++)
        {
          lpPkt[inx] = pktBuf[i + 2];
          inx++;
        }

        rcv_len += lenBuf - 2;
        // displayPkt((char *)lpPkt, MAX_RAW_DATA+2);    // 20
      }
      // if (pg_co > 0 && pg_tot > 0 && pg_co == pg_tot-1) {
      if (pg_co > 0 && pg_tot > 0 && pg_co == pg_tot)
      {
        // memcpy((char *)&mPkt, lpPkt, rcv_len);
        // displayPkt((char *)&mPkt, rcv_len);                                                           //#### DBG
        Serial.printf("Complete Len:%d\r\n", rcv_len);

        // displayPktTitle("Complete", (char *)&mPkt, rcv_len);                                          //#### DBG
        displayPktTitle((char *)"Completelp", (char *)lpPkt, rcv_len); // #### DBG

        // lenPkt = mPkt.len;            // Review!!!!!   2022.04.04
        // Serial.printf("**** lenPkt: %d\r\n", lenPkt);
        ret = getPktCmd2(lpPkt, &buf_cmd);
        if (ret == MY_ERROR)
        {
          Serial.println("CMD Error");
          //} else {
          //  Serial.printf("CMD2 : 0x%02x\r\n", buf_cmd);
        }

        memset(buf_dt, 0x00, sizeof(buf_dt)); // 2022.05.11

        ret = getPktData2(lpPkt, buf_dt); // return data length
        if (ret == MY_ERROR)
        {
          Serial.println("DATA Error");
          //} else {
          //  Serial.printf("DATA : %s\r\n", buf_dt);
        }

        ret = processCmd(buf_cmd, buf_dt, ret); // --> ble.ino / processCmd()
      }

      /*
      for (int i=0; i<rxValue.length(); i++) {
        //Serial.print(rxValue[i]);
        ch_data = rxValue[i];

        //ret = mkPktF(ch_data);
        ret = mkPktF2(ch_data, lpPkt, sizeof(lBufPkt));
        if (ret == sizeof(lBufPkt)) {
          memset(buf_dt, 0x00, sizeof(buf_dt));
          buf_cmd = 0x00;

          //displayPkt(lBufPkt, sizeof(lBufPkt));

          ret = getPktCmd(lBufPkt, &buf_cmd);
          if (ret == MY_ERROR) {
            Serial.println("CMD Error");
          } else {
            Serial.printf("CMD2 : 0x%02x\r\n", buf_cmd);
          }

          ret = getPktData(lBufPkt, buf_dt);
          if (ret == MY_ERROR) {
            Serial.println("DATA Error");
          //} else {
          //  Serial.printf("DATA : %s\r\n", buf_dt);
          }

          ckbyte = getCheckSum(lBufPkt, sizeof(lBufPkt)-2);
          //Serial.printf("CKSUM : 0x%02x, 0x%02x\r\n", ckbyte, lBufPkt[36]);
          Serial.printf("CKSUM : 0x%02x, 0x%02x\r\n", ckbyte, lBufPkt[MAX_DATA+4]);
          //Serial.println("## PKT O.K.");

          ret = processCmd(buf_cmd, buf_dt, ret);
        }
        //Serial.print(ch_data);
      } // for
      */
#else  // DBG_MULTI_PKT
      for (int i = 0; i < rxValue.length(); i++)
      {
        // Serial.print(rxValue[i]);
        ch_data = rxValue[i];

        // ret = mkPktF(ch_data);
        ret = mkPktF2(ch_data, lBufPkt, sizeof(lBufPkt));
        if (ret == sizeof(lBufPkt))
        {
          memset(buf_dt, 0x00, sizeof(buf_dt));
          buf_cmd = 0x00;

          // displayPkt(lBufPkt, sizeof(lBufPkt));

          ret = getPktCmd(lBufPkt, &buf_cmd);
          if (ret == MY_ERROR)
          {
            Serial.println("CMD Error");
          }
          else
          {
            Serial.printf("CMD2 : 0x%02x\r\n", buf_cmd);
          }

          ret = getPktData(lBufPkt, buf_dt);
          if (ret == MY_ERROR)
          {
            Serial.println("DATA Error");
            //} else {
            //  Serial.printf("DATA : %s\r\n", buf_dt);
          }

          ckbyte = getCheckSum(lBufPkt, sizeof(lBufPkt) - 2);
          // Serial.printf("CKSUM : 0x%02x, 0x%02x\r\n", ckbyte, lBufPkt[36]);
          Serial.printf("CKSUM : 0x%02x, 0x%02x\r\n", ckbyte, lBufPkt[MAX_DATA + 4]);
          // Serial.println("## PKT O.K.");

          ret = processCmd(buf_cmd, buf_dt, ret);
        }
        // Serial.print(ch_data);
      } // for
#endif // DBG_MULTI_PKT
    }
  }

  void onRead(BLECharacteristic *pCharacteristic)
  {
    Serial.println("onRead");
  }

  void onNotify(BLECharacteristic *pCharacteristic)
  {
    // Serial.println("onNotify");
  }

  void onStatus(BLECharacteristic *pCharacteristic)
  {
    Serial.println("onStatus");
  }
};

int getSensorData(void *pPktData)
{
  if (pPktData == NULL)
  {
    return MY_ERROR;
  }

  memcpy(pPktData, &pktData, sizeof(PKT_DATA));

  return MY_SUCCESS;
}

// ## BLE
// ##################################################################
// ##################################################################

// ##################################################################
// ##################################################################
// ## STACK SIZE
void *StackPtrAtStart;
void *StackPtrEnd;
UBaseType_t watermarkStart;

#if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 16384;
#endif

// ## STACK SIZE
// ##################################################################
// ##################################################################

// int cnvStrToHex(char *sBuf, int szBuf)
int cnvStrToHex(char *sBuf, uint32_t *pVal)
{
  uint32_t hVal;
  uint32_t tVal;
  unsigned char b;
  int len;
  int i;

  // if(sBuf == NULL || szBuf <= 0) {
  if (sBuf == NULL)
  {
    return MY_ERROR;
  }
  len = strlen(sBuf);

  hVal = 0;
  for (i = 0; i < len; i++)
  {
    b = sBuf[len - i - 1];
    if (0x30 <= b && b <= 0x39)
    {
      tVal = (uint32_t)(b - 0x30);
      hVal += tVal << (i * 4);
      continue;
    }
    if (0x41 <= b && b <= 0x46)
    {
      tVal = (uint32_t)(b - 0x37);
      hVal += tVal << (i * 4);
      continue;
    }
    if (0x61 <= b && b <= 0x66)
    {
      tVal = (uint32_t)(b - 0x57);
      hVal += tVal << (i * 4);
      continue;
    }

    return MY_ERROR;
  } // for

  *pVal = hVal;

  return i;
}

void testHex(void)
{
  uint32_t testVal = 0;
  int retVal;

  Serial.printf("CNV0 : 0x%08x\r\n", REMOCON_VAL_POWEROFF);

  retVal = cnvStrToHex((char *)"a", &testVal);
  if (retVal < 0)
  {
    Serial.printf("CNV1 Error\r\n");
  }
  else
  {
    Serial.printf("CNV1 : 0x%08x\r\n", testVal);
  }

  retVal = cnvStrToHex((char *)"ab", &testVal);
  if (retVal < 0)
  {
    Serial.printf("CNV2 Error\r\n");
  }
  else
  {
    Serial.printf("CNV2 : 0x%08x\r\n", testVal);
  }

  retVal = cnvStrToHex((char *)"ab1", &testVal);
  if (retVal < 0)
  {
    Serial.printf("CNV3 Error\r\n");
  }
  else
  {
    Serial.printf("CNV3 : 0x%08x\r\n", testVal);
  }

  retVal = cnvStrToHex((char *)"ab1k", &testVal);
  if (retVal < 0)
  {
    Serial.printf("CNV4 Error\r\n");
  }
  else
  {
    Serial.printf("CNV4 : 0x%08x\r\n", testVal);
  }

  retVal = cnvStrToHex((char *)"f0f12345", &testVal);
  if (retVal < 0)
  {
    Serial.printf("CNV5 Error\r\n");
  }
  else
  {
    Serial.printf("CNV5 : 0x%08x\r\n", testVal);
  }
}

// ##############################################################################
// ##############################################################################
// ##############################################################################

// //ze08 포름알데히드
// HardwareSerial serialEXT(1);
// HardwareSerial serialZE08(2);
// // Ze08CH2O 센서 객체 생성
// Ze08CH2O ze08{&serialZE08};
// Ze08CH2O::concentration_t ch2o;

// ##############################################################################
// ##############################################################################
// ##############################################################################
// ## SETUP

// 리셋 버튼이 연결된 GPIO 핀 번호
#define RESET_BUTTON_PIN 34

void IRAM_ATTR resetButtonPressed() {
  // 여기에 리셋 버튼이 눌렸을 때 실행할 코드를 작성하세요.
  // NVS 초기화
  nvs_flash_erase();

  // BLE Mode 
  int retVal = setNvsInt((char *)NVS_MODE, 0x01);
  if (retVal == MY_ERROR) {
    Serial.println("SET BLE Mode");
  }
}

// Semaphore
extern SemaphoreHandle_t hI2C_sem; // predefined in task.cpp

void setup()
{
  
  char devName[MAX_DEV_NAME]; // BLE Device Name
#if DBG_WEBSOCKET_FUNC
  char propServerIp[20];       // nvs   // 2022.05.11
  int propServerPort;          // nvs   // 2022.05.11
  char propFwServerIp[20];     // nvs   // 2022.05.13
  int propFwServerPort;        // nvs   // 2022.05.13
  int propBuildingId;          // nvs   // 2022.05.16
  char propRoomID[MAX_ROOMID]; // nvs
  int propRoomNum;             // nvs
  char propSSID[MAX_SSID];     // nvs
  char propKey[MAX_KEY];       // nvs
  char propIp[20];             // 000.000.000.000, 15 bytes
  char propSubnet[20];
  char propGateway[20];
  char propDNS1[20];
  char propDNS2[20];
  IPAddress local_IP;     // ESP32가 사용할 IP address             // IPAddress local_IP(192, 168, 1, 200);
  IPAddress gateway;      // Gateway IP address (공유기 IP주소)     // IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet;       // subnet mask                         // IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS;   // primary DNS server IP address       // IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS; // secondary DNS server IP address     // IPAddress secondaryDNS(8, 8, 4, 4);
#endif                    // DBG_WEBSOCKET_FUNC
#if DBG_IR_FUNC
  char propRemoteCmd[16];
#endif // DBG_IR_FUNC
  uint32_t remoconVal;
  int chipId;
  int propMode;
  int isDHCP;
  // int nvsInt;
  int retVal;

  delay(500); // wait for fw update
  delay(500); // wait for fw update
  Serial.begin(115200);
  delay(100); // wait for initialization      // 50
  Serial.println("Test");
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  // client.setCallback(callback);
  // //mqtt 와 연결

  // #########################
  // ## STACK SIZE
  void *SpStart = NULL;
  StackPtrAtStart = (void *)&SpStart;
  watermarkStart = uxTaskGetStackHighWaterMark(NULL);
  StackPtrEnd = StackPtrAtStart - watermarkStart;
  // ## STACK SIZE
  // #########################

  // Serial.println("### START ####");

  /*
  Serial.printf("PKT_DATA size is %d\r\n", sizeof(PKT_DATA));
  Serial.printf("PKT size is %d\r\n", sizeof(PKT));
  Serial.printf("float size is %d\r\n", sizeof(float));
  Serial.printf("unsigned short size is %d\r\n", sizeof(unsigned short));
  */

  // #########################
  // ## STACK SIZE
  /*
  Serial.printf("\r\n\r\nAddress of Stackpointer near start is: %p\r\n", (void *)StackPtrAtStart);
  Serial.printf("End of Stack is near: %p \r\n", (void *)StackPtrEnd);
  Serial.printf("Free Stack near start is: %d \r\n", (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);
  */
  // ## STACK SIZE
  // #########################

  memset(&applParm, 0x00, sizeof(applParm));
  memset(&taskParm1, 0x00, sizeof(taskParm1));
  memset(&taskParm2, 0x00, sizeof(taskParm2));
  memset(&pktData, 0x00, sizeof(pktData));

#if DBG_EN_OTA
  Serial.println(" ");
  Serial.println("###########################");
  Serial.printf("# IoT Edge Light Ver.%05d #\r\n", fw_ver);
  Serial.println("###########################");
  Serial.println(" ");
#endif // DBG_EN_OTA

  // ####################################
  // ####################################

  // testHex();
  // dispMacAdd();            // debug.ino
  // dispMacAdd2(macAdd);     // debug.ino

  chipId = getChipId();
  Serial.printf("ESP32 ChipId: %d\r\n", chipId);

    // GPIO 핀을 입력으로 설정
  pinMode(RESET_BUTTON_PIN, INPUT);

  // 리셋 버튼에 대한 인터럽트 핸들러 설정
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), resetButtonPressed, RISING);

  // set bluetooth mode
  setNvsInt((char*)NVS_MODE, MODE_BLUETOOTH);

  retVal = getNvsInt((char *)NVS_MODE, &propMode);
  if (retVal == MY_ERROR)
  {
    Serial.println("GET MODE Error");
    propMode = MODE_WIFI;
    propBLE = false;
  }
  else
  {
    if (propMode == MODE_WIFI) // CHECK WIFI
    {
      Serial.println("## WIFI MODE");
      propBLE = false;
    }
    else
    {
      Serial.println("## BLUETOOTH MODE");
      propBLE = true;
    }
  }

  dispFlashInfo(); // debug.ino chipSize, chipMode, chipSpeed

  // #########################
  // ## EEPROM
  //?? initPreference();     // eeprom.ino

  // #########################
  // ## ChipID
  // dispChipInfo();          // debug.ino

  // #########################
  // ##
  initUI(); // 2021.12.15 pinMode()
  led_init();

  // #if DBG_IR_FUNC
  pinMode(GPIO_BUTTON, INPUT);
  // attachInterrupt(GPIO_BUTTON, Ext_INT1_ISR, RISING);      // IR test by button
  // #endif // DBG_IR_FUNC

#if DBG_EN_MUTEX
  // Reserved
  // Create mutex before starting tasks
  mutex = xSemaphoreCreateMutex();
#endif // DBG_EN_MUTEX

#if REL_EN_ETHER_ENC
  Serial.println("#### Ethernet Init");
  Ethernet.init(USE_THIS_SS_PIN); // SPI CS Enable

  // ## DBG_EN_STATIC_IP
  // Ethernet.begin(mac, ip, gateway, subnet);
  // ## DBG_EN_DHCP
  // ## ENC28J60 DHCP Problem
  // ## https://github.com/jandrassy/EthernetENC/issues/9
  // ## https://github.com/jandrassy/EthernetENC/wiki/Limitations
  // ## Microchip ENC28J60.pdf page 48 (real page 50), ERXFCON Register
  // ## Enc28J60Network.cpp
  // ## 100:  //writeReg(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);              // ori
  // ## 101:  writeReg(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN|ERXFCON_BCEN);   // sjsim
  // Ethernet.begin(mac);

  Ethernet.begin(mac);
  Serial.println("Ethernet begin finish####");
  // if (Ethernet.begin(mac) == 0) {
  //     Serial.println("Failed to configure Ethernet using DHCP");
  //     while(1) { ; }
  // }

  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("My Gateway IP address: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("My SubnetMask IP address: ");
  Serial.println(Ethernet.subnetMask());
  // DBG_EN_DHCP

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  else
  {
    isEthernetHW = true;
  }

  while (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
    // ############# LED Display
    led4_on();
    delay(50);
    led4_off();
    delay(100);
    led4_on();
    delay(50);
    led4_off();
    // delay(100);

    delay(800);
  }
  led4_on();

#else  // REL_EN_ETHER_ENC
  Serial.println("#### Ethernet Init SKIP!!!");
  isEthernetHW = false;
#endif // REL_EN_ETHER_ENC
       // serialZE08.begin(9600, SERIAL_8N1, 4, 2);
  delay(100);

  // ####################################
  // ####################################
  // ## CHECK MODE
#if DBG_EN_CHKBLE
  // isBLE = chkBLE();
  if (propBLE == true)
  {
    isBLE = true;
  }
  else
  {
    Serial.println("## chkBLE");
    isBLE = chkBLE();
    if (isBLE == true)
    {
      Serial.println("## chkBLE BLUETOOTH MODE");
    }
    else
    {
      Serial.println("## chkBLE WIFI MODE");
    }
  }
#endif // DBG_EN_CHKBLE

#if DBG_EN_MODE_BLUETOOTH
  Serial.println("## FORCE TO BLUETOOTH MODE");
  isBLE = true;
#endif // DBG_EN_MODE_BLUETOOTH
#if DBG_EN_MODE_WEBSOCKET
  Serial.println("## FORCE TO WIFI(WEBSOCKET) MODE");
  isBLE = false;
#endif // DBG_EN_MODE_BLUETOOTH
  // ## CHECK MODE
  // ####################################
  // ####################################

  // ####################################
  // ####################################
  // ## NVS, Load Setting

#if DBG_EN_NVS
  initNVS(); // nvs.ino   // move from below

#endif       // DBG_EN_NVS

#if DBG_IR_FUNC
  // retVal = setNvsStr((char *)NVS_RMTDT_POWEROFF, value);
  retVal = getNvsStr((char *)NVS_RMTDT_POWEROFF, propRemoteCmd);
  if (retVal < 0)
  {
    Serial.println("GET RMT POWEROFF Error");
    sprintf(propRemoteCmd, "%08x", REMOCON_VAL_POWEROFF); // Set Default
    setNvsStr((char *)NVS_RMTDT_POWEROFF, propRemoteCmd);
  }

  remoconVal = 0;
  retVal = cnvStrToHex(propRemoteCmd, &remoconVal);
  if (retVal < 0)
  {
    Serial.printf("RMT POWEROFF Value Error\r\n");
  }
  else
  {
    Serial.printf("RMT POWEROFF Value : 0x%08x\r\n", remoconVal);
    Ac_Set_Cmd(REMOCON_CMD_POWEROFF, remoconVal);
  }

  // retVal = setNvsStr((char *)NVS_RMTDT_POWERON, value);
  retVal = getNvsStr((char *)NVS_RMTDT_POWERON, propRemoteCmd);
  if (retVal < 0)
  {
    Serial.println("GET RMT POWERON Error");
    sprintf(propRemoteCmd, "%08x", REMOCON_VAL_POWERON); // Set Default
    setNvsStr((char *)NVS_RMTDT_POWERON, propRemoteCmd);
  }

  remoconVal = 0;
  retVal = cnvStrToHex(propRemoteCmd, &remoconVal);
  if (retVal < 0)
  {
    Serial.printf("RMT POWERON Value Error\r\n");
  }
  else
  {
    Serial.printf("RMT POWERON Value : 0x%08x\r\n", remoconVal);
    Ac_Set_Cmd(REMOCON_CMD_POWERON, remoconVal);
  }
#endif // DBG_IR_FUNC

#if DBG_WEBSOCKET_FUNC
  if (isBLE == false)
  {

    // ## Load setting for SOCKETIO

    // 2022.05.11
    retVal = getNvsStr((char *)NVS_SERVER_IP, propServerIp);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET SERVER IP Error");
      strcpy(propServerIp, DEFAULT_SERVER_IP); //
      //} else {
    }

    // 2022.05.11
    retVal = getNvsInt((char *)NVS_SERVER_PORT, &propServerPort);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET SERVER Port Error");
      propServerPort = DEFAULT_SERVER_PORT;
      //} else {
    }

    // 2022.05.11
    retVal = getNvsStr((char *)NVS_FWSERVER_IP, propFwServerIp);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET FWSERVER IP Error");
      strcpy(propFwServerIp, DEFAULT_FWSERVER_IP); //
      //} else {
    }
#if DBG_EN_OTA
    strcpy(upd_server_ip, propFwServerIp);
#endif // DBG_EN_OTA

    // 2022.05.11
    retVal = getNvsInt((char *)NVS_FWSERVER_PORT, &propFwServerPort);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET FWSERVER Port Error");
      propFwServerPort = DEFAULT_FWSERVER_PORT;
      //} else {
    }
#if DBG_EN_OTA
    upd_server_port = propFwServerPort;
#endif // DBG_EN_OTA

    // 2022.05.16
    retVal = getNvsInt((char *)NVS_BUILDINGID, &propBuildingId);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET BUILDING Error");
      propBuildingId = DEFAULT_BUILDING_ID;
      //} else {
    }
    global_building_id = propBuildingId;

    retVal = getNvsStr((char *)NVS_ROOMID, propRoomID);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET ROOMID Error");
      strcpy(propRoomID, DEFAULT_ROOMID); //
      //} else {
    }

    retVal = getNvsInt((char *)NVS_ROOMNUM, &propRoomNum);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET ROOMNUM Error");
      propRoomNum = DEFAULT_ROOMNUM;
      //} else {
    }

    retVal = getNvsStr((char *)NVS_SSID, propSSID);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET SSID Error");
      strcpy(propSSID, DEFAULT_SSID); // STATION_SSID
      //} else {
    }

    retVal = getNvsStr((char *)NVS_KEY, propKey);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET SSID Error");
      strcpy(propKey, DEFAULT_KEY); // STATION_PASSWORD
      //} else {
    }

    // test
    strcpy(propSSID, DEFAULT_SSID);
    strcpy(propKey, DEFAULT_KEY);

    retVal = getNvsInt((char *)NVS_DHCP, &isDHCP);
    if (retVal == MY_ERROR)
    {
      Serial.println("GET DHCP Error");
      isDHCP = DHCP_ON; // DHCP Enable
      //} else {
    }
#if DBG_EN_DHCP_ON
    isDHCP = DHCP_ON;
#endif // DBG_EN_DHCP_ON

    // Serial.printf("## ROOM:%s, SSID:%s, KEY:%s\r\n", propRoomID, propSSID, propKey);
    Serial.printf("## ROOMNUM:%d, SSID:%s, KEY:%s\r\n", propRoomNum, propSSID, propKey);

    if (isDHCP == DHCP_OFF)
    {
      // ## STATIC IP
      retVal = getNvsStr((char *)NVS_IP, propIp);
      if (retVal == MY_ERROR)
      {
        Serial.println("GET IP ADD Error");
        strcpy(propIp, DEFAULT_IP);
        //} else {
      }

      retVal = getNvsStr((char *)NVS_SUBNET, propSubnet);
      if (retVal == MY_ERROR)
      {
        Serial.println("GET SUBNET Error");
        strcpy(propSubnet, DEFAULT_SUBNET);
        //} else {
      }

      retVal = getNvsStr((char *)NVS_GATEWAY, propGateway);
      if (retVal == MY_ERROR)
      {
        Serial.println("GET GATEWAY Error");
        strcpy(propGateway, DEFAULT_GATEWAY);
        //} else {
      }

      retVal = getNvsStr((char *)NVS_DNS, propDNS1);
      if (retVal == MY_ERROR)
      {
        Serial.println("GET DNS1 Error");
        strcpy(propDNS1, DEFAULT_DNS1);
        //} else {
      }

      // retVal = getNvsStr((char *)NVS_DNS2, propDNS2);
      // if (retVal == MY_ERROR) {
      //   Serial.println("GET DNS2 Error");
      // } else {
      strcpy(propDNS2, DEFAULT_DNS2);
      //}

      local_IP.fromString(propIp);
      gateway.fromString(propSubnet);
      subnet.fromString(propGateway);
      primaryDNS.fromString(propDNS1);
      secondaryDNS.fromString(propDNS2);

      // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      //   Serial.println("STA failed to configure");
      // }
    }

// 2022.10.25
#if DBG_FIX_SSID
    Serial.println("FIX SSID");
    strcpy(propSSID, DEFAULT_SSID); // STATION_SSID
    Serial.println("FIX KEY");
    strcpy(propKey, DEFAULT_KEY); // STATION_PASSWORD
#endif                            // DBG_FIX_SSID

// 2022.10.25
#if DBG_FIX_SERVER
    Serial.println("FIX SERVER IP");
    strcpy(propServerIp, DEFAULT_SERVER_IP); //
    Serial.println("FIX SERVER Port");
    propServerPort = DEFAULT_SERVER_PORT;
#endif // DBG_FIX_SERVER

  } // if(isBLE == false)

#endif // DBG_WEBSOCKET_FUNC

  // ## NVS, Load Setting
  // ####################################
  // ####################################

#if REL_EN_DISP_SSD
  Serial.println("#### SSD1306 Init");
  // initialize OLED
  Wire.begin(21, 22, (uint32_t)90000);
  display.init();
  // display.setFont(ArialMT_Plain_16);
  display.flipScreenVertically();
  display.setContrast(255);
  // display.setBrightness(255);
  // display.invertDisplay();
  display.clear();

  // sjsim
  display.setLogBuffer(5, 30);
  display.println(F("Starting WebServer on 1"));
  display.drawLogBuffer(0, 0);
  display.display();

#else  // REL_EN_DISP_SSD
  Serial.println("#### Display Skip");
#endif // REL_EN_DISP_SSD

  Serial.println("## INIT SENSOR");
  init_sensor(); // sensor.ino

  // init_ze08(&serialZE08);

  Serial.println("## INIT RFID");
  init_rfid(); // sensor.ino

#if DBG_I2S
  I2S.setAllPins();
  I2S.setDuplex();
  // Init in MASTER mode
  if (!I2S.begin(I2S_PHILIPS_MODE, 8000, 16)) 
  {
      Serial.println("Failed to initialize I2S!");
  }
  else
  {
    Serial.println("Initialize I2S!");
  }

#endif

#if DBG_IR_FUNC
  // IR setup
  Serial.println("## INIT REMOCON");
  RemoconInit();
#endif // DBG_IR_FUNC

#if DBG_TASK_QUEUE
  // ## sjsim, 2021.11.26, Task communication
  queueRemote = xQueueCreate(queueRemoteSize, sizeof(int));
  if (queueRemote == NULL)
  {
    Serial.println("Error creating the queueRemote");
  }
#endif // DBG_TASK_QUEUE

#if DBG_TASK_QUEUE2
  // ## sjsim, 2022.01.26, Task communication for PKT
  queuePkt = xQueueCreate(queuePktSize, sizeof(PKT));
  if (queuePkt == NULL)
  {
    Serial.println("Error creating the queuePkt");
  }
#endif // DBG_TASK_QUEUE2

  delay(1000);

  applParm.isBLE = isBLE;
  applParm.intervalSocketIOEvent = INTERVAL_SOCKETIO_EVENT; // 4000ms (4s)   // websocket.ino / runSocketIO()
  applParm.lpPktData = &pktData;

  // ####################################
  // ####################################
  // ## TASK

  // ## TASK
  // ####################################
  // ####################################

  // ####################################
  // ####################################
  // ## BLE
  if (isBLE == true)
  {
    Serial.println("## INIT BLE");

    // Create the BLE Device

    // BLEDevice::init(DEFAULT_BLE_DEVICE_NAME);      // "MyESP32"
    sprintf(devName, "%s_%06x_v%d", DEFAULT_BLE_DEVICE_NAME, chipId, fw_ver);
    BLEDevice::init(devName); // KOUNO_EA05E8

    // BLEDevice::setMTU(46);                         // ESP32 Default : 23 Bytes   // 2022.03.29

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    // MyCallbacks *lpMyCallbacks = new MyCallbacks()
    // taskParm1
    // pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setCallbacks(new MyCallbacks((void *)&taskParm1)); // 2022.05.27

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising #1
    // pServer->getAdvertising()->start();
    // Serial.println("Waiting a client connection to notify...");

    // Start advertising #2
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);

    // pAdvertising->setScanResponse(false);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    /*
    //## functions that help with iPhone connections issue
    pAdvertising->setScanResponse(true);
    //pAdvertising->setMinPreferred(0x6);  // set value to 0x00 to not advertise this parameter
    pAdvertising->setMinPreferred(0x12);  // set value to 0x00 to not advertise this parameter
    */

    BLEDevice::startAdvertising();

    Serial.println("Waiting a client connection to notify...");
  }
  // ## BLE
  // ####################################
  // ####################################

  // ####################################
  // ####################################
  // ## SOCKETIO
  // #if DBG_WEBSOCKET_FUNC
  else
  {

    // 2022.03.02 ????????????????????????????????????????
#if 0 // 2022.05.17
    if (isDHCP == DHCP_OFF) {
      if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA failed to configure");
      }
    }
#endif

#if 0
    // 2022.03.02 ????????????????????????????????????????
    //WiFi.begin(STATION_SSID, STATION_PASSWORD);
    WiFi.begin(propSSID, propKey);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("...");
    }
#endif

#if DBG_EN_OTA
    HttpsOTA.onHttpEvent(HttpEvent);
    Serial.println("Starting OTA");
    HttpsOTA.begin(buf_url, server_certificate);
    //HttpsOTA.begin(buf_url, server_certificate, false);

    Serial.println("Please Wait it takes some time ...");
#endif // DBG_EN_OTA

    // from task.ino / task1()   2022.03.02
    // initSocketIO(&taskParm1);
  }
  // #endif // DBG_WEBSOCKET_FUNC

  // ## SOCKETIO
  // ####################################
  // ####################################

  // ####################################
  // ####################################
  // ## TASK
  taskParm1.isBLE = isBLE;
  taskParm1.isWiFiConnected = false; // 2022.05.17
  taskParm1.isOTA = true;            // 2022.05.17
  taskParm1.isOTAStarted = false;    // 2022.05.17
  taskParm1.propBLE = propBLE;
  taskParm1.delayTask = MS(10); // 10ms     //100ms    // MS(5),  5ms
  // taskParm1.skipcoSendSensor = SKIP_SEND_SERSOR;               // 100 -> run at every 10s (at 100ms delay, depend on delayTask)
  taskParm1.skipcoSendSensor = 1000;                         // 100 -> run at every 10s (at 100ms delay, depend on delayTask)
  taskParm1.intervalSocketIOEvent = INTERVAL_SOCKETIO_EVENT; // 4000ms (4s)   // websocket.ino / runSocketIO()
  taskParm1.lpPktData = &pktData;
  taskParm1.buildingid = global_building_id; // 2022.02.28
  // taskParm1.roomid = global_room_id;                           // 2022.02.28
  taskParm1.roomid = propRoomNum;               // 2022.03.29
  taskParm1.fw_ver = fw_ver;                    // 2022.05.18
  taskParm1.event_sensor = EVENT_SENSOR_SERVER; // 2022.05.18
  taskParm1.isSensorEvent = false;              // 2022.05.18

  // taskParm1.svr_port = WEBSOCKET_PORT;
  taskParm1.svr_port = propServerPort; // 2022.05.11

  strcpy(taskParm1.room_id_str, propRoomID);
  // strcpy(taskParm1.svr_ip, WEBSOCKET_SVRIP);
  strcpy(taskParm1.svr_ip, propServerIp); // 2022.05.11
  strcpy(taskParm1.ssid, propSSID);
  strcpy(taskParm1.key, propKey);

  taskParm2.isBLE = isBLE;
  taskParm2.isWiFiConnected = false; // 2022.05.17
  taskParm2.isOTA = true;            // 2022.05.17
  taskParm2.isOTAStarted = false;    // 2022.05.17
  taskParm2.propBLE = propBLE;
  taskParm2.delayTask = MS(100); // 100ms
  // taskParm2.timeSensorSend = INTERVAL_SENSOR_SEND;             // 2022.05.18
  taskParm2.skipcoSendSensor = SKIP_SEND_SERSOR;            // 100 -> run at every 10s (at 100ms delay, depend on delayTask)
  taskParm2.skipcoUpdateSensor = SKIP_UPDATE_SENSOR;        // 50 -> run at every 5s (at 100ms delay, depend on delayTask)
  taskParm2.skipcoUpdateSensorI2C = SKIP_UPDATE_SENSOR_I2C; //

  taskParm2.lpPktData = &pktData;
  taskParm2.buildingid = global_building_id; // 2022.02.28
  // taskParm1.roomid = global_room_id;                           // 2022.02.28
  taskParm2.roomid = propRoomNum;               // 2022.03.29
  taskParm2.fw_ver = fw_ver;                    // 2022.05.18
  taskParm2.event_sensor = EVENT_SENSOR_SERVER; // 2022.05.18
  taskParm2.isSensorEvent = false;              // 2022.05.18

  // Semaphore
  hI2C_sem = xSemaphoreCreateBinary();

  if(hI2C_sem != NULL)
  {
      xSemaphoreGive(hI2C_sem);
  }

#if 1
  // ## BLE, SOCKETIO TASK
  xTaskCreatePinnedToCore(
      task1,      // Task Function       // task.ino
      "task1",    // Task Namez
      4096,      // Stack Size          // 32768, 8192
      &taskParm1, // Task Parameter
      1,          // Task Priority
      NULL,       // Task Handle
      PRO_CPU_NUM // Task Core
  );
#endif

#if 1
  // ## SENSOR TASK
  xTaskCreatePinnedToCore(
      task2,      // Task Function
      "task2",    // Task Name
      4096,      // Stack Size          // 8192
      &taskParm2, // Task Parameter
      1,          // Task Priority
      NULL,       // Task Handle
      APP_CPU_NUM // Task Core
  );
#endif

#if 1
  // ## MIC I2C TASK
  xTaskCreatePinnedToCore(
      taskSensorI2C,
      "SensorI2C",
      2048,
      NULL,
      tskIDLE_PRIORITY + 3,
      NULL,
      PRO_CPU_NUM
  );
#endif

#if 0
  // ## MIC I2S TASK
  xTaskCreatePinnedToCore(
      taskSensorI2S,
      "SensorI2S",
      2048,
      NULL,
      tskIDLE_PRIORITY + 1,
      NULL,
      PRO_CPU_NUM
  );
#endif

}
// ## SETUP
// ##############################################################################
// ##############################################################################
// ##############################################################################

// ##############################################################################
// ##############################################################################
// ##############################################################################
// ## LOOP

int ze08count = 0;
int32_t audio_buffer[8000];
int dmic_sample = 0;
byte data[2];

void taskSensorI2C(void *parameter) {
  // I2C로 마이크 데이터 읽기
  float ads7828_val = 0;
  int raw_adc;
  ads7828_val = get_ads7828(0xb4);
  Serial.println(ads7828_val);
  PKT_SOUND pktSound;
  for (;;) 
  {
    Wire.beginTransmission(0x48);
    Wire.write(0xb4);
    Wire.endTransmission();
    Wire.requestFrom(0x48, 2);
    if (Wire.available() == 2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
    }
    raw_adc = ((data[0] & 0x0F) * 256) + data[1];
    // Serial.println(raw_adc);

    if (raw_adc > 50)
    {
      pktSound.sound = raw_adc;
      pktData.sound = raw_adc;
      //Serial.printf("mic_test raw_adc : %d\n", raw_adc);
      //Serial.printf("mic_test data[0] : %d\n", data[0]);
      //Serial.printf("mic_test data[1] : %d\n", data[1]);
    }
    vTaskDelay(10);
  }
}

void loop()
{

  // void taskSensorI2S(void *parameter)
  // {
  //   int sample = 0;
  //   for(;;)
  //   {
  //       sample = I2S.read();
  //       Serial.println(sample);
  //       if (sample && sample != -1 && sample != 1)
  //           dmic_sample = sample;
  //       vTaskDelay(10 / portTICK_PERIOD_MS);
  //   }
  // }

  // notify changed value

  // -> ble.ino / sendBtBin()
  // ze08count++;
  // ze08.read(ch2o);
  // Serial.printf("########ze80 : %d\n", ch2o);

  // if(ze08count == 100){
  //   ze08count = 0;
  //   ch2o = 0;
  //   //ze08.read(ch2o);
  //   Serial.printf("########ze80 : %d\n", ch2o);
  // }

  // if (deviceConnected) {
  //   Serial.println("send bluetooth..");
  //   String s = "notification string";
  //   pCharacteristic->setValue(s.c_str());
  //   pCharacteristic->notify();
  //   value++;
  //   delay(2000); // bluetooth stack will go into congestion, if too many packets are sent
  // }

  // //mqtt part
  // if (!client.connected()) {
  //   reconnect();
  // }
  // client.loop();

  // // MQTT 메시지 발행 예시
  // char msg[] = "Hello, MQTT!";
  // client.publish("your/topic", msg);
  // delay(1000);
  // //mqtt part
  
#if DBG_TASK_QUEUE2
    if (xQueueReceive(queuePkt, buf_rcvpkt, (TickType_t)0))
    { // Immediately return
      Serial.print("#### QUEUE Recv PKT: ");
      for (int i = 0; i < sizeof(PKT); i++)
      {
        Serial.printf("0x%02x,", buf_rcvpkt[i]);
      }
      Serial.println(" ");

      /*
      if (deviceConnected) {
        pCharacteristic->setValue((uint8_t*)buf_rcvpkt, sizeof(PKT));
        pCharacteristic->notify();
        value++;
        delay(2000); // bluetooth stack will go into congestion, if too many packets are sent
      }
      */
      sendBLE(buf_rcvpkt, sizeof(PKT));
    }
#endif // DBG_TASK_QUEUE2

  // runSensor();      // loop.ino       // task2() -> sensor.ino / update_sensor()

  if (isBLE == true)
  {
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
      delay(500);                  // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }

    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
    }
  }
  else
  {

#if DBG_EN_OTA_SERVER
    if (taskParm1.isWiFiConnected == true)
    {
      taskParm2.isWiFiConnected = true;
    }
#endif // DBG_EN_OTA_SERVER

#if DBG_EN_OTA
    if (taskParm1.isWiFiConnected == true && taskParm1.isOTAStarted == false)
    {
      taskParm1.isOTAStarted = true;

      // http://203.251.137.137:49001/d594b23abec72a4262ba2a2e41b4db06/kn_light_00001.bin
      if (upd_server_port == 80)
      {
        sprintf(buf_url, "http://%s/%s/%s_%05d.bin", upd_server_ip, md5_str, model, fw_ver + 1);
      }
      else
      {
        sprintf(buf_url, "http://%s:%d/%s/%s_%05d.bin", upd_server_ip, upd_server_port, md5_str, model, fw_ver + 1);
      }
      // Serial.println(buf_url);

      HttpsOTA.onHttpEvent(HttpEvent);
      Serial.println("Starting OTA");
      HttpsOTA.begin(buf_url, server_certificate);
      // HttpsOTA.begin(buf_url, server_certificate, false);

      Serial.println("Please Wait it takes some time ...");
    }
#endif // DBG_EN_OTA

#if DBG_EN_OTA
    if (taskParm1.isOTA == true)
    {
      otastatus = HttpsOTA.status();
      if (otastatus == HTTPS_OTA_SUCCESS)
      {
        Serial.println("Firmware written successfully. To reboot device, call API ESP.restart() or PUSH restart button on device");
        reboot();
      }
      else if (otastatus == HTTPS_OTA_FAIL)
      {
        Serial.println("Firmware Upgrade Fail");
        taskParm1.isOTA = false;
      }
    }
#else  // DBG_EN_OTA
      taskParm1.isOTA = false;
#endif // DBG_EN_OTA

#if DBG_WEBSOCKET_FUNC
    // from task.ino / task1()   2022.03.02
    // 2022.05.17   runSocketLoop();                    // success
    // 2022.05.17
    // if (taskParm1.isOTA == false) {
    // OTA Process가 종료되었으면....
    runSocketLoop();
    //}
#endif // DBG_WEBSOCKET_FUNC
  }
  // 기존 btn1 누를시 초기화 되던 부분 삭제 (23/09/20)
  // if (btn_status() == true)
  // {
  //   Serial.println("####### RESET BUTTON & REBOOT #######");
  //   reboot();
  // }

  // ## LED Display
  if (isBLE == true)
  {
    led_ble();
    //Serial.println("led ble");
  }
  else
  {
    if (getConnectionStatus() == true)
    {
      led_wifi_data();
    }
    else
    {
      led_wifi();
    }
  }

  // delay(1);                             // 1ms
  delay(10); // 10ms
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float o3 = taskParm2.lpPktData->mq131;
  float co = taskParm2.lpPktData->co;
  float no2 = taskParm2.lpPktData->no2;
  float nh3 = taskParm2.lpPktData->nh3;
  float c3h8 = taskParm2.lpPktData->c3h8;
  float c4h10 = taskParm2.lpPktData->c4h10;
  float ch4 = taskParm2.lpPktData->ch4;
  float h2 = taskParm2.lpPktData->h2;
  float c2h5oh = taskParm2.lpPktData->c2h5oh;
  float ch2o = taskParm2.lpPktData->ch2o;
  uint16_t srawVoc = taskParm2.lpPktData->voc;
  float co2 = taskParm2.lpPktData->co2;
  float temperature = taskParm2.lpPktData->temp;
  float humidity = taskParm2.lpPktData->humi;
  char messageChar[255];
  int len = snprintf(messageChar, sizeof(messageChar),"VOC:%u CH2O:%.4f MQ131:%.4f CO:%.4f NO2:%.4f NH3:%.4f C3H8:%.4f C4H10:%.4f CH4:%.4f H2:%.4f C2H5OH:%.4f CO2:%.4f Temperature:%.4f Humidity:%.4f", srawVoc, ch2o, o3, co, no2, nh3, c3h8, c4h10, ch4, h2, c2h5oh, co2, temperature, humidity);
  if (len < 0) {
      // snprintf에서 에러 발생
      Serial.printf("Error generating message\n");
  } else if (len >= sizeof(messageChar)) {
      // 메시지가 버퍼를 초과함
      Serial.printf("Message truncated\n");
  } else {
      // 정상적으로 메시지 생성됨
      client.publish("jude", messageChar);
  }


#if DBG_I2S
  const int buffer_size = 1024;
  uint8_t buffer[1024];
  int bytes_read = I2S.read();
  size_t bytes_written;

  Serial.println(bytes_read);
  delay(2000);
  
  // // size_t I2SClass::write(const void *buffer, size_t size)
  bytes_written = I2S.write((const void*)bytes_read, buffer_size);
  


  // Serial.println("#### audio_test");
  // DacAudio.FillBuffer();                
  // if(Sound.Playing==false)    {
  //   Serial.print("재생");
  //   DacAudio.Play(&Sound);   
  // }
    
  // Serial.println(DemoCounter++);


  

#endif
}
// ## LOOP
// ##############################################################################
// ##############################################################################
// ##############################################################################
