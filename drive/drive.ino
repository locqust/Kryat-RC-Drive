/**
   OpenSource FrSky SBUs RC control board for Astromechs
   Authors:
      Patrick Ryan <pat.m.ryan@gmail.com> - @circuitBurn
      Darren Poulson <darren.poulson@gmail.com> - Fediverse: dpoulson@fr.droidbuilders.uk
      Andrew Smith <locqust@gmail.com> 
    This is a heavily modified version of the Joe's drive FrSky SBUS RC control made by Darren, stripped 
    down and rebuilt for a Hamster Drive.
    I've also taken advantage of the fact I have a Kyber 15 button board on my TX so I have written the code to utliise that in a similar way to Kyber.
    This also takes ideas and parts of Padawan 360, originally made by Dan Kraus and customised by Steve Baudains of Imperial Light and Magic. I.e Meastro control, 
    I2C, Dome and sound automation etc. As well as other add-ons that I made to my own Padawan 360 for R2-D2. 
    Effectively I'm trying to get the best of both worlds of RC and Padawan while keeping it simple and open to others to play around with.

   Serial Ports:
      Serial1 - DFPlayer Mini
      Serial2 - SBUS RC

   Resources:
      https://github.com/circuitBurn/BB-8
      https://github.com/dpoulson/bb8_rc_control
      https://bb8builders.club/
      https://www.facebook.com/groups/863379917081398
      https://github.com/Imperiallandm/padawan_360_mega_maestro
*/
#include "sbus.h"
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h> // Added this include for the webserver functionality
#include <esp_now.h>
#include "config.h"
#include "constants.h"
#include "enums.h"
#include "Arduino.h"

// --- Function Prototypes ---
// from filesystem.ino
void loadConfiguration();
void saveConfiguration();
// from webserver.ino
void setupWebServer();
// from sound.ino
void check_sound();
void toggle_pad();
// from automation.ino
void triggerAutomation();
// from maestro.ino
void setupMaestro();
void stopAllMaestros();
// from imu.ino
void setupIMU();
void readIMU();
// from functions.ino
void send_dome_message();


// --- DFPlayer Mini ---
HardwareSerial DFPlayerSerial(1);
DFRobotDFPlayerMini myDFPlayer;

// --- SBUS RC Control ---
// SBUS connected to GPIO 16 (RX2)
bfs::SbusRx sbus_rx(&Serial2, 16, 16, false);
bfs::SbusData data;

// --- Global State ---
bool isInAutomationMode = false;
bool automationToggleState = false;
bool wifiToggleState = false;
bool wifiEnabled = true;

// --- ESP-Now ---
dome_message currentDomeMessage;
esp_now_peer_info_t peerInfo;

// --- Timers ---
unsigned long soundMillis = 0;
unsigned long wifiReconnectMillis = 0;


void setup() {
  Serial.begin(115200);
  Serial.println("Booting Krayt Drive...");

  pinMode(AUDIO_BUSY_PIN, INPUT);
  pinMode(AUDIO_OUTPUT_PIN, INPUT);

  // Load configuration from SPIFFS
  loadConfiguration();

  // Start SBUS
  sbus_rx.Begin();

  // Start DFPlayer
  DFPlayerSerial.begin(9600, SERIAL_8N1, DFPLAYER_TX_PIN, DFPLAYER_RX_PIN);
  if (myDFPlayer.begin(DFPlayerSerial)) {
    Serial.println("DFPlayer Mini online.");
    myDFPlayer.volume(15); // Default volume, will be overwritten by RC
  } else {
    Serial.println("Connecting to DFPlayer Mini... failed.");
  }

  // Setup Maestro Serial
  setupMaestro();

  // Setup WiFi
  setupWifi();

  // Setup ESP-Now
  setupEspNow();

  // Setup IMU
  setupIMU();
  
  // Start the web server
  setupWebServer();

  Serial.println("Startup complete!");
}

void loop() {
  // Read SBUS data
  if (sbus_rx.Read()) {
    data = sbus_rx.data();

    // Toggle Automation Mode
    toggle_automation();
    
    // Toggle WiFi
    toggle_wifi();

    // Toggle Button Pad Page
    toggle_pad();

    // Check for button presses and play sounds/actions
    check_sound();

    // Set volume from RC potentiometer
    set_volume_from_rc();
    
    // If in automation mode, run automation logic
    if (isInAutomationMode) {
      triggerAutomation();
    }
  }

  // Handle WiFi connection logic (reconnecting if STA mode)
  if (config.wifiMode == 1 && wifiEnabled) { // If in STA mode and WiFi is on
      handleWifiReconnect();
  }
  
  // Read IMU data periodically
  readIMU();

  // ESP-Now dome message and sound level check
  if ((millis() - soundMillis) > 25) {
    if (digitalRead(AUDIO_BUSY_PIN) == LOW) {
      int soundLevel = analogRead(AUDIO_OUTPUT_PIN);
      // Serial.println(soundLevel);
      if (soundLevel > 3600) {
        currentDomeMessage.psi = 1;
      } else {
        currentDomeMessage.psi = 0;
      }
    } else {
      currentDomeMessage.psi = 0;
    }
    // Initialize the 'effect' field to prevent sending garbage data
    currentDomeMessage.effect = 0; 
    send_dome_message();
    soundMillis = millis();
  }
}


// --- RC Toggle Functions ---

void toggle_automation() {
  if (config._automationToggleCH > 0) {
    bool newState = sbus_rx.data().ch[config._automationToggleCH - 1] > RC_MID;
    if (newState != automationToggleState) {
      automationToggleState = newState;
      if (automationToggleState) { // Only trigger on the rising edge of the switch flip
        isInAutomationMode = !isInAutomationMode;
        Serial.print("Automation Mode Toggled: ");
        Serial.println(isInAutomationMode ? "ON" : "OFF");

        // Send state change serial command
        if (config.enableSerial) {
          if (isInAutomationMode && strlen(config.automation.serialCommandOn) > 0) {
            Serial.println(config.automation.serialCommandOn);
          } else if (!isInAutomationMode && strlen(config.automation.serialCommandOff) > 0) {
            Serial.println(config.automation.serialCommandOff);
          }
        }
      }
    }
  }
}

void toggle_wifi() {
    if (config._wifiToggleCH > 0) {
        bool newState = sbus_rx.data().ch[config._wifiToggleCH - 1] > RC_MID;
        if (newState != wifiToggleState) {
            wifiToggleState = newState;
            if (wifiToggleState) { // Only trigger on rising edge
                wifiEnabled = !wifiEnabled;
                if (wifiEnabled) {
                    Serial.println("WiFi turned ON via RC. Re-initializing...");
                    setupWifi();
                } else {
                    Serial.println("WiFi turned OFF via RC.");
                    WiFi.mode(WIFI_OFF);
                }
            }
        }
    }
}

void set_volume_from_rc() {
    if (config._volumeCH > 0) {
        int vol_channel = config._volumeCH - 1;
        // Map the SBUS value (174-1815) to the DFPlayer volume range (0-30)
        int volume = map(sbus_rx.data().ch[vol_channel], RC_MIN, RC_MAX, 0, 30);
        myDFPlayer.volume(volume);
    }
}
