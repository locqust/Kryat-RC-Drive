// This variable holds which pad page is active (1 or 2)
int Pad = 1;

// These are state-tracking variables to prevent a button from being held down and firing continuously.
// A sound can only play again once the switch has been returned to its middle/resting position.
int soundTriggerState1 = 0; // For RC Buttons 1/2
int soundTriggerState2 = 0; // For RC Buttons 3/4
int soundTriggerState3 = 0; // For RC Buttons 5/6
int soundTriggerState4 = 0; // For the 15-button Pad

// --- HELPER FUNCTIONS ---

// Helper function to process an action (sound, maestro, serial)
// This avoids repeating the same block of code for every single button.
void processAction(const ButtonAction& action) {
  // 1. Play Sound
  // Check if a sound is configured (min value is not 0)
  if (action.soundMin > 0) {
    if (action.soundMax > action.soundMin) {
      // If min and max are different, play a random sound in that range
      myDFPlayer.play(random(action.soundMin, action.soundMax + 1));
    } else {
      // If min and max are the same, play that single sound file
      myDFPlayer.play(action.soundMin);
    }
  }

  // 2. Trigger Maestro Scripts
  // Check if Maestros are enabled in config and a script number is set (not 0)
  if (config._maestroCount >= 1 && action.maestro1Script > 0) {
    runMaestroScript(0, action.maestro1Script); // Target Maestro #1 (device 0)
  }
  if (config._maestroCount >= 2 && action.maestro2Script > 0) {
    runMaestroScript(1, action.maestro2Script); // Target Maestro #2 (device 1)
  }

  // 3. Send Serial Command
  // Check if serial is enabled and the command string is not empty
  if (config.enableSerial && strlen(action.serialCommand) > 0) {
    // This prints to the main USB serial port for debugging.
    // You would send this to another serial port for external devices.
    Serial.print("Sending Serial Command: ");
    Serial.println(action.serialCommand);
  }
}

// --- MAIN SOUND AND ACTION LOGIC ---

// This function checks the RC switches and the button pad
void play_sound() {

  // --- RC Buttons ---
  // Each block checks a 3-position switch (LOW, MID, HIGH)

  // Check RC Buttons 1 & 2 on their assigned channel
  if (soundTriggerState1 == 0 && config._rcButtons12CH > 0) {
    int channel = config._rcButtons12CH - 1;
    if (sbus_rx.data().ch[channel] < RC_DEADBAND_LOW) { // Switch is LOW
      processAction(config.actions_rc[0]); // Process action for RC Button 1
      soundTriggerState1 = 1; // Set trigger so it won't fire again until centered
    } else if (sbus_rx.data().ch[channel] > RC_DEADBAND_HIGH) { // Switch is HIGH
      processAction(config.actions_rc[1]); // Process action for RC Button 2
      soundTriggerState1 = 1;
    }
  }

  // Check RC Buttons 3 & 4
  if (soundTriggerState2 == 0 && config._rcButtons34CH > 0) {
    int channel = config._rcButtons34CH - 1;
    if (sbus_rx.data().ch[channel] < RC_DEADBAND_LOW) {
      processAction(config.actions_rc[2]);
      soundTriggerState2 = 1;
    } else if (sbus_rx.data().ch[channel] > RC_DEADBAND_HIGH) {
      processAction(config.actions_rc[3]);
      soundTriggerState2 = 1;
    }
  }

  // Check RC Buttons 5 & 6
  if (soundTriggerState3 == 0 && config._rcButtons56CH > 0) {
    int channel = config._rcButtons56CH - 1;
    if (sbus_rx.data().ch[channel] < RC_DEADBAND_LOW) {
      processAction(config.actions_rc[4]);
      soundTriggerState3 = 1;
    } else if (sbus_rx.data().ch[channel] > RC_DEADBAND_HIGH) {
      processAction(config.actions_rc[5]);
      soundTriggerState3 = 1;
    }
  }

  // --- 15 Button Pad ---
  if (soundTriggerState4 == 0 && config._buttonsCH > 0 && config.numButtons > 0) {
    int pad_channel = config._buttonsCH - 1;
    
    // Check if the button pad is sending a value (i.e., not in its resting state)
    if (sbus_rx.data().ch[pad_channel] != Resting) {
      
      // Loop through the number of configured physical buttons
      for (int i = 0; i < config.numButtons; i++) {
        
        // Check if the live SBUS value matches the saved value for this button
        if (sbus_rx.data().ch[pad_channel] == config.pwmButtonValues[i]) {

          // Check if this button is the designated STOP ALL button
          if ((i + 1) == config.stopAllButton && config.stopAllButton > 0) {
              Serial.println("STOP ALL Triggered!");
              myDFPlayer.stop();
              stopAllMaestros();
          } else {
            // It's a regular button, so process its action based on which Pad page is active
            if (Pad == 1) {
              processAction(config.actions_p1[i]);
            } else { // Pad == 2
              processAction(config.actions_p2[i]);
            }
          }
          
          soundTriggerState4 = 1; // Set trigger so it won't fire again until released
          break; // Exit the loop because we found the pressed button.
        }
      }
    }
  }
}

// This function checks if the triggers can be reset, allowing another sound/action.
void check_sound() {
  if (soundTriggerState1 == 0 && soundTriggerState2 == 0 && soundTriggerState3 == 0 && soundTriggerState4 == 0) {
    // All triggers are open, so we can check for new button presses.
    play_sound();
  } else {
    // One or more triggers are closed, check if they can be re-opened.
    if (soundTriggerState1 == 1 && config._rcButtons12CH > 0) {
      if (sbus_rx.data().ch[config._rcButtons12CH - 1] > RC_DEADBAND_LOW && sbus_rx.data().ch[config._rcButtons12CH - 1] < RC_DEADBAND_HIGH) {
        soundTriggerState1 = 0; // Switch is in the middle, re-open the trigger.
      }
    }
    if (soundTriggerState2 == 1 && config._rcButtons34CH > 0) {
      if (sbus_rx.data().ch[config._rcButtons34CH - 1] > RC_DEADBAND_LOW && sbus_rx.data().ch[config._rcButtons34CH - 1] < RC_DEADBAND_HIGH) {
        soundTriggerState2 = 0;
      }
    }
    if (soundTriggerState3 == 1 && config._rcButtons56CH > 0) {
      if (sbus_rx.data().ch[config._rcButtons56CH - 1] > RC_DEADBAND_LOW && sbus_rx.data().ch[config._rcButtons56CH - 1] < RC_DEADBAND_HIGH) {
        soundTriggerState3 = 0;
      }
    }
    if (soundTriggerState4 == 1 && config._buttonsCH > 0) {
      if (sbus_rx.data().ch[config._buttonsCH - 1] == Resting) {
        soundTriggerState4 = 0; // 15-button pad is in its resting state, re-open the trigger.
      }
    }
  }
}

// Toggles between Pad 1 and Pad 2 actions
void toggle_pad() {
  if (config._ToggleCH > 0) {
    if (sbus_rx.data().ch[config._ToggleCH - 1] < RC_MID) {
      Pad = 1;
    } else {
      Pad = 2;
    }
  }
}

