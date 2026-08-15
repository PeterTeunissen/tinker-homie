#include <FS.h>                   // Handle filesystems
#include <LittleFS.h>             // Use LittleFS instead of deprecated SPIFFS
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>          // Required for parsing JSON configuration file

// Define SoftwareSerial pins for RYLR module
// D1 (GPIO5) as RX, D2 (GPIO4) as TX
SoftwareSerial loraSerial(D5,D6); 

#define TRIGGER_PIN D7
#define LED_1 D1
#define LED_2 D2

// Global variables for MQTT configurations (with default fallback values)
char mqtt_server[40]   = "192.168.1.25";
char mqtt_port[6]      = "1883";
char mqtt_user[40]     = "";
char mqtt_pass[40]     = "";
char mqtt_topic[50]    = "/homie/loraGateway";
char mqtt_reply_topic[50]    = "/homie/loraGatewayReply";

// Flag for saving configuration
bool shouldSaveConfig = false;
byte ledStatus = 0;
bool msgArrived = false;

bool inConfigMode = false;
unsigned long previousMillis = 0;
const long interval = 250; // Blink interval in milliseconds
bool ledState = HIGH;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Callback notifying us that the web portal submitted new configurations
void saveConfigCallback () {
  Serial.println("Should save config flag set to true");
  shouldSaveConfig = true;
}

// Handle incoming MQTT messages and relay them to LoRa
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic [");
  Serial.print(topic);
  Serial.print("]: ");

  msgArrived = true;
  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // { "address": "44", "message": "..." }

  DynamicJsonDocument doc(1024);

  // 3. Deserialize/Parse the string payload
  DeserializationError error = deserializeJson(doc, message.c_str());
  
  if (!error) {
    Serial.println("\nMessage JSON configuration successfully.");
    if (doc["address"] && doc["message"]) {

      // Format: AT+SEND=<Address>,<Length>,<Data>
      String payload = doc["message"];
      String addr = doc["address"];
      String loraCommand = "AT+SEND=" + addr + "," + String(payload.length()) + "," + payload;
      
      loraSerial.println(loraCommand); 
      Serial.print("Relayed to LoRa: ");
      Serial.println(loraCommand);
      ledStatus++;
    
    } else {
      Serial.println("Mallformed message JSON: no 'address' and/or no 'message'");
      ledStatus=0;
    }    
  } else {
    Serial.println("Mallformed JSON");
    ledStatus=0;
  }  
}

void setup() {

  WiFiManager wifiManager;

  Serial.begin(115200);
  loraSerial.begin(9600); // RYLR default baud rate
  
  Serial.println("\nMounting LittleFS file system...");

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_BUILTIN,HIGH);
    
  if (digitalRead(TRIGGER_PIN) == LOW) {
    Serial.println("Resetting Settings...");
    wifiManager.resetSettings(); 
  }

  if (LittleFS.begin()) {
    Serial.println("Mounted file system successfully.");
    if (LittleFS.exists("/config.json")) {
      // File exists, reading and loading configurations
      Serial.println("Reading config file...");
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("Opened config file successfully.");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        
        if (!deserializeError) {
          Serial.println("\nParsed JSON configuration successfully.");
          if(json["mqtt_server"]) strcpy(mqtt_server, json["mqtt_server"]);
          if(json["mqtt_port"])   strcpy(mqtt_port, json["mqtt_port"]);
          if(json["mqtt_user"])   strcpy(mqtt_user, json["mqtt_user"]);
          if(json["mqtt_pass"])   strcpy(mqtt_pass, json["mqtt_pass"]);
          if(json["mqtt_topic"])  strcpy(mqtt_topic, json["mqtt_topic"]);
          if(json["mqtt_reply_topic"])  strcpy(mqtt_reply_topic, json["mqtt_reply_topic"]);
        } else {
          Serial.println("Failed to parse config JSON string.");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("Failed to mount LittleFS system.");
  }

  // Set up custom parameter fields for the WiFiManager portal page
  // Notice the "type=password" variant added to mask the secret input field
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Broker", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT Username", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_pass("pass", "MQTT Password", mqtt_pass, 40, "type='password'");
  WiFiManagerParameter custom_mqtt_topic("topic", "MQTT Topic", mqtt_topic, 50);
  WiFiManagerParameter custom_mqtt_reply_topic("reply_topic", "MQTT Reply Topic", mqtt_reply_topic, 50);

  // Set configuration save callback hook
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Add your configured parameters into the setup wizard
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_mqtt_topic);
  wifiManager.addParameter(&custom_mqtt_reply_topic);

  // Connects automatically using saved credentials. Spins up portal if connection fails.
  if (!wifiManager.autoConnect("ESP-LoRa-Gateway")) {
    Serial.println("Failed to connect and hit timeout. Resetting system...");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  // Device successfully bound to localized network infrastructure
  Serial.println("WiFi Connected successfully!");

  // Extract variables out of form submissions
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  strcpy(mqtt_topic, custom_mqtt_topic.getValue());
  strcpy(mqtt_reply_topic, custom_mqtt_reply_topic.getValue());

  // Save the custom parameters to LittleFS if changed
  if (shouldSaveConfig) {
    Serial.println("Saving configured values into LittleFS file...");
    DynamicJsonDocument json(1024);
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"]   = mqtt_port;
    json["mqtt_user"]   = mqtt_user;
    json["mqtt_pass"]   = mqtt_pass;
    json["mqtt_topic"]  = mqtt_topic;
    json["mqtt_reply_topic"]  = mqtt_reply_topic;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open local config file for writing storage.");
    }

    serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    Serial.println("\nConfigurations safely written to flash.");
  }

  // Initialize and point our MQTT instance to our parsed broker targets
  int portInt = atoi(mqtt_port);
  mqttClient.setServer(mqtt_server, portInt);
  mqttClient.setCallback(mqttCallback);

  digitalWrite(LED_BUILTIN, LOW); 
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print("...");
    
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    bool connected = false;
    if (strlen(mqtt_user) > 0) {
      connected = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass);
    } else {
      connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic);
      Serial.print("Subscribed to topic: ");
      Serial.println(mqtt_topic);
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      digitalWrite(LED_1, LOW);
      digitalWrite(LED_2, LOW);
      delay(5000);
    }
  }
}

void ledShow() {

  if (inConfigMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
    }
  }
  
  if (!msgArrived) {
    return;
  }
  
  if (ledStatus>2) {
    ledStatus=1;
  }

  if (ledStatus == 0) {
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);  
  }

  if (ledStatus == 1) {
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, LOW);      
  }

  if (ledStatus == 2) {
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, HIGH);      
  }
}

void parseAndPublishLoRaMessage(String rylrStr) {

  // Strip off the "+RCV=" string
  String payload = rylrStr.substring(5);

  // 2. Find and extract the Address
  int comma1 = payload.indexOf(',');
  if (comma1 == -1) {
    Serial.println("Can not find first comma");
    ledStatus=0;
    return;
  }
  
  String addrStr = payload.substring(0, comma1);
  int address=addrStr.toInt();
  
  // 3. Find and extract the Length
  int comma2 = payload.indexOf(',', comma1 + 1);
  if (comma2 == -1) {
    Serial.println("Can not find second comma");
    ledStatus=0;
    return;
  }

  String lenStr = payload.substring(comma1 + 1, comma2);

  // Convert header values to integers
  int len = lenStr.toInt();

  // 4. Use the explicit length to cleanly slice out the Data payload
  int dataStart = comma2 + 1;
  if (dataStart + len > payload.length()) {
    Serial.println("String to short to get message from");
    ledStatus=0;
    return; // Error: Payload string is shorter than the stated length
  }
  
  String message = payload.substring(dataStart, dataStart + len);

  // 5. Parse RSSI and SNR from the remaining string tail
  String remainder = payload.substring(dataStart + len);
  if (!remainder.startsWith(",")) {
    Serial.println("String to short to get rssi and snr");
    return; // Error: Missing formatting comma after data payload
  }
  
  // Skip the leading comma of the remainder
  remainder = remainder.substring(1); 
  int comma3 = remainder.indexOf(',');
  if (comma3 == -1) {
    Serial.println("String to short to get rssi and snr");
    return;
  }

  String rssiStr = remainder.substring(0, comma3);
  String snrStr = remainder.substring(comma3 + 1);

  int rssi = rssiStr.toInt();
  int snr = snrStr.toInt();

  // 5. Build the JSON document
  // Allocate static memory on the stack (fast and safe for small JSONs)
  DynamicJsonDocument doc(1024);
   
  doc["address"] = addrStr;
  doc["message"] = message;
  doc["rssi"] = rssi;
  doc["snr"] = snr;

  // 6. Serialize JSON document into a printable String
  String jsonOutput;
  serializeJson(doc, jsonOutput);

  int jsonLength = jsonOutput.length() + 1;
  char jsonBuffer[jsonLength];
  jsonOutput.toCharArray(jsonBuffer, jsonLength);

  if (mqttClient.publish(mqtt_reply_topic, jsonBuffer)) {
    Serial.println("MQTT Reply Publish Successful!");
    Serial.print("Payload: ");
    Serial.println(jsonBuffer);
    ledStatus++;
  } else {
    Serial.println("MQTT Publish Failed!");
    ledStatus=0;
  }  
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  if (loraSerial.available()>0) {
    
    String incomingString = loraSerial.readStringUntil('\n');
    incomingString.trim(); // Remove extra carriage returns or spaces

    Serial.print("Received LoraMessage:");
    Serial.println(incomingString);
    
    // Check if the message starts with "+RCV="
    if (incomingString.startsWith("+RCV=")) {
      parseAndPublishLoRaMessage(incomingString);
    } else {
      Serial.println("Lora Message not an RCV message");   
    }
  }
   
  ledShow();
}
