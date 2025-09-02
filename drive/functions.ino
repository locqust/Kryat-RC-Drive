// --- WiFi Functions ---
void setupWifi() {
  if (config.wifiMode == 0) { // AP Mode
    Serial.print("Setting up AP mode. SSID: ");
    Serial.println(config.ap_ssid);
    WiFi.softAP(config.ap_ssid, config.ap_password);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  } else { // STA Mode
    Serial.print("Connecting to STA mode SSID: ");
    Serial.println(config.sta_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.sta_ssid, config.sta_password);
    wifiReconnectMillis = millis();
  }
}

void handleWifiReconnect() {
  // Check if it's time to try reconnecting
  if (millis() - wifiReconnectMillis > 5000) { 
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to WiFi...");
      WiFi.begin(config.sta_ssid, config.sta_password);
    }
    wifiReconnectMillis = millis();
  }
}

// --- ESP-Now Functions ---
// Callback when data is sent
// Corrected the function signature's first parameter to match the expected 'wifi_tx_info_t' type.
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  // This is where you would handle confirmation of sent messages.
  // For now, we can leave it blank or add a Serial print for debugging.
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setupEspNow() {
  if (WiFi.getMode() == WIFI_OFF) {
      // ESP-Now requires WiFi to be on, even if not connected to a network.
      // We set it to STA mode as a base for ESP-Now.
      WiFi.mode(WIFI_STA);
  }
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, config.dome_mac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("ESP-Now Initialized.");
}

void send_dome_message() {
    esp_err_t result = esp_now_send(config.dome_mac, (uint8_t *) &currentDomeMessage, sizeof(currentDomeMessage));
    // if (result != ESP_OK) {
    //   Serial.println("Error sending ESP-Now data");
    // }
}
