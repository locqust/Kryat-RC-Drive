

void send_dome_message()
{
  
    // Send to dome ESP
    esp_err_t result = esp_now_send(dome_mac, (uint8_t *) &currentDomeMessage, sizeof(currentDomeMessage));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
}


