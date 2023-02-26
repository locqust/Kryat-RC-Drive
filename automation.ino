//How much the dome may turn during automation.
int turnDirection = RC_MID;
byte dometurn = 0;
byte automateDelay = random(5,20);// set this to min and max seconds between sounds
unsigned long automateMillis = 0;


void triggerAutomation()
{
  // Plays random sounds or dome movements for automations when in automation mode
    unsigned long currentMillis = millis();

    if (currentMillis - automateMillis > (automateDelay * 1000)) {
      automateMillis = millis();
      automateAction = random(1, 5);

      if (automateAction < 4) {
        myDFPlayer.play(random(1, 35));
        //Dome lights
      // triggerI2C (14,random(100, 110)); 
      }
      if (automateAction = 5) {
        // sbus_tx.data().ch[CH_Dome_servo]= turnDirection;
        // sbus_tx.Write();

        // delay(750);

        // if(turnDirection > RC_MID){
        //   turnDirection = RC_MIN;
        // } else {
        //   turnDirection = RC_MAX;
      //  }
      }
      // sets the mix, max seconds between automation actions - sounds and dome movement
      automateDelay = random(5,15);
    }
}