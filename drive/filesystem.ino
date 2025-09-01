#include <SPIFFS.h>
#include "config.h"

#define FORMAT_SPIFFS_IF_FAILED true

// Global instance of the config struct
Config config;

// Helper function to turn a ButtonAction array into a JSON array
void serializeButtonActionArray(JsonArray& arr, const ButtonAction actions[], int count) {
    for (int i = 0; i < count; i++) {
        JsonObject obj = arr.createNestedObject();
        obj["name"] = actions[i].name;
        obj["soundMin"] = actions[i].soundMin;
        obj["soundMax"] = actions[i].soundMax;
        obj["delay"] = actions[i].delay;
        obj["maestro1Script"] = actions[i].maestro1Script;
        obj["maestro2Script"] = actions[i].maestro2Script;
        obj["serialCommand"] = actions[i].serialCommand;
    }
}

// Helper function to parse a JSON array into a ButtonAction array
void deserializeButtonActionArray(const JsonArray& arr, ButtonAction actions[], int count) {
    int i = 0;
    for (JsonVariant v : arr) {
        if (i >= count) break;
        JsonObject obj = v.as<JsonObject>();
        strlcpy(actions[i].name, obj["name"] | "", sizeof(actions[i].name));
        actions[i].soundMin = obj["soundMin"] | 0;
        actions[i].soundMax = obj["soundMax"] | 0;
        actions[i].delay = obj["delay"] | 0;
        actions[i].maestro1Script = obj["maestro1Script"] | 0;
        actions[i].maestro2Script = obj["maestro2Script"] | 0;
        strlcpy(actions[i].serialCommand, obj["serialCommand"] | "", sizeof(actions[i].serialCommand));
        i++;
    }
}

void saveConfiguration() {
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return;
    }

    DynamicJsonDocument doc(10240); 

    // General
    doc["_buttonsCH"] = config._buttonsCH;
    doc["_ToggleCH"] = config._ToggleCH;
    doc["_volumeCH"] = config._volumeCH;
    doc["_maestroCount"] = config._maestroCount;
    doc["_rcButtons12CH"] = config._rcButtons12CH;
    doc["_rcButtons34CH"] = config._rcButtons34CH;
    doc["_rcButtons56CH"] = config._rcButtons56CH;
    doc["_wifiToggleCH"] = config._wifiToggleCH;
    doc["_automationToggleCH"] = config._automationToggleCH;
    doc["enableSerial"] = config.enableSerial;
    doc["numButtons"] = config.numButtons;
    doc["stopAllButton"] = config.stopAllButton;

    // WiFi
    doc["wifiMode"] = config.wifiMode;
    doc["ap_ssid"] = config.ap_ssid;
    doc["ap_password"] = config.ap_password;
    doc["sta_ssid"] = config.sta_ssid;
    doc["sta_password"] = config.sta_password;
    
    // PID & MAC
    JsonArray dome_mac = doc.createNestedArray("dome_mac");
    for(int i=0; i<6; i++) dome_mac.add(config.dome_mac[i]);
    doc["pid_kp"] = config.pid_kp;
    doc["pid_ki"] = config.pid_ki;
    doc["pid_kd"] = config.pid_kd;

    // Automation
    JsonObject automation = doc.createNestedObject("automation");
    automation["description"] = config.automation.description;
    automation["delayMin"] = config.automation.delayMin;
    automation["delayMax"] = config.automation.delayMax;
    automation["soundMin"] = config.automation.soundMin;
    automation["soundMax"] = config.automation.soundMax;
    automation["maestroMin"] = config.automation.maestroMin;
    automation["maestroMax"] = config.automation.maestroMax;
    automation["serialCommandOn"] = config.automation.serialCommandOn;
    automation["serialCommandOff"] = config.automation.serialCommandOff;

    // PWM
    JsonArray pwm = doc.createNestedArray("pwmButtonValues");
    for (int i = 0; i < 15; i++) pwm.add(config.pwmButtonValues[i]);
    
    // Actions
    JsonArray actions1 = doc.createNestedArray("actions_p1");
    serializeButtonActionArray(actions1, config.actions_p1, 15);
    JsonArray actions2 = doc.createNestedArray("actions_p2");
    serializeButtonActionArray(actions2, config.actions_p2, 15);
    JsonArray actions_rc = doc.createNestedArray("actions_rc");
    serializeButtonActionArray(actions_rc, config.actions_rc, 6);
    
    if (serializeJson(doc, configFile) == 0) {
        Serial.println("Failed to write to config file");
    }
    configFile.close();
    Serial.println("Configuration saved.");
}

void loadConfiguration() {
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file, using defaults.");
        // Generate a default AP SSID if one doesn't exist
        uint8_t mac[6];
        WiFi.macAddress(mac);
        sprintf(config.ap_ssid, "Krayt-%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
        strlcpy(config.ap_password, "123456789", sizeof(config.ap_password));
        config.numButtons = 15; // Default to 15 buttons
        config.stopAllButton = 0; // Default to no stop button
        config.enableSerial = false; // Default to disabled
        return;
    }

    DynamicJsonDocument doc(10240);
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        configFile.close();
        return;
    }

    // General
    config._buttonsCH = doc["_buttonsCH"] | 9;
    config._ToggleCH = doc["_ToggleCH"] | 10;
    config._volumeCH = doc["_volumeCH"] | 11;
    config._maestroCount = doc["_maestroCount"] | 0;
    config._rcButtons12CH = doc["_rcButtons12CH"] | 13;
    config._rcButtons34CH = doc["_rcButtons34CH"] | 14;
    config._rcButtons56CH = doc["_rcButtons56CH"] | 15;
    config._wifiToggleCH = doc["_wifiToggleCH"] | 0;
    config._automationToggleCH = doc["_automationToggleCH"] | 0;
    config.enableSerial = doc["enableSerial"] | false;
    config.numButtons = doc["numButtons"] | 15;
    config.stopAllButton = doc["stopAllButton"] | 0;

    // WiFi
    config.wifiMode = doc["wifiMode"] | 0;
    strlcpy(config.ap_ssid, doc["ap_ssid"] | "", sizeof(config.ap_ssid));
    strlcpy(config.ap_password, doc["ap_password"] | "123456789", sizeof(config.ap_password));
    strlcpy(config.sta_ssid, doc["sta_ssid"] | "", sizeof(config.sta_ssid));
    strlcpy(config.sta_password, doc["sta_password"] | "", sizeof(config.sta_password));

    // PID & MAC
    JsonArray dome_mac = doc["dome_mac"];
    for(int i=0; i<6; i++) config.dome_mac[i] = dome_mac[i] | 0;
    config.pid_kp = doc["pid_kp"] | 1.0;
    config.pid_ki = doc["pid_ki"] | 0.1;
    config.pid_kd = doc["pid_kd"] | 0.01;

    // Automation
    JsonObject auto_doc = doc["automation"];
    strlcpy(config.automation.description, auto_doc["description"] | "", sizeof(config.automation.description));
    config.automation.delayMin = auto_doc["delayMin"] | 5;
    config.automation.delayMax = auto_doc["delayMax"] | 15;
    config.automation.soundMin = auto_doc["soundMin"] | 1;
    config.automation.soundMax = auto_doc["soundMax"] | 35;
    config.automation.maestroMin = auto_doc["maestroMin"] | 0;
    config.automation.maestroMax = auto_doc["maestroMax"] | 0;
    strlcpy(config.automation.serialCommandOn, auto_doc["serialCommandOn"] | "", sizeof(config.automation.serialCommandOn));
    strlcpy(config.automation.serialCommandOff, auto_doc["serialCommandOff"] | "", sizeof(config.automation.serialCommandOff));

    // PWM
    JsonArray pwm = doc["pwmButtonValues"];
    for (int i = 0; i < 15; i++) config.pwmButtonValues[i] = pwm[i] | 0;
    
    // Actions
    deserializeButtonActionArray(doc["actions_p1"], config.actions_p1, 15);
    deserializeButtonActionArray(doc["actions_p2"], config.actions_p2, 15);
    deserializeButtonActionArray(doc["actions_rc"], config.actions_rc, 6);

    configFile.close();
    Serial.println("Configuration loaded.");
}

