/*
 * REFERENCES :
 * https://projetsdiy.fr/mq135-mesure-qualite-air-polluant-arduino/
 * 
 * 
 */

#include <SPI.h>
#include <LoRa.h>

#include "MQ135.h"
#include "DHT.h"

const int mq135Pin = 0;     // Pin sur lequel est branché de MQ135

MQ135 gasSensor = MQ135(mq135Pin);  // Initialise l'objet MQ135 sur le Pin spécifié

DHT dht;

void setup()
{
  pinMode(A1, OUTPUT);
  digitalWrite(A1, HIGH);
  
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa AIR TEST Sender");

  if (!LoRa.begin(433E6))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Get/Set calibration data
  float rzero = gasSensor.getRZero();
  
  Serial.print("R0: ");
  Serial.println(rzero);  // Valeur à reporter ligne 27 du fichier mq135.h après 48h de préchauffage

  digitalWrite(A1, LOW);

  //Setup DHT11
  dht.setup(4); //com on pin 4
}

void loop()
{
  digitalWrite(A1, HIGH);
  
  // Get Air Measure
  
  Serial.println("Measuring...");

  // Temp & Hum Sampling
  delay(dht.getMinimumSamplingPeriod()); // Delay of amount equal to sampling period

  // Gas Measure
  float ppm = gasSensor.getPPM();
  
  //Serial.print("A0 : ");
  //Serial.print(analogRead(mq135Pin));
  Serial.print("CO2 : ");
  Serial.print(ppm);

  // Get Temp & Hum measure

  float humidity = dht.getHumidity(); // Get humidity value
  float temperature = dht.getTemperature(); // Get temperature value
  
  //Serial.print(dht.getStatusString()); // Print status of communication
  Serial.print(" ppm ; Hum : ");
  Serial.print(humidity, 1);
  Serial.print(" % ; Temp : ");
  Serial.print(temperature, 1);
  Serial.print(" °C");
  //Serial.println(dht.toFahrenheit(temperature), 1); // Convert temperature to Fahrenheit units

  Serial.println();

  //Send Result
  
  Serial.print("Sending packet...");
  
  // send packet
  LoRa.beginPacket();

  LoRa.print("p");
  LoRa.print(ppm);

  LoRa.print(",h");
  LoRa.print(humidity, 1);
  LoRa.print(",t");
  LoRa.print(temperature, 1);
  
  LoRa.endPacket();

  Serial.println("Done.");

  // END
  digitalWrite(A1, LOW);
  
  // WAIT
  delay(10000);
}
