#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

char data[512] = {0};
int count = 0;

WiFiClient client;
PubSubClient mqttclient(client);
SoftwareSerial softSerial(12, 13);

const char* ssid = "wifi-ssid";
const char* password = "wifi-passwd";
const char* mqtt_server = "mqtt server name";

void setup() {
  Serial.begin(115200);
  softSerial.begin(9600);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  mqttclient.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (!mqttclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttclient.connect("onsemiesp8266")) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {  
  if (!mqttclient.connected()) {
    reconnect();
  }
  mqttclient.loop();

  while (softSerial.available() != 0) {
    data[count] = softSerial.read();
    if (data[count] == '\r') {
      data[count] = 0;
      if (strlen(data) > 0)
        dataRecovery();
      count = 0;
    } else if (data[count] != NULL) {
      count++;
    }
  }
  delay(1000); 
}

void dataRecovery() {
  String rawdata = String(data);
  String type = rawdata.substring(0,2);
  int id = type.toInt();
  int datalen = strlen(data);
  String sdata = rawdata.substring(3, datalen);
  sdata.trim();

  switch (id) {
    case 10:
      Serial.println("Data transfer start");
      softSerial.write(10);
      break;
  
    case 20:
      Serial.println("DATA received ");
      mqttclient.publish("onsemi/", sdata.c_str());
      Serial.println("Data upload to MQTT server finished!!");
      softSerial.write(20);
      break;
      
    default:
      softSerial.write(50);
      break;  
  }
}

