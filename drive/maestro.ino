#include <HardwareSerial.h>

// Use Serial1 for Maestro communication.
// Connect Maestro RX to ESP32's GPIO 2 (TX1)
#define MaestroSerial Serial1

void setupMaestro() {
    // Baud rate for Pololu Maestro is typically 9600
    MaestroSerial.begin(9600);
}

// Function to send the "Stop Script" command to all active Maestros
void stopAllMaestros() {
    // The Compact Protocol "Stop Script" command (0xA4) is sent to the controller.
    // If you have multiple Maestros daisy-chained on the same serial line,
    // this command will be heard by all of them, effectively stopping all scripts.
    if (config._maestroCount > 0) {
        MaestroSerial.write(0xA4); 
        Serial.println("Sent STOP command to Maestros.");
    }
}


// Function to run a specific script on a specific Maestro
// maestroNum: 0 for Maestro #1, 1 for Maestro #2
// scriptNum: The script number to run (0-127)
void runMaestroScript(byte maestroNum, byte scriptNum) {
    // This function assumes you are using the Pololu Protocol to target specific devices
    // The device number for Maestro #1 is 12 (default), #2 is 13.
    // You must set these device numbers in the Maestro Control Center software.
    byte deviceNum = 12 + maestroNum;

    // Pololu Protocol Command: 0xAA, device number, 0x07 (Run Script), script number
    MaestroSerial.write(0xAA);
    MaestroSerial.write(deviceNum);
    MaestroSerial.write(0x07);
    MaestroSerial.write(scriptNum);
    
    Serial.print("Running script ");
    Serial.print(scriptNum);
    Serial.print(" on Maestro #");
    Serial.println(maestroNum + 1);
}

