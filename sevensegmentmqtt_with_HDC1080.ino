/* MQTT to 7 Segment display
 *  used to display the water consumption on a adafruit 7 segment display
 *  based on HT16k33 backpack.
 *  
 *  by Alexander 
 *  version 01 - 8 july 2020 - Proof of Concept
 *  version 02 - 12 july 2020 - Added Si7021 temp/humidity sensor on i2c for use in bathroom
 *  version 03 - 12 july 2020 - Added additional MQTT Subscription to submit brighness level of display!
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_Si7021.h"

Adafruit_Si7021 sensor = Adafruit_Si7021();

Adafruit_7segment matrix = Adafruit_7segment();


// Update these with values suitable for your network.

const char* ssid = "YOUR SSID";
const char* password = "YOUR WIFI PASSWORD";
const char* mqtt_server = "YOUR MQTT SERVER IP"; //like 192.168.1.4


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;



void setup() {
  
  matrix.begin(0x70); //initialize Adafruit 7 segment display

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

    Serial.println("Si7021 test!");
  
  if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
    while (true);
  }
}



void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  matrix.blinkRate(1);
 // matrix.print(0xC000,HEX); //to display C000 as in starting wifi
 matrix.writeDigitRaw(0,0b01000000 );
 matrix.writeDigitRaw(1,0b01000000 );
 matrix.writeDigitRaw(2,0b00000000 );
 matrix.writeDigitRaw(3,0b01001001 );
 matrix.writeDigitRaw(4,0b01001001 );

  matrix.writeDisplay();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
 // matrix.print(0xAA00,HEX); //to display AA00 when WiFi is connected
 matrix.blinkRate(0);
 matrix.writeDigitRaw(0,0b00111001 );
 matrix.writeDigitRaw(1,0b00111111 );
 matrix.writeDigitRaw(2,0b00000000 );
 matrix.writeDigitRaw(3,0b00110111 );
 matrix.writeDigitRaw(4,0b00110111 );
  matrix.writeDisplay();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  //check for 1st topic
  if(strcmp(topic,"home/watermeter/daycount") == 0){ 
    payload[length] = '\0';
    String s = String((char*)payload);
    int k= s.toInt(); 
    matrix.print(k,DEC); //moet dit hele zwikkie niet buiten de loop (of kan de hele for eruit omdat ie direct de payload pakt?
    matrix.writeDisplay();
    }
  //Check for 2nd topic
  else if(strcmp(topic,"home/badkamer/brightness") == 0){
    payload[length] = '\0';
    String s = String((char*)payload);
    int brightness= s.toInt(); 
    matrix.setBrightness(brightness);
    }

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client","YOUR MQ USERNAME,"YOUR MQPASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("home/watermeter/daycount"); //get watermeter stand
      client.subscribe("home/badkamer/brightness"); //get brightness level of display 
      
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
  if (now - lastMsg > 30000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 10, "%.2f", sensor.readTemperature());
    client.publish("home/badkamer/temperature", msg);
    delay(200);
    snprintf (msg, 10, "%.2f", sensor.readHumidity());
    client.publish("home/badkamer/humidity", msg);


    Serial.print("Humidity:    "); Serial.print(sensor.readHumidity(), 2);
    Serial.print("\tTemperature: "); Serial.println(sensor.readTemperature(), 2);
   

  }
}
