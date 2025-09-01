unsigned long automateMillis = 0;

void triggerAutomation() {
    // Only proceed if a delay range is set
    if (config.automation.delayMax <= 0) return;

    // Check if it's time to trigger a new action
    if (millis() > automateMillis) {
        
        // 1. Trigger a random sound, if configured
        if (config.automation.soundMin > 0 && config.automation.soundMax >= config.automation.soundMin) {
            int soundToPlay = random(config.automation.soundMin, config.automation.soundMax + 1);
            myDFPlayer.play(soundToPlay);
            Serial.print("Automation played sound: ");
            Serial.println(soundToPlay);
        }

        // 2. Trigger a random Maestro script, if configured
        if (config._maestroCount > 0 && config.automation.maestroMin > 0 && config.automation.maestroMax >= config.automation.maestroMin) {
            int scriptToRun = random(config.automation.maestroMin, config.automation.maestroMax + 1);
            
            // Randomly pick which Maestro to run it on if both are enabled
            int maestroTarget = 0;
            if (config._maestroCount == 2) {
                maestroTarget = random(0, 2); // Result is 0 or 1
            }
            runMaestroScript(maestroTarget, scriptToRun);
        }
        
        // 3. Set the timer for the next action
        int delaySeconds = random(config.automation.delayMin, config.automation.delayMax + 1);
        automateMillis = millis() + (delaySeconds * 1000);
        Serial.print("Next automation action in: ");
        Serial.print(delaySeconds);
        Serial.println(" seconds.");
    }
}

