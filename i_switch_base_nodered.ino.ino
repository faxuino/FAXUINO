
/*i_switch_base_nodered.ino
  Microcontroller: ESP8266 12F
   Forward data  to Raspberry Pi server with Red Node software:

   Temperature
   Humidity
   Rel1 and Rel2 control through toggle circuit with 4013 bistable
   12 I_switch sensors can be connected in parallel
   The operating algorithm is: increment on the rising front 100msec switches REL1 on / off

   Rel1: on / minimum light level
   Rel2: on / maximum light level
   Rel1: off / minimum light level
   Rel2: off / off light



*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <stdlib.h>
#include <Arduino.h>


// Module connection pins (Digital Pins)
#define SW2_OUT 13 // toggle output1 
#define SW1_OUT 12 // toggle output2 
#define DHTPin 2 // DHT I/O pin
#define REL1_IN 5 // Input. It monitors the REL1 state  to correctly change displayed state in Red Node
#define REL2_IN 4 // Input. It monitors the REL2 state  to correctly change  displayed state in Red Node
#define IR_IN 0 // IR receiver input
#define IR_OUT 15 // output IR LED
#define PIR_IN 16 // PIR digital input
#define SMOKE_IN 15 // digital smoke sensor input
#define ANALOG_IN A0 // analog input 0 ... 1V 12bit



// Uncomment one of the lines bellow for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "xxxxxxxxxxxxx";
const char* password = "yyyyyyyyyyyy";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.110.105";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient esp_iswbase1;
PubSubClient client(esp_iswbase1);

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi()
{
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
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/REL, you check if the message is either on or off. Turns the REL1 GPIO according to the message
  if (topic == "room/level1")
  {
    Serial.print("Changing level1 to ");
    if (messageTemp == "on")
    { if (digitalRead(REL1_IN) == LOW)
      {
        digitalWrite(SW1_OUT, HIGH);
        delay(100);
        digitalWrite(SW1_OUT, LOW);
        Serial.print("on");
      }
    }
    if (messageTemp == "off")
    { if (digitalRead(REL1_IN) == HIGH)
      {
        digitalWrite(SW1_OUT, HIGH);
        delay(100);
        digitalWrite(SW1_OUT, LOW);
        Serial.print("off");
      }
    }
  }

  if (topic == "room/level2")
  {
    Serial.print("Changing level2 to ");
    if (messageTemp == "on")
    { if (digitalRead(REL2_IN) == LOW)
      {
        digitalWrite(SW2_OUT, HIGH);
        delay(100);
        digitalWrite(SW2_OUT, LOW);
        Serial.print("on");
      }
    }
    if (messageTemp == "off")
    { if (digitalRead(REL2_IN) == HIGH)
      {
        digitalWrite(SW2_OUT, HIGH);
        delay(100);
        digitalWrite(SW2_OUT, LOW);
        Serial.print("on");
        
      }
    }
  }Serial.println();
}
  // This functions reconnects your ESP8266 to your MQTT broker
  // Change the function below if you want to subscribe to more topics with your ESP8266
  void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      /*
        YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
        To change the ESP device ID, you will have to give a new name to the ESP8266.
        Here's how it looks:
         if (client.connect("ESP8266Client")) {
        You can do it like this:
         if (client.connect("ESP1_Office")) {
        Then, for the other ESP:
         if (client.connect("ESP2_Garage")) {
        That should solve your MQTT multiple connections problem
      */
      if (client.connect("esp_iswbase1")) {
        Serial.println("connected");
        // Subscribe or resubscribe to a topic
        // You can subscribe to more topics (to control more LEDs in this example)
        client.subscribe("room/level1");
        client.subscribe("room/level2");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }

  // The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
  // Sets your mqtt broker and sets the callback function
  // The callback function is what receives messages and actually controls the LEDs

  void setup()
  {
    pinMode(SW1_OUT, OUTPUT);
    pinMode(SW2_OUT, OUTPUT);
    pinMode(REL1_IN, INPUT);
    pinMode(REL2_IN, INPUT);

    delay(1000);

    digitalWrite(SW1_OUT, HIGH);
    delay(100);
    digitalWrite(SW1_OUT, LOW);

    digitalWrite(SW2_OUT, HIGH);
    delay(100);
    digitalWrite(SW2_OUT, LOW);


    dht.begin();

    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);


  }

  // For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker

  void loop()
  {

    if (!client.connected())
    {
      reconnect();
    }
    if (!client.loop())
      client.connect("esp_iswbase1");

    now = millis();





    // Publishes new temperature and humidity every 30 seconds
    if (now - lastMeasure > 30000)
    {
      lastMeasure = now;
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) || isnan(f))
      {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }

      // Computes temperature values in Celsius
      float hic = dht.computeHeatIndex(t, h, false);
      static char temperatureTemp[7];
      dtostrf(hic, 6, 2, temperatureTemp);

      // Uncomment to compute temperature values in Fahrenheit
      // float hif = dht.computeHeatIndex(f, h);
      // static char temperatureTemp[7];
      // dtostrf(hic, 6, 2, temperatureTemp);

      static char humidityTemp[7];
      dtostrf(h, 6, 2, humidityTemp);

      // Publishes Temperature and Humidity values
      client.publish("room/temperature", temperatureTemp);
      client.publish("room/humidity", humidityTemp);

      // Publishes REL1 and REL 2 state every  1 seconds



      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t Temperature: ");
      Serial.print(t);
      Serial.print(" *C ");
      Serial.print(f);
      Serial.print(" *F\t Heat index: ");
      Serial.print(hic);
      Serial.println(" *C ");



    }

  }





