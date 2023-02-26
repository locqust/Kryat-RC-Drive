/**
   FrSky SBUs RC control board for Hamster drive BB8
   Authors:
      Patrick Ryan <pat.m.ryan@gmail.com> - @circuitBurn
      Darren Poulson <darren.poulson@gmail.com> - Fediverse: dpoulson@fr.droidbuilders.uk
      Andrew Smith <locqust@gmail.com> 
    This is a heavily modified version of the Joe's drive FrSky SBUS RC control made by Darren, stripped 
    down for a Hamster drive (which I call Kryat)
    I've also taken advantage of the fact I have a Kyber 15 button board on my TX so I have written the code to utliise that in a similar way to Kyber.
    I have also lifted the Automation code from Padawan 360, though I currently have no idea how to tell the dome to rotate in a simialr way without 
    breaking the TX ability to control the dome servo too. 

   Serial Ports:
      Serial1 - DFPlayer Mini
      Serial2 - SBUS RC

   Resources:
      https://github.com/circuitBurn/BB-8
      https://github.com/dpoulson/bb8_rc_control
      https://bb8builders.club/
      https://www.facebook.com/groups/863379917081398
*/


#include "sbus.h"
#include "Arduino.h"
#include <esp_now.h>
#include <Wire.h>
#include "DFRobotDFPlayerMini.h"
#include <HardwareSerial.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "enums.h"
#include "constants.h"
#include <EEPROM.h>




/* SBUS object, reading SBUS */
bfs::SbusRx sbus_rx(&Serial2, 16, 16, false);
bfs::SbusData data;
//bfs::SbusTx sbus_tx(&Serial2, 16, 16, false);


DFRobotDFPlayerMini myDFPlayer;
unsigned long lastMillis, soundMillis;

struct dome_message currentDomeMessage;
esp_now_peer_info_t peerDome;
int chan;

byte automateAction = 0;
boolean isInAutomationMode = false;



void setup() {
 sbus_rx.Begin();
  delay(2000);

  Serial.begin(115200);
  randomSeed(analogRead(0));

  pinMode(AUDIO_OUTPUT_PIN, INPUT);


    WiFi.mode(WIFI_STA);  // Needed for ESPNow

    // WIFI STATION CODE and OTA not currently working
  // #ifdef ENABLE_WIFI
    // Setup Wifi and OTA updates
    // WiFi.begin(ssid, password);
    // //WiFi.begin();
    // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //   Serial.println("Connection Failed! Rebooting...");
    //   delay(5000);
    //   ESP.restart();
    // }

  
    // ArduinoOTA
    //   .onStart([]() {
    //     String type;
    //     if (ArduinoOTA.getCommand() == U_FLASH)
    //       type = "sketch";
    //     else // U_SPIFFS
    //       type = "filesystem";
  
    //     // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    //     Serial.println("Start updating " + type);
    //   })
    //   .onEnd([]() {
    //     Serial.println("\nEnd");
    //   })
    //   .onProgress([](unsigned int progress, unsigned int total) {
    //     Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    //   })
    //   .onError([](ota_error_t error) {
    //     Serial.printf("Error[%u]: ", error);
    //     if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    //     else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    //     else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    //     else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    //     else if (error == OTA_END_ERROR) Serial.println("End Failed");
    //   });
  
    // ArduinoOTA.begin();
  
    // Serial.println("Ready");
    // Serial.print("IP address: ");
    // Serial.println(WiFi.localIP());
    // Serial.println(WiFi.macAddress());
  //#endif
  
  // Sound
  #ifdef AUDIO_ENABLED
    Serial1.begin(9600, SERIAL_8N1, 18, 19);
    myDFPlayer.begin(Serial1);
    myDFPlayer.volume(30);
    delay (1000);
    myDFPlayer.play(12);
    Serial.println("Audio Started");
  #endif


  if (esp_now_init() != 0) {
    Serial.println("Problem during ESP-NOW init");
    return;
  }

  // Register peer
  memcpy(peerDome.peer_addr, dome_mac, 6);
  peerDome.channel = 0;  
  peerDome.encrypt = false;

  
  // Add peer        
  if (esp_now_add_peer(&peerDome) != ESP_OK){
    Serial.println("**** Failed to add Dome");
    return;
  }

  currentDomeMessage.psi = false;
  currentDomeMessage.effect = 1; // Set connection annimation
  esp_err_t result = esp_now_send(NULL, (uint8_t *) &currentDomeMessage, sizeof(currentDomeMessage));
  if (result == ESP_OK) {
    Serial.println("Initial Sent with success");
  }
  else {
    Serial.println("Error sending initial data");
  }
  
  Serial.println("Startup complete!");
}

void loop() {

  if (sbus_rx.Read())
  {
  // 15 Button Toggle  
  toggle_pad(); 
  //play_sound(); 
  check_sound();
      data = sbus_rx.data();
    /* Display the received data */
    //  Serial.print(data.ch[CH_Button_Pad]);
    //  Serial.print("\t");
         
      // Plays random sounds or dome movements for automations when in automation mode
      if(isInAutomationMode){
        triggerAutomation();
        }
  }
  if ((millis() - soundMillis) > 25)

    {
      int soundLevel = analogRead(AUDIO_OUTPUT_PIN);
      //int soundLevel =1;
      //Serial.println(soundLevel);
      if (soundLevel > 1600)
      {
        currentDomeMessage.psi = 1;
        Serial.println("PSI1 more 1600");        
      }
      else if (soundLevel < 1600)
      {
        currentDomeMessage.psi = 0;
        Serial.println("PSI0 less 1600"); 
      }
      else
      {
        currentDomeMessage.psi = 0;
        Serial.println("PSI0"); 
      }
      send_dome_message();
    } else {
      if (currentDomeMessage.psi == 1)
      {
        currentDomeMessage.psi = 0;
        send_dome_message();
        Serial.println("send message"); 
      }
     
    } 
    soundMillis = millis();

  // //hub.handle();


  // #ifdef ENABLE_WIFI
  //   ArduinoOTA.handle();
  // #endif

}



