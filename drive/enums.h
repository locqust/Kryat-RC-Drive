#ifndef ENUMS_H
#define ENUMS_H

// Define the structure for the data to be sent via ESP-Now
// This now matches the structure expected by the dome sketch.
typedef struct dome_message {
  bool psi; // Flash the PSI light on the dome
  int effect; // Trigger an effect
};

#endif // ENUMS_H
