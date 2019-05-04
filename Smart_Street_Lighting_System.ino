#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include<FirebaseArduino.h>

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883      
#define AIO_USERNAME    "raunak1998"
#define AIO_KEY         "ac661ef9a5af4106ad8d06f3025d3d50"

#define WLAN_SSID       "4167"
#define WLAN_PASS       "notforfree"

#define FIREBASE_HOST "street-lighting-86a4d.firebaseio.com"
#define FIREBASE_AUTH "EcmhNwfD1G4aYoOklYnhvCpiaS3dlI8HOGtHsagO"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/street-light");
//Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
const int trigPin = D3;  
const int echoPin = D2;  
const int relayPin = D5;
bool overrideOn = false;
bool turnedOn = false;

// defines variables
long duration;
int distance;
int startTime;

void setup() {
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT); 
pinMode(relayPin, OUTPUT);
digitalWrite(relayPin, HIGH);
Serial.begin(9600); 
Serial.print("Connecting to ");
Serial.println(WLAN_SSID);

WiFi.begin(WLAN_SSID, WLAN_PASS);
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
mqtt.subscribe(&onoffbutton);
Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
    MQTT_connect();
    Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      if(strcmp((char*)onoffbutton.lastread, "ON")==0)
        overrideOn = true;
      else
        overrideOn = false;
    }
  }
int sensorValue = analogRead(A0);
  
float voltage = sensorValue * (5.0 / 1023.0);
digitalWrite(trigPin, LOW);
delayMicroseconds(2);

digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

duration = pulseIn(echoPin, HIGH);

distance= duration*0.034/2;
Serial.print("Distance: ");
Serial.println(distance);
Firebase.pushInt("/ultrasonic", distance);
Serial.print("LDR: ");
Serial.println(voltage);
Firebase.pushInt("/ldr", voltage);
if(overrideOn || (voltage<2.5 && distance<50)){
    digitalWrite(relayPin, LOW);
    startTime = millis();
    turnedOn = true;
    }
    else if(turnedOn == true){
    digitalWrite(relayPin, HIGH);
    Firebase.pushFloat("/time", (millis()-startTime)/(1000.0*60.0));
    turnedOn = false;
    }

   delay(1000);
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
  delay(500);
}
