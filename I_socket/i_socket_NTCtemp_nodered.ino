

 // Microcontroller: ESP8266 12F
   




#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <math.h>

#include <stdlib.h>
#include <Arduino.h>

#define REL  14 // releu
#define LED 15 // led semnalizare

unsigned int Rs = 120000;// resistor already soldered on ESP board
double Vcc = 3.3;


// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "xxxxx";
const char* password = "xxxxxxxx";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.110.105";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient esp_isocket1temp;
PubSubClient client(esp_isocket1temp);

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
  if (topic == "room/isocket1")
  {
    Serial.print("Changing isocket1 to ");
    if (messageTemp == "on")
    {
      digitalWrite(REL, HIGH);
      digitalWrite(LED, HIGH);
      Serial.print("On");
    }
    else if (messageTemp == "off")
    {
      digitalWrite(REL, LOW);
      digitalWrite(LED, LOW);
      Serial.print("Off");
    }
  }


  Serial.println();
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
    if (client.connect("esp_isocket1temp")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/isocket1");
      //client.subscribe("room/REL2");
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
  pinMode(REL, OUTPUT);
  pinMode(LED, OUTPUT);
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
    client.connect("esp_isocket1temp");

  now = millis();

  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 30000)
  {
    lastMeasure = now;
    float tempNTC = Thermister(AnalogRead());

    // Computes temperature values in Celsius

    static char temperatureNTC[7];
    dtostrf(tempNTC, 6, 2, temperatureNTC);
    // Publishes Temperature and Humidity values
    client.publish("room/temperatureNTC", temperatureNTC);


   
    Serial.print(" %\t Temperature: ");
    Serial.print(Thermister(AnalogRead()));
    Serial.print(" *C ");
    Serial.println();


  }
}



int AnalogRead() 
 {
  int val = 0;
  for (int i = 0; i < 20; i++) // averaging values
  {
    val += analogRead(A0);
    delay(1);
  }

  val = val / 20;
  return val;
 }

double Thermister(int val) 
 {
  double V_NTC = (double)val / 1024;
  double R_NTC = (Rs * V_NTC) / (Vcc - V_NTC);
  R_NTC = log(R_NTC);
  double Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * R_NTC * R_NTC )) * R_NTC );
  Temp = Temp - 273.15;
  return Temp;

 }




