#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <WiFi.h>
#include <espnow.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN D4
#define LED_COUNT 9
#define BRIGHTNESS 50

#define MAIN_EYE 0
#define LED_HP 1
#define LED_PSI 2
#define FRONT_LOGIC_1 3
#define FRONT_LOGIC_2 4
#define FRONT_LOGIC_3 5
#define FRONT_LOGIC_4 6
#define REAR_LOGIC_1 7
#define REAR_LOGIC_2 8

const char* ssid = "your SSID";
const char* password = "your pass";


struct dome_message {
  bool psi; // Flash the PSI
  int effect; // Trigger an effect
};

dome_message latestDomeMessage;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void onDataReceiver(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
   // We don't use mac to verify the sender
   // Let us transform the incomingData into our message structure
   memcpy(&latestDomeMessage, incomingData, sizeof(latestDomeMessage));
   Serial.print("Message Received: psi - ");
   Serial.print(latestDomeMessage.psi);
   Serial.print(" effect - ");
   Serial.println(latestDomeMessage.effect);
}


void setup() {
  Serial.begin(115200);
  Serial.print("started");
  WiFi.disconnect();
  ESP.eraseConfig();
 
  // Wifi STA Mode
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  //WiFi.begin();  

  // Get Mac Add
  Serial.print("Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("\nESP-Now Receiver");
  
  // Initializing the ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Problem during ESP-NOW init");
    return;
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  // We can register the receiver callback function
  esp_now_register_recv_cb(onDataReceiver);

  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);
  pixels.show();

  // HP
  pixels.setPixelColor(LED_HP, pixels.Color(0, 0, 255));

  // PSI
  pixels.setPixelColor(LED_PSI, pixels.Color(0, 0, 0));

  // Front Logic
  pixels.setPixelColor(FRONT_LOGIC_1, pixels.Color(255, 255, 255));
  pixels.setPixelColor(FRONT_LOGIC_2, pixels.Color(255, 255, 255));
  pixels.setPixelColor(FRONT_LOGIC_3, pixels.Color(255, 255, 255));
  pixels.setPixelColor(FRONT_LOGIC_4, pixels.Color(255, 255, 255));

  // Rear Logic
  pixels.setPixelColor(REAR_LOGIC_1, pixels.Color(0, 0, 255));
  pixels.setPixelColor(REAR_LOGIC_2, pixels.Color(0, 0, 255));

  // Main Eye
  pixels.setPixelColor(MAIN_EYE, pixels.Color(255, 0, 0));

  pixels.show();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (latestDomeMessage.psi == 1) {
    pixels.setPixelColor(LED_PSI, pixels.Color(255, 255, 255));
    pixels.show();
    Serial.println("doing psi stuff");
  } else {
    pixels.setPixelColor(LED_PSI, pixels.Color(0, 0, 0));
     pixels.show();
  }
}