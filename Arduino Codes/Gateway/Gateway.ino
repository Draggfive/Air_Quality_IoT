#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>

#define SS 15
#define RST 16
#define DIO0 4

#define LED 5

#define MQTTSUB_1 "room"
#define MQTTSUB_1_PPM "room/PPM"
#define MQTTSUB_1_HUM "room/Humidity"
#define MQTTSUB_1_TMP "room/Temperature"

//Uncomment if doing LoRa disabled tests
//#define OFFLINE

// Change the credentials below, so your ESP8266 connects to your router
const char* hname = "IoT-LoRa-MQTT-Gateway";
const char* ssid = "";      //Set your router SSID
const char* password = "";  //Set your router Password

// Change the variable to your MQTT Broker, so it connects to it
const char* mqttServer = "iot.fr-par.scw.cloud";
const int mqttPort = 1883;
const char* mqttUser = "";      //Set yout user ID
const char* mqttPassword = "";  //Set your MQTT Password

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_WiFi()
{
  delay(10);
  
  // We start by connecting to a WiFi network
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.hostname(hname);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("WiFi connected - IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(char* topic, byte* message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.println("Message: ");
  
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection... ");
    
    // Attempt to connect
    if (client.connect("ESP8266Client", mqttUser, mqttPassword ))
    {
      Serial.println("Connected");
      
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics
      client.subscribe(MQTTSUB_1);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      Serial.println("Try again in 5 seconds");
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_LoRa()
{
  //Configure and enable LoRa module
  Serial.print("Enabling LoRa... ");

  LoRa.setPins(SS, RST, DIO0);
  if (LoRa.begin(433E6))
  {
    Serial.println("Done.");
  }
  else
  {
    Serial.println("Starting LoRa failed!");
  }
}


// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  Serial.begin(115200);
  delay(5000);

  digitalWrite(LED, LOW);

  Serial.println("LoRa to WiFi MQTT Gateway");
  
  setup_WiFi();
  setup_LoRa();
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  reconnect();
}

void loop()
{
  // CHECK STATUS
  
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected ! Trying to reconnect...");
    setup_WiFi();
  }
  
  if (!client.connected() || !client.loop())
  {
    Serial.println("MQTT Disconnected !");
    reconnect();
  }

  // RECEIVE DATA
  
  int packetSize = LoRa.parsePacket();

#ifdef OFFLINE
  packetSize = 18;
#endif
  
  if (packetSize)
  {
    digitalWrite(LED, HIGH); //Receiving status

    // read packet
    String packet = "";
    
    while (LoRa.available())
    {
      packet += LoRa.read();
    }

#ifdef OFFLINE
    packet = "p310.2,h39.5,t25.3,";
#endif

    //Print packet
    Serial.println();
    Serial.println("LoRa Received :");
    Serial.println(packet);

    // print RSSI of packet
    Serial.print("RSSI : ");
    Serial.println(LoRa.packetRssi());


    // COMPUTE DATA

    //get ppm
    int ppm_start = packet.indexOf("p");
    int ppm_end = packet.indexOf(",", ppm_start);

    //extract ppm value
    String ppm_str = packet.substring(ppm_start+1, ppm_end+1);
    char ppm[ppm_str.length()];
    ppm_str.toCharArray(ppm, ppm_str.length());
    
    Serial.print("Found PPM value : ");
    Serial.println(ppm);

    //get hum
    int hum_start = packet.indexOf("h");
    int hum_end = packet.indexOf(",", hum_start);

    //extract hum value
    String hum_str = packet.substring(hum_start+1, hum_end+1);
    char hum[hum_str.length()];
    hum_str.toCharArray(hum, hum_str.length());
    
    Serial.print("Found Humidity value : ");
    Serial.println(hum);

    //get tmp
    int tmp_start = packet.indexOf("t");
    int tmp_end = packet.indexOf(",", tmp_start);

    //extract tmp value
    String tmp_str = packet.substring(tmp_start+1, tmp_end+1);
    char tmp[tmp_str.length()];
    tmp_str.toCharArray(tmp, tmp_str.length());

    Serial.print("Found Temperature value : ");
    Serial.println(tmp);


    // TRANSMIT DATA
    
    client.publish(MQTTSUB_1_PPM, ppm);
    client.publish(MQTTSUB_1_HUM, hum);
    client.publish(MQTTSUB_1_TMP, tmp);

#ifdef OFFLINE
    delay(1000);
#endif

    digitalWrite(LED, LOW); // End status

#ifdef OFFLINE
    delay(4000);
#endif
  }
} 
