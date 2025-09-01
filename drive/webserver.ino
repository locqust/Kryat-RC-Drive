#include <ESPAsyncWebServer.h>
#include "config.h"

// The HTML page is now embedded here.
extern const char index_html[] PROGMEM; // Defined at the bottom of this file

// Declare the global IMU data struct from imu.ino
extern struct IMUData imuData;

// From drive.ino, to get live SBUS data
extern bfs::SbusData data; 

AsyncWebServer server(80);

void handleSaveReboot(AsyncWebServerRequest *request, const char* message) {
    saveConfiguration();
    request->send(200, "text/plain", message);
    delay(1000);
    ESP.restart();
}

void parseButtonAction(AsyncWebServerRequest *request, ButtonAction& action, int index, const char* prefix) {
    String base = "b" + String(index) + "_";
    
    if(request->hasParam(base + "name", true)) strlcpy(action.name, request->getParam(base + "name", true)->value().c_str(), sizeof(action.name));
    if(request->hasParam(base + "soundMin", true)) action.soundMin = request->getParam(base + "soundMin", true)->value().toInt();
    if(request->hasParam(base + "soundMax", true)) action.soundMax = request->getParam(base + "soundMax", true)->value().toInt();
    if(request->hasParam(base + "delay", true)) action.delay = request->getParam(base + "delay", true)->value().toInt();
    if(request->hasParam(base + "m1", true)) action.maestro1Script = request->getParam(base + "m1", true)->value().toInt();
    if(request->hasParam(base + "m2", true)) action.maestro2Script = request->getParam(base + "m2", true)->value().toInt();
    if(request->hasParam(base + "serial", true)) strlcpy(action.serialCommand, request->getParam(base + "serial", true)->value().c_str(), sizeof(action.serialCommand));
}


void setupWebServer() {
    // --- Page and Data Routes ---
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        String json;
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
        JsonArray host_mac = doc.createNestedArray("host_mac");
        uint8_t mac[6];
        WiFi.macAddress(mac);
        for(int i=0; i<6; i++) host_mac.add(mac[i]);
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
        
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    server.on("/sbus", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (config._buttonsCH > 0) {
            request->send(200, "text/plain", String(data.ch[config._buttonsCH - 1]));
        } else {
            request->send(200, "text/plain", "N/A");
        }
    });

    server.on("/imu", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(256);
        doc["ax"] = imuData.ax;
        doc["ay"] = imuData.ay;
        doc["az"] = imuData.az;
        doc["gx"] = imuData.gx;
        doc["gy"] = imuData.gy;
        doc["gz"] = imuData.gz;
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // --- NEW: Export and Import Routes ---
    server.on("/export-config", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/config.json", "application/json", true);
    });

    server.on("/import-config", HTTP_POST, 
        // Success response handler
        [](AsyncWebServerRequest *request){
            request->send(200, "text/plain", "Configuration Uploaded. Rebooting...");
            delay(1000);
            ESP.restart();
        }, 
        // File upload handler
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
            if(!index){
                // Open the file on first chunk of data
                request->_tempFile = SPIFFS.open("/config.json", "w");
                Serial.printf("Upload Start: %s\n", filename.c_str());
            }
            if(len){
                // Write the chunk to the file
                request->_tempFile.write(data, len);
            }
            if(final){
                // Close the file on the last chunk
                request->_tempFile.close();
                Serial.printf("Upload End: %s, %u bytes\n", filename.c_str(), index+len);
            }
        }
    );


    // --- SAVE ENDPOINTS ---
    server.on("/save-general", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("_buttonsCH", true)) config._buttonsCH = request->getParam("_buttonsCH", true)->value().toInt();
        if(request->hasParam("_ToggleCH", true)) config._ToggleCH = request->getParam("_ToggleCH", true)->value().toInt();
        if(request->hasParam("_rcButtons12CH", true)) config._rcButtons12CH = request->getParam("_rcButtons12CH", true)->value().toInt();
        if(request->hasParam("_rcButtons34CH", true)) config._rcButtons34CH = request->getParam("_rcButtons34CH", true)->value().toInt();
        if(request->hasParam("_rcButtons56CH", true)) config._rcButtons56CH = request->getParam("_rcButtons56CH", true)->value().toInt();
        if(request->hasParam("_volumeCH", true)) config._volumeCH = request->getParam("_volumeCH", true)->value().toInt();
        if(request->hasParam("_wifiToggleCH", true)) config._wifiToggleCH = request->getParam("_wifiToggleCH", true)->value().toInt();
        if(request->hasParam("_automationToggleCH", true)) config._automationToggleCH = request->getParam("_automationToggleCH", true)->value().toInt();
        if(request->hasParam("_maestroCount", true)) config._maestroCount = request->getParam("_maestroCount", true)->value().toInt();
        
        config.enableSerial = request->hasParam("enableSerial", true);
        if(request->hasParam("numButtons", true)) config.numButtons = request->getParam("numButtons", true)->value().toInt();
        if(request->hasParam("stopAllButton", true)) config.stopAllButton = request->getParam("stopAllButton", true)->value().toInt();

        handleSaveReboot(request, "General Settings Saved. Rebooting...");
    });
    
    server.on("/save-wifi", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("wifiMode", true)) config.wifiMode = request->getParam("wifiMode", true)->value().toInt();
        if(request->hasParam("ap_ssid", true)) strlcpy(config.ap_ssid, request->getParam("ap_ssid", true)->value().c_str(), sizeof(config.ap_ssid));
        if(request->hasParam("ap_password", true)) strlcpy(config.ap_password, request->getParam("ap_password", true)->value().c_str(), sizeof(config.ap_password));
        if(request->hasParam("sta_ssid", true)) strlcpy(config.sta_ssid, request->getParam("sta_ssid", true)->value().c_str(), sizeof(config.sta_ssid));
        if(request->hasParam("sta_password", true)) strlcpy(config.sta_password, request->getParam("sta_password", true)->value().c_str(), sizeof(config.sta_password));
        handleSaveReboot(request, "WiFi Settings Saved. Rebooting...");
    });

    server.on("/save-pid", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("dome_mac", true)) {
            String macStr = request->getParam("dome_mac", true)->value();
            sscanf(macStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                &config.dome_mac[0], &config.dome_mac[1], &config.dome_mac[2], 
                &config.dome_mac[3], &config.dome_mac[4], &config.dome_mac[5]);
        }
        if(request->hasParam("pid_kp", true)) config.pid_kp = request->getParam("pid_kp", true)->value().toFloat();
        if(request->hasParam("pid_ki", true)) config.pid_ki = request->getParam("pid_ki", true)->value().toFloat();
        if(request->hasParam("pid_kd", true)) config.pid_kd = request->getParam("pid_kd", true)->value().toFloat();
        handleSaveReboot(request, "PID Settings Saved. Rebooting...");
    });

    server.on("/save-automation", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("auto_desc", true)) strlcpy(config.automation.description, request->getParam("auto_desc", true)->value().c_str(), sizeof(config.automation.description));
        if(request->hasParam("auto_delayMin", true)) config.automation.delayMin = request->getParam("auto_delayMin", true)->value().toInt();
        if(request->hasParam("auto_delayMax", true)) config.automation.delayMax = request->getParam("auto_delayMax", true)->value().toInt();
        if(request->hasParam("auto_soundMin", true)) config.automation.soundMin = request->getParam("auto_soundMin", true)->value().toInt();
        if(request->hasParam("auto_soundMax", true)) config.automation.soundMax = request->getParam("auto_soundMax", true)->value().toInt();
        if(request->hasParam("auto_maestroMin", true)) config.automation.maestroMin = request->getParam("auto_maestroMin", true)->value().toInt();
        if(request->hasParam("auto_maestroMax", true)) config.automation.maestroMax = request->getParam("auto_maestroMax", true)->value().toInt();
        if(request->hasParam("auto_serial_on", true)) strlcpy(config.automation.serialCommandOn, request->getParam("auto_serial_on", true)->value().c_str(), sizeof(config.automation.serialCommandOn));
        if(request->hasParam("auto_serial_off", true)) strlcpy(config.automation.serialCommandOff, request->getParam("auto_serial_off", true)->value().c_str(), sizeof(config.automation.serialCommandOff));
        handleSaveReboot(request, "Automation Settings Saved. Rebooting...");
    });

    server.on("/save-pwm", HTTP_POST, [](AsyncWebServerRequest *request){
        for(int i = 0; i < 15; i++) {
            String paramName = "pwm" + String(i);
            if (request->hasParam(paramName, true)) {
                config.pwmButtonValues[i] = request->getParam(paramName, true)->value().toInt();
            }
        }
        handleSaveReboot(request, "PWM Settings Saved. Rebooting...");
    });

    server.on("/save-buttons-rc", HTTP_POST, [](AsyncWebServerRequest *request){
        for (int i = 0; i < 6; i++) {
            parseButtonAction(request, config.actions_rc[i], i, "rc");
        }
        handleSaveReboot(request, "RC Button Settings Saved. Rebooting...");
    });
    
    server.on("/save-buttons1", HTTP_POST, [](AsyncWebServerRequest *request){
        for (int i = 0; i < 15; i++) {
            parseButtonAction(request, config.actions_p1[i], i, "p1");
        }
        handleSaveReboot(request, "Buttons 1 Settings Saved. Rebooting...");
    });

    server.on("/save-buttons2", HTTP_POST, [](AsyncWebServerRequest *request){
        for (int i = 0; i < 15; i++) {
            parseButtonAction(request, config.actions_p2[i], i, "p2");
        }
        handleSaveReboot(request, "Buttons 2 Settings Saved. Rebooting...");
    });

    server.begin();
    Serial.println("Web server started.");
}

// Store the HTML content in PROGMEM at the end of the file. This is the full webpage.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Krayt Controls Config</title>
    <style>
        /* Embedded TailwindCSS and Custom Styles for Offline Use */
        :root {
            --color-black: #000;
            --color-white: #fff;
            --color-gray-400: #9ca3af;
            --color-gray-500: #6b7280;
            --color-gray-600: #4b5563;
            --color-gray-700: #374151;
            --color-gray-800: #1f2937;
            --color-gray-900: #111827;
            --color-blue-400: #60a5fa;
            --color-blue-500: #3b82f6;
            --color-blue-600: #2563eb;
            --color-blue-700: #1d4ed8;
            --color-green-500: #22c55e;
            --color-green-600: #16a34a;
            --color-steel-blue: steelblue;
        }
        *, ::before, ::after { box-sizing: border-box; border-width: 0; border-style: solid; border-color: #e5e7eb; }
        html { line-height: 1.5; -webkit-text-size-adjust: 100%; -moz-tab-size: 4; tab-size: 4; font-family: ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"; }
        body { margin: 0; line-height: inherit; font-family: "Trebuchet MS", Arial, sans-serif; background-color: var(--color-black); color: var(--color-white); padding: 1rem; }
        .container { width: 100%; margin-left: auto; margin-right: auto; padding-left: 1rem; padding-right: 1rem; }
        @media (min-width: 640px) { .container { max-width: 640px; } body { padding: 1.5rem; } }
        @media (min-width: 768px) { .container { max-width: 768px; } }
        @media (min-width: 1024px) { .container { max-width: 1024px; } }
        @media (min-width: 1280px) { .container { max-width: 1280px; } }
        .max-w-xl { max-width: 36rem; } .max-w-2xl { max-width: 42rem; } .max-w-3xl { max-width: 48rem; } .max-w-4xl { max-width: 56rem; } .max-w-6xl { max-width: 72rem; } .max-w-sm { max-width: 24rem; }
        .mx-auto { margin-left: auto; margin-right: auto; }
        .p-1 { padding: 0.25rem; } .p-2 { padding: 0.5rem; } .p-3 { padding: 0.75rem; } .p-4 { padding: 1rem; } .p-6 { padding: 1.5rem; }
        .px-4 { padding-left: 1rem; padding-right: 1rem; } .py-1 { padding-top: 0.25rem; padding-bottom: 0.25rem; } .py-2 { padding-top: 0.5rem; padding-bottom: 0.5rem; }
        .mb-1 { margin-bottom: 0.25rem; } .mb-2 { margin-bottom: 0.5rem; } .mb-4 { margin-bottom: 1rem; } .mt-4 { margin-top: 1rem; } .mt-6 { margin-top: 1.5rem; }
        .gap-1 { gap: 0.25rem; } .gap-2 { gap: 0.5rem; } .gap-4 { gap: 1rem; } .gap-6 { gap: 1.5rem; }
        .space-x-2 > :not([hidden]) ~ :not([hidden]) { margin-right: 0; margin-left: 0.5rem; }
        .space-y-2 > :not([hidden]) ~ :not([hidden]) { margin-top: 0.5rem; margin-bottom: 0; }
        .space-y-4 > :not([hidden]) ~ :not([hidden]) { margin-top: 1rem; margin-bottom: 0; }
        .flex { display: flex; } .inline-flex { display: inline-flex; }
        .grid { display: grid; }
        .hidden { display: none; }
        .items-center { align-items: center; } .justify-center { justify-content: center; } .justify-between { justify-content: space-between; }
        .flex-wrap { flex-wrap: wrap; }
        .grid-cols-1 { grid-template-columns: repeat(1, minmax(0, 1fr)); }
        .grid-cols-2 { grid-template-columns: repeat(2, minmax(0, 1fr)); }
        .w-full { width: 100%; } .w-1\/2 { width: 50%; } .w-28 { width: 7rem; }
        .rounded { border-radius: 0.25rem; } .rounded-md { border-radius: 0.375rem; } .rounded-lg { border-radius: 0.5rem; }
        .bg-gray-600 { background-color: var(--color-gray-600); } .bg-gray-700 { background-color: var(--color-gray-700); } .bg-gray-800 { background-color: var(--color-gray-800); } .bg-gray-900 { background-color: var(--color-gray-900); }
        .bg-blue-600 { background-color: var(--color-blue-600); } .hover\:bg-blue-700:hover { background-color: var(--color-blue-700); }
        .bg-green-500 { background-color: var(--color-green-500); } .hover\:bg-green-600:hover { background-color: var(--color-green-600); }
        .text-center { text-align: center; }
        .text-sm { font-size: 0.875rem; line-height: 1.25rem; } .text-lg { font-size: 1.125rem; line-height: 1.75rem; } .text-xl { font-size: 1.25rem; line-height: 1.75rem; } .text-2xl { font-size: 1.5rem; line-height: 2rem; } .text-3xl { font-size: 1.875rem; line-height: 2.25rem; }
        .font-bold { font-weight: 700; } .font-mono { font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace; } .font-semibold { font-weight: 600; }
        .text-gray-400 { color: var(--color-gray-400); } .text-blue-400 { color: var(--color-blue-400); } .text-blue-500 { color: var(--color-blue-500); }
        .cursor-pointer { cursor: pointer; } .cursor-not-allowed { cursor: not-allowed; }
        .block { display: block; }
        input, select, button { font-family: inherit; font-size: 100%; margin: 0; padding: 0; line-height: inherit; color: inherit; }
        button, a { text-transform: none; background-color: transparent; background-image: none; cursor: pointer; text-decoration: none;}
        input[type="radio"], input[type="checkbox"] { -webkit-appearance: none; -moz-appearance: none; appearance: none; padding: 0; -webkit-print-color-adjust: exact; print-color-adjust: exact; display: inline-block; vertical-align: middle; background-origin: border-box; -webkit-user-select: none; -moz-user-select: none; user-select: none; flex-shrink: 0; height: 1rem; width: 1rem; color: var(--color-blue-500); background-color: #fff; border-color: #6b7280; border-width: 1px; }
        input[type="radio"] { border-radius: 100%; }
        input[type="checkbox"] { border-radius: 0.25rem; }
        input[type="radio"]:checked, input[type="checkbox"]:checked { background-color: currentColor; }
        input:disabled { background-color: var(--color-gray-500); cursor: not-allowed; }
        @media (min-width: 640px) { .sm\:grid-cols-2 { grid-template-columns: repeat(2, minmax(0, 1fr)); } }
        @media (min-width: 768px) { .md\:grid-cols-2 { grid-template-columns: repeat(2, minmax(0, 1fr)); } .md\:grid-cols-3 { grid-template-columns: repeat(3, minmax(0, 1fr)); } .md\:col-span-2 { grid-column: span 2 / span 2; } }
        @media (min-width: 1024px) { .lg\:grid-cols-3 { grid-template-columns: repeat(3, minmax(0, 1fr)); } }
        
        /* Custom non-Tailwind styles */
        .nav-link.active { background-color: var(--color-steel-blue) !important; }
        .form-card { background-color: #2d3748; }
        .form-input { background-color: #4a5568; color: var(--color-white); border: 1px solid #718096; }
        .grid-item { background-color: #222; border: 1px solid var(--color-steel-blue); }
        .grid-item.disabled { background-color: #333; border-color: #555; }
    </style>
</head>
<body class="bg-black text-white p-4 sm:p-6">
    <div class="container max-w-6xl mx-auto">
        <h2 class="text-2xl font-bold text-center bg-gray-700 p-3 rounded-md mb-4">Krayt Balancing Droid Control System V0.1</h2>
        
        <div id="main-nav" class="nav flex flex-wrap justify-center gap-2 mb-4">
            <a href="#general" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('general')">General</a>
            <a href="#system" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('system')">System</a>
            <a href="#wifi" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('wifi')">Wifi</a>
            <a href="#pid" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('pid')">PID</a>
            <a href="#automation" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('automation')">Automation</a>
            <a href="#pwm" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('pwm')">PWM</a>
            <a href="#buttons-rc" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('buttons-rc')">Buttons RC</a>
            <a href="#buttons1" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('buttons1')">Buttons 1</a>
            <a href="#buttons2" class="nav-link bg-gray-600 px-4 py-2 rounded-md" onclick="showPage('buttons2')">Buttons 2</a>
        </div>

        <main>
            <!-- General Page -->
            <div id="general" class="page">
                <form id="form-general" class="form-card p-6 rounded-lg">
                    <div class="grid grid-cols-1 md:grid-cols-2 gap-x-6 gap-y-4 max-w-4xl mx-auto">
                        <!-- Column 1 -->
                        <div class="space-y-4">
                            <h3 class="font-bold text-lg text-blue-400 border-b border-gray-600 pb-1">Channels</h3>
                            <div><label class="block mb-1 text-gray-400">Buttons Channel (SBUS):</label><input type="number" name="_buttonsCH" class="form-input w-full p-2 rounded"></div>
                            <div><label class="block mb-1 text-gray-400">Button Page Toggle Channel:</label><input type="number" name="_ToggleCH" class="form-input w-full p-2 rounded"></div>
                            <div><label class="block mb-1 text-gray-400">RC Buttons 1-2 Channel:</label><input type="number" name="_rcButtons12CH" class="form-input w-full p-2 rounded"></div>
                            <div><label class="block mb-1 text-gray-400">RC Buttons 3-4 Channel:</label><input type="number" name="_rcButtons34CH" class="form-input w-full p-2 rounded"></div>
                            <div><label class="block mb-1 text-gray-400">RC Buttons 5-6 Channel:</label><input type="number" name="_rcButtons56CH" class="form-input w-full p-2 rounded"></div>
                            <div><label class="block mb-1 text-gray-400">Volume Channel:</label><input type="number" name="_volumeCH" class="form-input w-full p-2 rounded"></div>
                            <div><label class="block mb-1 text-gray-400">WiFi Toggle Channel:</label><input type="number" name="_wifiToggleCH" class="form-input w-full p-2 rounded"></div>
                            <div><label class="block mb-1 text-gray-400">Automation Toggle Channel:</label><input type="number" name="_automationToggleCH" class="form-input w-full p-2 rounded"></div>
                        </div>
                        <!-- Column 2 -->
                        <div class="space-y-4">
                            <h3 class="font-bold text-lg text-blue-400 border-b border-gray-600 pb-1">Features</h3>
                            <div>
                                <label class="block mb-1 text-gray-400">Number of Maestros:</label>
                                <select name="_maestroCount" class="form-input w-full p-2 rounded"><option value="0">0</option><option value="1">1</option><option value="2">2</option></select>
                            </div>
                            <div class="flex items-center pt-2">
                                <input type="checkbox" id="enableSerial" name="enableSerial" class="form-input h-5 w-5 rounded">
                                <label for="enableSerial" class="ml-2 text-gray-300">Enable Serial Command Support</label>
                            </div>
                            <h3 class="font-bold text-lg text-blue-400 border-b border-gray-600 pb-1 pt-4">Button Pad</h3>
                            <div>
                                <label class="block mb-1 text-gray-400">Number of Physical Buttons (0-15):</label>
                                <input type="number" name="numButtons" min="0" max="15" class="form-input w-full p-2 rounded">
                            </div>
                             <div>
                                <label class="block mb-1 text-gray-400">STOP ALL Button Number (0=Off, 1-15):</label>
                                <input type="number" name="stopAllButton" min="0" max="15" class="form-input w-full p-2 rounded">
                            </div>
                        </div>
                    </div>
                    <div class="text-center mt-6"><button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button></div>
                </form>
            </div>

             <!-- System Page -->
            <div id="system" class="page" style="display:none;">
                <div class="form-card p-6 rounded-lg max-w-xl mx-auto">
                    <h3 class="font-bold text-xl text-blue-400 mb-4 text-center">System Configuration</h3>
                    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
                        <div class="bg-gray-800 p-4 rounded-lg text-center">
                            <h4 class="font-semibold text-lg mb-2">Export</h4>
                            <p class="text-sm text-gray-400 mb-4">Download the current configuration as a JSON file to create a backup.</p>
                            <a href="/export-config" download="krayt_config.json" class="inline-block bg-green-500 hover:bg-green-600 text-white font-bold py-2 px-6 rounded-lg">Export</a>
                        </div>
                        <div class="bg-gray-800 p-4 rounded-lg text-center">
                            <h4 class="font-semibold text-lg mb-2">Import</h4>
                            <p class="text-sm text-gray-400 mb-4">Upload a saved JSON file to restore a previous configuration.</p>
                            <form id="form-import" method="POST" action="/import-config" enctype="multipart/form-data">
                                <input type="file" name="configfile" id="configfile" class="hidden" onchange="document.getElementById('upload-btn').style.display='inline-block'">
                                <label for="configfile" class="inline-block bg-gray-600 hover:bg-gray-700 text-white font-bold py-2 px-6 rounded-lg cursor-pointer">Choose File</label>
                                <button type="submit" id="upload-btn" class="hidden mt-2 bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Upload & Reboot</button>
                            </form>
                        </div>
                    </div>
                </div>
            </div>

            <!-- WiFi Page -->
            <div id="wifi" class="page" style="display:none;">
                <form id="form-wifi" class="form-card p-6 rounded-lg max-w-2xl mx-auto">
                    <div class="text-center mb-6">
                        <label class="block mb-2 text-gray-400">Active Wi-Fi Mode:</label>
                        <div class="inline-flex bg-gray-700 rounded-lg p-1">
                            <label class="flex items-center space-x-2 px-4 py-1 rounded-md cursor-pointer">
                                <input type="radio" name="wifiMode" value="0" class="form-radio text-blue-500">
                                <span>Hotspot (AP)</span>
                            </label>
                            <label class="flex items-center space-x-2 px-4 py-1 rounded-md cursor-pointer">
                                <input type="radio" name="wifiMode" value="1" class="form-radio text-blue-500">
                                <span>Home WiFi (STA)</span>
                            </label>
                        </div>
                    </div>
                    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
                        <div class="bg-gray-800 p-4 rounded-lg">
                            <h3 class="font-bold text-lg text-blue-400 mb-2">Hotspot Settings</h3>
                            <div><label class="block mb-1 text-sm text-gray-400">SSID:</label><input type="text" name="ap_ssid" class="form-input w-full p-2 rounded"></div>
                            <div class="mt-4"><label class="block mb-1 text-sm text-gray-400">Password:</label><input type="password" name="ap_password" class="form-input w-full p-2 rounded"></div>
                        </div>
                        <div class="bg-gray-800 p-4 rounded-lg">
                            <h3 class="font-bold text-lg text-blue-400 mb-2">Home WiFi Settings</h3>
                            <div><label class="block mb-1 text-sm text-gray-400">SSID:</label><input type="text" name="sta_ssid" class="form-input w-full p-2 rounded"></div>
                            <div class="mt-4"><label class="block mb-1 text-sm text-gray-400">Password:</label><input type="password" name="sta_password" class="form-input w-full p-2 rounded"></div>
                        </div>
                    </div>
                    <div class="text-center mt-6"><button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button></div>
                </form>
            </div>
            
            <!-- PID Page -->
            <div id="pid" class="page" style="display:none;">
                 <form id="form-pid" class="form-card p-6 rounded-lg max-w-2xl mx-auto">
                    <div class="space-y-4">
                        <div class="bg-gray-800 p-4 rounded-lg">
                            <h3 class="font-bold text-lg text-blue-400 mb-2">ESP-Now Communication</h3>
                            <div>
                                <label class="block mb-1 text-sm text-gray-400">This ESP32's MAC Address (Read-Only):</label>
                                <div id="host-mac-display" class="form-input w-full p-2 rounded bg-gray-600 cursor-not-allowed"></div>
                            </div>
                             <div class="mt-4">
                                <label class="block mb-1 text-sm text-gray-400">Dome ESP32's MAC Address:</label>
                                <input type="text" name="dome_mac" class="form-input w-full p-2 rounded" placeholder="e.g., AA:BB:CC:DD:EE:FF">
                            </div>
                        </div>

                        <div class="bg-gray-800 p-4 rounded-lg">
                            <h3 class="font-bold text-lg text-blue-400 mb-2">PID Values</h3>
                             <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
                                <div><label class="block mb-1 text-sm text-gray-400">Kp:</label><input type="number" step="0.01" name="pid_kp" class="form-input w-full p-2 rounded"></div>
                                <div><label class="block mb-1 text-sm text-gray-400">Ki:</label><input type="number" step="0.01" name="pid_ki" class="form-input w-full p-2 rounded"></div>
                                <div><label class="block mb-1 text-sm text-gray-400">Kd:</label><input type="number" step="0.01" name="pid_kd" class="form-input w-full p-2 rounded"></div>
                            </div>
                        </div>

                        <div class="bg-gray-800 p-4 rounded-lg">
                            <h3 class="font-bold text-lg text-blue-400 mb-2">Live IMU Data</h3>
                            <div class="grid grid-cols-2 gap-4 font-mono text-center">
                                <div class="bg-gray-900 p-2 rounded">
                                    <div class="text-sm text-gray-400">Accel X</div>
                                    <div id="imu-ax" class="text-xl">0.00</div>
                                </div>
                                <div class="bg-gray-900 p-2 rounded">
                                    <div class="text-sm text-gray-400">Gyro X</div>
                                    <div id="imu-gx" class="text-xl">0.00</div>
                                </div>
                                <div class="bg-gray-900 p-2 rounded">
                                    <div class="text-sm text-gray-400">Accel Y</div>
                                    <div id="imu-ay" class="text-xl">0.00</div>
                                </div>
                                <div class="bg-gray-900 p-2 rounded">
                                    <div class="text-sm text-gray-400">Gyro Y</div>
                                    <div id="imu-gy" class="text-xl">0.00</div>
                                </div>
                                <div class="bg-gray-900 p-2 rounded">
                                    <div class="text-sm text-gray-400">Accel Z</div>
                                    <div id="imu-az" class="text-xl">0.00</div>
                                </div>
                                <div class="bg-gray-900 p-2 rounded">
                                    <div class="text-sm text-gray-400">Gyro Z</div>
                                    <div id="imu-gz" class="text-xl">0.00</div>
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="text-center mt-6"><button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button></div>
                </form>
            </div>

            <!-- Automation Page -->
            <div id="automation" class="page" style="display:none;">
                <form id="form-automation" class="form-card p-6 rounded-lg">
                    <h3 class="font-bold text-xl text-blue-400 mb-4 text-center">Automation Sequence Settings</h3>
                    <div class="space-y-4 max-w-4xl mx-auto">
                        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
                            <div>
                                <label class="block mb-1 text-gray-400">Description:</label>
                                <input type="text" name="auto_desc" class="form-input w-full p-2 rounded">
                            </div>
                            <div>
                                <label class="block mb-1 text-gray-400">Delay (Min/Max Seconds):</label>
                                <div class="flex gap-2">
                                    <input type="number" name="auto_delayMin" class="form-input w-1/2 p-2 rounded">
                                    <input type="number" name="auto_delayMax" class="form-input w-1/2 p-2 rounded">
                                </div>
                            </div>
                            <div>
                                <label class="block mb-1 text-gray-400">Sound (Min/Max):</label>
                                <div class="flex gap-2">
                                    <input type="number" name="auto_soundMin" class="form-input w-1/2 p-2 rounded">
                                    <input type="number" name="auto_soundMax" class="form-input w-1/2 p-2 rounded">
                                </div>
                            </div>
                            <div>
                                <label class="block mb-1 text-gray-400">Maestro Script (Min/Max):</label>
                                <div class="flex gap-2">
                                    <input type="number" name="auto_maestroMin" class="form-input w-1/2 p-2 rounded">
                                    <input type="number" name="auto_maestroMax" class="form-input w-1/2 p-2 rounded">
                                </div>
                            </div>
                        </div>
                        <div class="bg-gray-800 p-4 rounded-lg">
                            <h4 class="font-bold text-lg text-blue-400 mb-2">State Change Serial Commands</h4>
                            <div>
                                <label class="block mb-1 text-gray-400">Automation ON Serial Command:</label>
                                <input type="text" name="auto_serial_on" class="form-input w-full p-2 rounded">
                            </div>
                            <div class="mt-4">
                                <label class="block mb-1 text-gray-400">Automation OFF Serial Command:</label>
                                <input type="text" name="auto_serial_off" class="form-input w-full p-2 rounded">
                            </div>
                        </div>
                    </div>
                    <div class="text-center mt-6">
                        <button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button>
                    </div>
                </form>
            </div>

            <!-- PWM Page -->
            <div id="pwm" class="page" style="display:none;">
                <form id="form-pwm" class="form-card p-6 rounded-lg">
                    <div class="bg-gray-800 p-4 rounded-lg mb-4 max-w-sm mx-auto">
                        <h3 class="text-lg text-center font-semibold text-blue-400">Live SBUS Value</h3>
                        <div id="sbus-value-display" class="text-center text-3xl font-mono p-2 bg-gray-900 rounded mt-2">...</div>
                    </div>
                    <div class="pwm-container grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 gap-4">
                        <!-- JS will populate this -->
                    </div>
                    <div class="text-center mt-6"><button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button></div>
                </form>
            </div>

            <!-- Buttons RC Page -->
            <div id="buttons-rc" class="page" style="display:none;">
                <form id="form-buttons-rc" class="form-card p-6 rounded-lg">
                    <div class="grid-container grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4"></div>
                    <div class="text-center mt-6"><button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button></div>
                </form>
            </div>
            
            <!-- Buttons 1 Page -->
            <div id="buttons1" class="page" style="display:none;">
                <form id="form-buttons1" class="form-card p-6 rounded-lg">
                    <div class="grid-container grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4"></div>
                    <div class="text-center mt-6"><button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button></div>
                </form>
            </div>

            <!-- Buttons 2 Page -->
            <div id="buttons2" class="page" style="display:none;">
                <form id="form-buttons2" class="form-card p-6 rounded-lg">
                    <div class="grid-container grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4"></div>
                    <div class="text-center mt-6"><button type="submit" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-6 rounded-lg">Save & Reboot</button></div>
                </form>
            </div>
        </main>
    </div>

<script>
let configData = {};
let sbusInterval;
let imuInterval; // New interval for IMU

function showPage(pageId) {
    document.querySelectorAll('.page').forEach(p => p.style.display = 'none');
    document.getElementById(pageId).style.display = 'block';
    document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
    document.querySelector(`.nav-link[href="#${pageId}"]`).classList.add('active');
    window.location.hash = pageId;

    if (pageId === 'pwm') startSbusFetching();
    else stopSbusFetching();

    if (pageId === 'pid') startImuFetching(); 
    else stopImuFetching(); 
}

function startSbusFetching() {
    if (sbusInterval) return;
    sbusInterval = setInterval(() => {
        fetch('/sbus')
            .then(response => response.text())
            .then(value => {
                const display = document.getElementById('sbus-value-display');
                if(display) display.textContent = value;
            }).catch(err => {
                console.error("SBUS fetch error:", err);
                stopSbusFetching();
            });
    }, 200);
}

function stopSbusFetching() {
    clearInterval(sbusInterval);
    sbusInterval = null;
}

function startImuFetching() {
    if (imuInterval) return;
    imuInterval = setInterval(() => {
        fetch('/imu')
            .then(response => response.json())
            .then(data => {
                document.getElementById('imu-ax').textContent = data.ax.toFixed(2);
                document.getElementById('imu-ay').textContent = data.ay.toFixed(2);
                document.getElementById('imu-az').textContent = data.az.toFixed(2);
                document.getElementById('imu-gx').textContent = data.gx.toFixed(2);
                document.getElementById('imu-gy').textContent = data.gy.toFixed(2);
                document.getElementById('imu-gz').textContent = data.gz.toFixed(2);
            }).catch(err => {
                console.error("IMU fetch error:", err);
                stopImuFetching();
            });
    }, 250); 
}

function stopImuFetching() {
    clearInterval(imuInterval);
    imuInterval = null;
}


window.onload = function() {
    const pageId = window.location.hash.substring(1) || 'general';
    
    fetch('/config').then(res => res.json()).then(data => {
        configData = data;
        updateNav();
        populateForms();
        showPage(pageId); 
    }).catch(err => console.error("Error fetching config:", err));

    // Normal form saves
    document.getElementById('form-general').addEventListener('submit', e => saveForm(e, '/save-general'));
    document.getElementById('form-wifi').addEventListener('submit', e => saveForm(e, '/save-wifi'));
    document.getElementById('form-pid').addEventListener('submit', e => saveForm(e, '/save-pid'));
    document.getElementById('form-automation').addEventListener('submit', e => saveForm(e, '/save-automation'));
    document.getElementById('form-pwm').addEventListener('submit', e => saveForm(e, '/save-pwm'));
    document.getElementById('form-buttons-rc').addEventListener('submit', e => saveForm(e, '/save-buttons-rc'));
    document.getElementById('form-buttons1').addEventListener('submit', e => saveForm(e, '/save-buttons1'));
    document.getElementById('form-buttons2').addEventListener('submit', e => saveForm(e, '/save-buttons2'));

    // Special handler for import form to give user feedback
    document.getElementById('form-import').addEventListener('submit', function(event) {
        event.preventDefault();
        const formData = new FormData(this);
        const fileInput = document.getElementById('configfile');
        
        if (!fileInput.files || fileInput.files.length === 0) {
            alert('Please choose a file to upload.');
            return;
        }

        fetch('/import-config', {
            method: 'POST',
            body: formData,
        })
        .then(response => response.text())
        .then(result => {
            alert(result); // Show "Rebooting..." message from server
            // The server will reboot, so no further action is needed here.
        })
        .catch(error => {
            console.error('Error:', error);
            alert('Upload failed!');
        });
    });
};

function updateNav() {
    const numButtons = configData.numButtons || 0;
    const btn1Tab = document.querySelector('a[href="#buttons1"]');
    const btn2Tab = document.querySelector('a[href="#buttons2"]');
    
    if (numButtons > 0) {
        btn1Tab.style.display = 'inline-block';
        btn2Tab.style.display = 'inline-block';
    } else {
        btn1Tab.style.display = 'none';
        btn2Tab.style.display = 'none';
    }
}


function macArrayToString(macArray) {
    if (!macArray || macArray.length !== 6) return "00:00:00:00:00:00";
    return macArray.map(byte => byte.toString(16).toUpperCase().padStart(2, '0')).join(':');
}

function populateForms() {
    // General
    document.querySelector('#form-general [name="_buttonsCH"]').value = configData._buttonsCH;
    document.querySelector('#form-general [name="_ToggleCH"]').value = configData._ToggleCH;
    document.querySelector('#form-general [name="_rcButtons12CH"]').value = configData._rcButtons12CH;
    document.querySelector('#form-general [name="_rcButtons34CH"]').value = configData._rcButtons34CH;
    document.querySelector('#form-general [name="_rcButtons56CH"]').value = configData._rcButtons56CH;
    document.querySelector('#form-general [name="_volumeCH"]').value = configData._volumeCH;
    document.querySelector('#form-general [name="_wifiToggleCH"]').value = configData._wifiToggleCH;
    document.querySelector('#form-general [name="_automationToggleCH"]').value = configData._automationToggleCH;
    document.querySelector('#form-general [name="_maestroCount"]').value = configData._maestroCount;
    document.querySelector('#form-general [name="enableSerial"]').checked = configData.enableSerial;
    document.querySelector('#form-general [name="numButtons"]').value = configData.numButtons;
    document.querySelector('#form-general [name="stopAllButton"]').value = configData.stopAllButton;


    // WiFi
    document.querySelector(`#form-wifi input[name="wifiMode"][value="${configData.wifiMode}"]`).checked = true;
    document.querySelector('#form-wifi [name="ap_ssid"]').value = configData.ap_ssid;
    document.querySelector('#form-wifi [name="ap_password"]').value = configData.ap_password;
    document.querySelector('#form-wifi [name="sta_ssid"]').value = configData.sta_ssid;
    document.querySelector('#form-wifi [name="sta_password"]').value = configData.sta_password;

    // PID
    document.getElementById('host-mac-display').textContent = macArrayToString(configData.host_mac);
    document.querySelector('#form-pid [name="dome_mac"]').value = macArrayToString(configData.dome_mac);
    document.querySelector('#form-pid [name="pid_kp"]').value = configData.pid_kp;
    document.querySelector('#form-pid [name="pid_ki"]').value = configData.pid_ki;
    document.querySelector('#form-pid [name="pid_kd"]').value = configData.pid_kd;

    // Automation
    if (configData.automation) {
        document.querySelector('#form-automation [name="auto_desc"]').value = configData.automation.description;
        document.querySelector('#form-automation [name="auto_delayMin"]').value = configData.automation.delayMin;
        document.querySelector('#form-automation [name="auto_delayMax"]').value = configData.automation.delayMax;
        document.querySelector('#form-automation [name="auto_soundMin"]').value = configData.automation.soundMin;
        document.querySelector('#form-automation [name="auto_soundMax"]').value = configData.automation.soundMax;
        document.querySelector('#form-automation [name="auto_maestroMin"]').value = configData.automation.maestroMin;
        document.querySelector('#form-automation [name="auto_maestroMax"]').value = configData.automation.maestroMax;
        document.querySelector('#form-automation [name="auto_serial_on"]').value = configData.automation.serialCommandOn;
        document.querySelector('#form-automation [name="auto_serial_off"]').value = configData.automation.serialCommandOff;
    }
    
    // PWM Page
    const pwmContainer = document.querySelector('#pwm .pwm-container');
    let pwmHtml = '';
    for (let i = 0; i < 15; i++) {
        pwmHtml += `
            <div class="flex items-center justify-between bg-gray-700 p-2 rounded">
                <label class="font-bold">Button ${i + 1}:</label>
                <input type="number" name="pwm${i}" value="${configData.pwmButtonValues[i]}" class="form-input w-28 p-1 rounded text-center">
            </div>`;
    }
    pwmContainer.innerHTML = pwmHtml;

    // Buttons
    document.querySelector('#buttons-rc .grid-container').innerHTML = createButtonInputs(0, 6, configData.actions_rc, "RC");
    document.querySelector('#buttons1 .grid-container').innerHTML = createButtonInputs(0, configData.numButtons || 0, configData.actions_p1, "P1");
    document.querySelector('#buttons2 .grid-container').innerHTML = createButtonInputs(0, configData.numButtons || 0, configData.actions_p2, "P2");
}

function createButtonInputs(start, count, actions, prefix) {
    let html = '';
    const maestroCount = configData._maestroCount || 0;
    const enableSerial = configData.enableSerial || false;
    const stopButton = configData.stopAllButton || 0;
    const buttonLabels = {
        "RC": ["1 (Low)", "2 (High)", "3 (Low)", "4 (High)", "5 (Low)", "6 (High)"]
    };

    for (let i = start; i < count; i++) {
        const action = actions[i] || {};
        const isStopButton = (prefix !== "RC" && (i + 1) == stopButton);
        
        let buttonLabel = `${prefix}-${i + 1}`;
        if (prefix === "RC" && buttonLabels["RC"][i]) {
            buttonLabel = `RC ${buttonLabels["RC"][i]}`;
        }
        
        const isDisabled = isStopButton ? 'disabled' : '';
        const cardClass = isStopButton ? 'grid-item disabled' : 'grid-item';
        const nameValue = isStopButton ? 'STOP ALL' : (action.name || '');

        html += `<div class="${cardClass} p-3 rounded-md">
            <h3 class="font-bold text-lg text-blue-400 mb-2">${buttonLabel}</h3>
            <div class="space-y-2 text-sm">
                <div><label>Name:</label><input type="text" name="b${i}_name" value="${nameValue}" class="form-input w-full p-1 rounded" ${isDisabled}></div>
                <div><label>Sound (Min/Max):</label>
                    <div class="flex gap-1">
                        <input type="number" name="b${i}_soundMin" value="${action.soundMin || 0}" class="form-input w-1/2 p-1 rounded" ${isDisabled}>
                        <input type="number" name="b${i}_soundMax" value="${action.soundMax || 0}" class="form-input w-1/2 p-1 rounded" ${isDisabled}>
                    </div>
                </div>
                <div><label>Delay:</label><input type="number" name="b${i}_delay" value="${action.delay || 0}" class="form-input w-full p-1 rounded" ${isDisabled}></div>
                
                <div class="${maestroCount >= 1 ? '' : 'hidden'}"><label>Maestro 1 Script:</label><input type="number" name="b${i}_m1" value="${action.maestro1Script || 0}" class="form-input w-full p-1 rounded" ${isDisabled}></div>
                <div class="${maestroCount >= 2 ? '' : 'hidden'}"><label>Maestro 2 Script:</label><input type="number" name="b${i}_m2" value="${action.maestro2Script || 0}" class="form-input w-full p-1 rounded" ${isDisabled}></div>
                <div class="${enableSerial ? '' : 'hidden'}"><label>Serial Cmd:</label><input type="text" name="b${i}_serial" value="${action.serialCommand || ''}" class="form-input w-full p-1 rounded" ${isDisabled}></div>
            </div>
        </div>`;
    }
    return html;
}

function saveForm(event, url) {
    event.preventDefault();
    const formData = new FormData(event.target);
    fetch(url, { method: 'POST', body: formData })
        .then(res => res.ok ? res.text() : Promise.reject('Save failed'))
        .then(text => { alert(text); })
        .catch(err => { alert(err); });
}
</script>
</body>
</html>
)rawliteral";

