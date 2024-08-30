#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <MiCS6814-I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ze08_ch2o.h>
#include <SensirionI2CSgp40.h>

const char* ssid = "Kouno_Zone_2G";
const char* password = "1092724855";
const char* mqtt_server = "10.0.0.70";

SensirionI2CSgp40 sgp40;
Ze08CH2O ze80{&Serial2};
Ze08CH2O::concentration_t ch2o;
WiFiClient espClient;
PubSubClient client(espClient);
SensirionI2CScd4x scd4x;
MiCS6814 sensor;

bool sensorConnected;
int preheatTime = 10;
long lastMsg = 0;
char msg[63];
int value = 0;
uint16_t basenh3;
uint16_t basered;
uint16_t baseox;
float co= 0.0000f;
float no2= 0.0000f;
float nh3= 0.0000f;
float c3h8= 0.0000f;
float c4h10= 0.0000f;
float ch4= 0.0000f;
float h2= 0.0000f;
float c2h5oh= 0.0000f;
float valueR0 = 1917.22;
int secToRead = 80;
float valueRL = 1000;
float lastValueRs;
float o3;

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

int get_ads7828(uint8_t ch_val){
  byte data[2];
  int raw_adc;
  
  raw_adc = -1;
  
  // Start I2C Transmission
  Wire.beginTransmission(0x48);

  //Wire.write(0x04);
  Wire.write(ch_val);
  // Stop I2C transmission
  Wire.endTransmission();
  
  // Request 2 bytes of data
  Wire.requestFrom(0x48, 2);
  
  // Read 2 bytes of data
  // raw_adc msb, raw_adc lsb
  if(Wire.available() == 2) {
    data[0] = Wire.read();
    data[1] = Wire.read();
    
    //delay(300);
    
    // Converting the data to 12 bits
    raw_adc = ((data[0] & 0x0F) * 256) + data[1];
    // raw_adc = ((data[0] & 0x0F) * 64) + data[1];
    // Output data to serial monitor
    // Serial.print("Digital value of analog input : ");
    // Serial.println(raw_adc);
  }

  return raw_adc;
}

float readRs() {
  uint8_t valueSensor = get_ads7828(0xa4);
  float vRL = ((float)valueSensor) / 4048.0 * 5.0;
  if(!vRL) return 0.0f; //division by zero prevention
  float rS = (5.0 / vRL - 1.0) * valueRL;
  return rS;
}

float getO3(int unit) {
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

  float ratio = 0.0;
  ratio = lastValueRs / valueR0 * (-0.0103 * 30 + 1.1507); // 온습도 30 , 55 로 가정
  // float MQ131Class::getEnvCorrectRatio() {
  //  	// Select the right equation based on humidity
  //  	// If default value, ignore correction ratio
  //  	if(humidityPercent == 60 && temperatureCelsuis == 20) {
  //  		return 1.0;
  //  	}
  //  	// For humidity > 75%, use the 85% curve
  //  	if(humidityPercent > 75) {
  //     // R^2 = 0.996
  //    	return -0.0103 * temperatureCelsuis + 1.1507;
  //  	}
  //  	// For humidity > 50%, use the 60% curve
  //  	if(humidityPercent > 50) {
  //  		// R^2 = 0.9976
  //  		return -0.0119 * temperatureCelsuis + 1.3261;
  //  	}

  //  	// Humidity < 50%, use the 30% curve
  //   // R^2 = 0.9986
  //  	return -0.0141 * temperatureCelsuis + 1.5623;
  //  }

  return convert(9.4783 * pow(ratio, 2.3348), 1, unit);
}

float convert(float input, int unitIn, int unitOut) {
  if(unitIn == unitOut) {
    return input;
  }

  float concentration = 0;

  switch(unitOut) {
    case 0 :
      // We assume that the unit IN is PPB as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPB to PPM
      return input / 1000.0;
    case 1 :
      // We assume that the unit IN is PPM as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPM to PPB
      return input * 1000.0;
    case 2 :
      if(unitIn == 0) {
        concentration = input;
      } else {
        concentration = input / 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    case 3 :
      if(unitIn == 1) {
        concentration = input;
      } else {
        concentration = input * 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    default :
      return input;
  }
}

void setup() {
  Serial.begin(115200);

  delay(1000);
  sensorConnected = sensor.begin();

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);
  sgp40.begin(Wire);

  if (sensorConnected == true) {
    // Print status message
    Serial.println("Connected to MiCS-6814 sensor");

    // Turn heater element on
    // sensor.powerOn();

    // Serial.print("Preheating sensor ... ");

    // // Wait the required amount of time, printing
    // // status every minute
    // while (preheatTime > 0) {
    //     Serial.print(preheatTime);
    //     Serial.print(" ... ");
        
    //     // Wait a minute
    //     delay(10000);

    //     preheatTime = preheatTime - 1;
    // }
    // Serial.println("done!");
    // Print header for live values
    Serial.println("Current concentrations:");
    Serial.println("CO\tNO2\tNH3\tC3H8\tC4H10\tCH4\tH2\tC2H5OH");
    // sensor.calibrate();
    basenh3 = sensor.getBaseResistance(CH_NH3);
    basered = sensor.getBaseResistance(CH_RED);
    baseox = sensor.getBaseResistance(CH_OX);

    // Print new base resistances stored in the sensor
    Serial.println("Calibration done");
    Serial.println("New base resistances:");
    Serial.print("NH3: ");
    Serial.print(basenh3);
    Serial.print("\tRED: ");
    Serial.print(basered);
    Serial.print("\tOX: ");
    Serial.println(baseox);
    co = sensor.measureCO();
    no2 = sensor.measureNO2();
    nh3 = sensor.measureNH3();
    c3h8 = sensor.measureC3H8();
    c4h10 = sensor.measureC4H10();
    ch4 = sensor.measureCH4();
    h2 = sensor.measureH2();
    c2h5oh = sensor.measureC2H5OH();

    } else {
      // Print error message on failed connection
      Serial.println("Couldn't connect to MiCS-6814 sensor");
  }

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  lastValueRs = readRs();
  uint16_t error;
  char errorMessage[256];

  // Read Measurement
  uint16_t co2 = 0;
  float temperature = 0.0f;
  float humidity = 0.0f;
  bool isDataReady = false;
  error = scd4x.getDataReadyFlag(isDataReady);
  if (error) {
      Serial.print("Error trying to execute getDataReadyFlag(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      return;
  }
  if (!isDataReady) {
      return;
  }
  error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error) {
      Serial.print("Error trying to execute readMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  } else if (co2 == 0) {
      Serial.println("Invalid sample detected, skipping.");
  } else {
      // Serial.print("Co2:");
      // Serial.print(co2);
      // Serial.print("\t");
      // Serial.print("Temperature:");
      // Serial.print(temperature);
      // Serial.print("\t");
      // Serial.print("Humidity:");
      // Serial.println(humidity);
  }

  uint16_t defaultRh = 0x8000;
  uint16_t defaultT = 0x6666;
  uint16_t srawVoc = 0;

  error = sgp40.measureRawSignal(defaultRh, defaultT, srawVoc);
  if (error) {
    Serial.print("Error trying to execute measureRawSignal(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    // Serial.print("SRAW_VOC:");
    // Serial.println(srawVoc);
  }

  long now = millis();
  if (now - lastMsg > 500) {
    
    lastMsg = now;
    o3 = getO3(0);
    co = sensor.measureCO();
    no2 = sensor.measureNO2();
    nh3 = sensor.measureNH3();
    c3h8 = sensor.measureC3H8();
    c4h10 = sensor.measureC4H10();
    ch4 = sensor.measureCH4();
    h2 = sensor.measureH2();
    c2h5oh = sensor.measureC2H5OH();
    ze80.read(ch2o);
    char messageChar[255];
    int len = snprintf(messageChar, sizeof(messageChar),"VOC:%u CH2O:%.4f O3:%.4f CO:%.4f NO2:%.4f NH3:%.4f C3H8:%.4f C4H10:%.4f CH4:%.4f H2:%.4f C2H5OH:%.4f CO2:%.4f Temperature:%.4f Humidity:%.4f", srawVoc, ch2o, o3, co, no2, nh3, c3h8, c4h10, ch4, h2, c2h5oh, co2, temperature, humidity);
    if (len < 0) {
        // snprintf에서 에러 발생
        Serial.printf("Error generating message\n");
    } else if (len >= sizeof(messageChar)) {
        // 메시지가 버퍼를 초과함
        Serial.printf("Message truncated\n");
    } else {
        // 정상적으로 메시지 생성됨
        client.publish("jude", messageChar);
        Serial.print("hellow");
    }
  }
}