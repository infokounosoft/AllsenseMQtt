/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
#include <MiCS6814-I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

const char* ssid = "Kouno_Zone_2G";
const char* password = "1092724855";

const char* mqtt_server = "10.0.0.70";

WiFiClient espClient;
PubSubClient client(espClient);

MiCS6814 sensor;
bool sensorConnected;
int preheatTime = 10;
long lastMsg = 0;
char msg[63];
int value = 0;

void setup() {
  Serial.begin(115200);

  delay(1000);
  sensorConnected = sensor.begin();

  if (sensorConnected == true) {
    // Print status message
    Serial.println("Connected to MiCS-6814 sensor");

    // Turn heater element on
    sensor.powerOn();

    Serial.print("Preheating sensor ... ");

    // Wait the required amount of time, printing
    // status every minute
    while (preheatTime > 0) {
        Serial.print(preheatTime);
        Serial.print(" ... ");
        
        // Wait a minute
        delay(10000);

        preheatTime = preheatTime - 1;
    }
    Serial.println("done!");
    // Print header for live values
    Serial.println("Current concentrations:");
    Serial.println("CO\tNO2\tNH3\tC3H8\tC4H10\tCH4\tH2\tC2H5OH");
    sensor.calibrate();
    Serial.println("Calibration done");
    Serial.println("New base resistances:");
    Serial.print("NH3: ");
    Serial.print(sensor.getBaseResistance(CH_NH3));
    Serial.print("\tRED: ");
    Serial.print(sensor.getBaseResistance(CH_RED));
    Serial.print("\tOX: ");
    Serial.println(sensor.getBaseResistance(CH_OX));

    } else {
      // Print error message on failed connection
      Serial.println("Couldn't connect to MiCS-6814 sensor");
  }


  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

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
    Serial.print("data recieved ");
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
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;
    
    int co = sensor.measureCO();
    int no2 = sensor.measureNO2();
    int nh3 = sensor.measureNH3();
    int c3h8 = sensor.measureC3H8();
    int c4h10 = sensor.measureC4H10();
    int ch4 = sensor.measureCH4();
    int h2 = sensor.measureH2();
    int c2h5oh = sensor.measureC2H5OH();
    String Message = "CO: ";
    char senString[50];
    dtostrf(co, 1, 2, senString);
    Message += String(senString);
    Message += " NO2: ";
    dtostrf(no2, 1, 2, senString);
    Message += String(senString);
    Message += " NH3: ";
    dtostrf(nh3, 1, 2, senString);
    Message += String(senString);
    Message += " C3H8: ";
    dtostrf(c3h8, 1, 2, senString);
    Message += String(senString);
    Message += " C4h10: ";
    dtostrf(c4h10, 1, 2, senString);
    Message += String(senString);
    Message += " CH4: ";
    dtostrf(ch4, 1, 2, senString);
    Message += String(senString);
    Message += " H2: ";
    dtostrf(h2, 1, 2, senString);
    Message += String(senString);
    Message += " C2H5OH: ";
    dtostrf(c2h5oh, 1, 2, senString);
    Message += String(senString);
    
    char messageChar[255];
    strcpy(messageChar, Message.c_str());
    client.publish("jude", messageChar);


  }
}