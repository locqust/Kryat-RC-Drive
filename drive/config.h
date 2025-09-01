#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>

// Defines the configuration for a single button action.
struct ButtonAction {
    char name[24];
    int soundMin;
    int soundMax;
    int delay;
    int maestro1Script;
    int maestro2Script;
    char serialCommand[32];
};

// Defines the single set of rules for automation mode
struct AutomationConfig {
    char description[32];
    int delayMin;
    int delayMax;
    int soundMin;
    int soundMax;
    int maestroMin;
    int maestroMax;
    char serialCommandOn[32];
    char serialCommandOff[32];
};

// This structure holds all of our configuration parameters.
struct Config {
    // General Settings
    int _buttonsCH;
    int _ToggleCH;
    int _volumeCH;
    int _maestroCount;
    int _rcButtons12CH;
    int _rcButtons34CH;
    int _rcButtons56CH;
    int _wifiToggleCH;
    int _automationToggleCH;
    bool enableSerial;
    int numButtons;
    int stopAllButton;


    // WiFi Settings
    int wifiMode; // 0 for AP, 1 for STA
    char ap_ssid[32];
    char ap_password[64];
    char sta_ssid[32];
    char sta_password[64];

    // PID & MAC Settings
    uint8_t dome_mac[6];
    float pid_kp;
    float pid_ki;
    float pid_kd;

    // Automation Settings
    AutomationConfig automation;

    // PWM Button Values
    int pwmButtonValues[15];

    // Actions
    ButtonAction actions_p1[15];       // Pad 1
    ButtonAction actions_p2[15];       // Pad 2
    ButtonAction actions_rc[6];        // RC Switches
};

// Declare a global instance of the config
extern Config config;
extern bool isInAutomationMode; // from drive.ino

// Function prototypes for filesystem operations
void loadConfiguration();
void saveConfiguration();
void serializeButtonActionArray(JsonArray& arr, const ButtonAction actions[], int count);
void deserializeButtonActionArray(const JsonArray& arr, ButtonAction actions[], int count);
void setupWifi();
void handleWifiReconnect();
void setupEspNow();

#endif // CONFIG_H

