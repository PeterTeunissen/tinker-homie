#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define WIFI_SSID "...."
#define WIFI_PASSWORD "...."

#define MQTT_HOST IPAddress(?.?.?.?)
#define MQTT_PORT 1883

char topic[20] = "EBedstripL";
char mqttUsername[20] = ".....";
char mqttPassword[20] = ".....";

const int PIN_RELAY = 5; // D1
const int PIN_PIR = 4; // D2

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

bool relayOn = false;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  char buf[30];

  strcpy(buf,"/homie/");
  strcat(buf,topic);
  strcat(buf,"/on");
  
  uint16_t packetIdSub = mqttClient.subscribe(buf, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);

  strcpy(buf,"/homie/");
  strcat(buf,topic);
  strcat(buf,"/pir");
    
  uint16_t packetIdPub2 = mqttClient.publish(buf, 2, true, "true");
  Serial.println("Publishing at QoS 2");
  Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  char buf[len+3];
  memcpy(buf,payload,len);
  buf[len]=0;
  Serial.print("  payload: ");
  Serial.println(buf);

  if (strcmp(buf,"true")==0) {
    digitalWrite(PIN_RELAY,HIGH);
    relayOn = true;  
  }

  if (strcmp(buf,"false")==0) {
    digitalWrite(PIN_RELAY,LOW);
    relayOn = false;      
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  pinMode(PIN_RELAY,OUTPUT);
  pinMode(PIN_PIR,INPUT);

  digitalWrite(PIN_RELAY,LOW);
  
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.setCredentials(mqttUsername,mqttPassword);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

bool lastPir = false;
void loop() {

  bool pirOn = (digitalRead(PIN_PIR)==HIGH);
  if (pirOn!=lastPir) {
    char buf[30];
    strcpy(buf,"/homie/");
    strcat(buf,topic);
    strcat(buf,"/pir");
      
    uint16_t packetIdPub2 = mqttClient.publish(buf, 2, true, (pirOn?"true":"false"));
    Serial.print("Publishing PIR:");
    Serial.print(pirOn?"true":"false");
    Serial.print(" status:");
    Serial.println(packetIdPub2);
    
    lastPir = pirOn;
  }

}
