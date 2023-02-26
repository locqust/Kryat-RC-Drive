int soundTriggerState = 0;
int soundTriggerState1 = 0;
int soundTriggerState2 = 0;
int soundTriggerState3 = 0;
int soundTriggerState4 = 0;
int soundTriggerStateauto = 0;
int Pad = 1;



void check_sound()
 {
  if     
   ((soundTriggerState1 == 0) && (soundTriggerState2 == 0) && (soundTriggerState3 == 0) && (soundTriggerState4 == 0))
   //Allow sound to play
   {
   play_sound(); 
   //Serial.println("Trigger is open"); 
  }  
//Reset triggers so sound will work again
 else if  
 //Reset SwitchA
    (soundTriggerState1 == 1)    
     {  
      // Serial.println("Checking status");
       if ((sbus_rx.data().ch[CH_Buttons_1_2]) == RC_MID)
       {
          //soundTriggerState = 0;  
          soundTriggerState1 = 0;
          Serial.println("Trigger1 reopened");       
       }
     } 
  else if  
  //Reset SwitchB
    (soundTriggerState2 == 1)    
     {  
       //Serial.println("Checking status");
       if ((sbus_rx.data().ch[CH_Buttons_3_4]) == RC_MID)
       {
          //soundTriggerState = 0;  
          soundTriggerState2 = 0;
          Serial.println("Trigger2 reopened");       
       }
     }   
     
  else if  
  //Reset SwitchC
    (soundTriggerState3 == 1)    
     {  
      // Serial.println("Checking status");
       if ((sbus_rx.data().ch[CH_Buttons_5_6]) == RC_MID)
       {
          //soundTriggerState = 0;  
          soundTriggerState3 = 0;
          Serial.println("Trigger3 reopened");       
       }
     }   

  else if  
  //Reset Button pad
    (soundTriggerState4 == 1)  
     {  
      // Serial.println("Checking status");
       if ((sbus_rx.data().ch[CH_Button_Pad]) == Resting)
       {
          //soundTriggerState = 0;  
          soundTriggerState4 = 0;
          Serial.println("Trigger pad reopened");       
       }
     }      
}  

void toggle_pad()
{
  if 
    (sbus_rx.data().ch[CH_Button_Toggle] == RC_MAX)
      {
        Pad = 2;
       // Serial.println("Button pad 2 selected");
      }    
  else if 
    (sbus_rx.data().ch[CH_Button_Toggle] == RC_MIN)
      {
        Pad = 1;
      //  Serial.println("Button pad 1 selected");
      } 
}


void play_sound()
{
  if (sbus_rx.data().ch[CH_Buttons_1_2] == RC_MIN)
  //SwitchA
  {
    myDFPlayer.play(random(10,12));
    Serial.println("Button 1");
    //soundTriggerState = 1;
    Serial.println("Trigger 1 closed");
    soundTriggerState1 = 1;
   }
  else if 
    (sbus_rx.data().ch[CH_Buttons_1_2] == RC_MAX)
    //SwitchA 
    {    
    myDFPlayer.play(random(20, 22));
    Serial.println("Button 2");
    //soundTriggerState = 1;
    Serial.println("Trigger 2 closed");
    soundTriggerState1 = 1;
    }              
  else if 
    (sbus_rx.data().ch[CH_Buttons_3_4] == RC_MIN)
    //SwitchB 
    {
      myDFPlayer.play(random(5,7));
      Serial.println("Button 3");
      //soundTriggerState = 1;
      Serial.println("Trigger 3 closed");
      soundTriggerState2 = 1;
    }
  else if 
    (sbus_rx.data().ch[CH_Buttons_3_4] == RC_MAX)
    //SwitchB
    {
      myDFPlayer.play(random(1, 4));
      Serial.println("Button 4");
      //soundTriggerState = 1;
      Serial.println("Trigger 4 closed");
      soundTriggerState2 = 1;
    }              
  else if
    (sbus_rx.data().ch[CH_Buttons_5_6] == RC_MIN)
    //SwitchB    
     {
        myDFPlayer.play(random(16, 19));
        Serial.println("Button 5");
        //soundTriggerState = 1;
        Serial.println("Trigger 5 closed");
        soundTriggerState3 = 1;
      }
  else if 
    (sbus_rx.data().ch[CH_Buttons_5_6] == RC_MAX) 
    //SwitchC
      {
        myDFPlayer.play(random(23, 25));
        Serial.println("Button 6");
        //soundTriggerState = 1;
        Serial.println("Trigger 6 closed");
        soundTriggerState3 = 1;
      }  
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button1)
      {    
      if (Pad == 1)
      {
        myDFPlayer.play(random(2, 4));
        Serial.println("Pad 1-1");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-1 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(4, 6));
        Serial.println("Pad 2-1");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-1 closed");
        soundTriggerState4 = 1;
      } 
     }         
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button2)
    {    
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-2");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-2 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-2");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-2 closed");
        soundTriggerState4 = 1;
      }
    }       
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button3)
    {
      if (Pad == 1)
      {
        myDFPlayer.play(random(10, 12));
        Serial.println("Pad 1-3");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-3 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(12, 14));
        Serial.println("Pad 2-3");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-3 closed");
        soundTriggerState4 = 1;
      }   
    }  
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button4)
    {
      if (Pad == 1)
      {
        myDFPlayer.play(random(14, 16));
        Serial.println("Pad 1-4");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-4 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(16, 18));
        Serial.println("Pad 2-4");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-4 closed");
        soundTriggerState4 = 1;
      }
    } 
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button5)
    {
      if (Pad == 1)
      {
        myDFPlayer.play(random(18, 20));
        Serial.println("Pad 1-5");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-5 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(20, 22));
        Serial.println("Pad 2-5");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-5 closed");
        soundTriggerState4 = 1;
      }
    } 
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button6)
    {
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-6");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-6 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-6");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-6 closed");
        soundTriggerState4 = 1;
      }
    }
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button7)
    {
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-7");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-7 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-7");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-7 closed");
        soundTriggerState4 = 1;
      }
    } 
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button8)
     {
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-8");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-8 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-8");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-8 closed");
        soundTriggerState4 = 1;
      }
     }  
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button9)
    {
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-9");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-9 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-9");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-9 closed");
        soundTriggerState4 = 1;
      }
    }        
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button10)
    { 
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-10");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-10 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-10");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-10 closed");
        soundTriggerState4 = 1;
      }
    }  
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button11)
     {
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-11");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-11 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-11");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-11 closed");
        soundTriggerState4 = 1;
      }
     }   
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button12)
     { 
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-12");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-12 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-12");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-12 closed");
        soundTriggerState4 = 1;
      }
     } 
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button13)
    {
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-13");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-13 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-13");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-13 closed");
        soundTriggerState4 = 1;
      }
    } 
    //Automation mode
  else if  
    (sbus_rx.data().ch[CH_Button_Pad] == Button14)
     {
      if (Pad == 1)
      {
        if(isInAutomationMode){
          isInAutomationMode = false;
          automateAction = 0;
          Serial.println("Pad 1-14");
         //soundTriggerState = 1;
          Serial.println("Trigger Automation off");
          soundTriggerState4= 1;
          myDFPlayer.play(8);
          } else {
              isInAutomationMode = true;
              myDFPlayer.play(9);
              Serial.println("Trigger Automation on");
             soundTriggerState4= 1;
          }
      } 
      else if (Pad == 2)  
      {
        if(isInAutomationMode){
          isInAutomationMode = false;
          automateAction = 0;
          Serial.println("Pad 2-14");
          //soundTriggerState = 1;
          Serial.println("Trigger Automation off");
          soundTriggerState4 = 1;
          myDFPlayer.play(8);
          } else {
              isInAutomationMode = true;
              myDFPlayer.play(9);
              Serial.println("Trigger Automation on");
              soundTriggerState4= 1;
          }
      }
     }   
  else if  
    // This will eventually be a cut off/reset button
    (sbus_rx.data().ch[CH_Button_Pad] == Button15)
     { 
      if (Pad == 1)
      {
        myDFPlayer.play(random(6, 8));
        Serial.println("Pad 1-15");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 1-15 closed");
        soundTriggerState4 = 1;
      } 
      else if (Pad == 2)  
      {
        myDFPlayer.play(random(8, 10));
        Serial.println("Pad 2-15");
        //soundTriggerState = 1;
        Serial.println("Trigger Pad 2-15 closed");
        soundTriggerState4 = 1;
      }
     }                                             
}  
