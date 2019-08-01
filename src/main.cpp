#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>             // Library to use I2C to display
#include "SSD1306Wire.h"      // Display library

SSD1306Wire  display(0x3c, 5, 4);
void show(String text){
  display.clear();
  delay(100);
  // display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  // clear the display
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawStringMaxWidth(0, 0, 120, text);
  display.display();
  delay(200);
}
// Update these with values suitable for your network.

const char* ssid = "YourWifi"; 
const char* password = "Password";
const char* mqtt_server = "MQTT Server or IP";
const char* mqtt_username = "MQTTUsername";
const char* mqtt_password = "MQTTUsername";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

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

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  //char* c;
  char* c;
   for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //c[i] = (char)payload[i];
  }
  display.init();
  c = "";
  c =(char*) payload;
  Serial.println();
  show(c);

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(16, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(16, HIGH);  // Turn the LED off by making the voltage HIGH
  }
  //payload = NULL;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic/1", "Connected");
      // ... and resubscribe
      client.subscribe("inTopic");
      client.subscribe("inTopic/1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(16, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
    if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, "hello world #%d", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic/1", msg);
  }
}