/* 
 *  Copyright 2021 by: Gary Gurney
 *  
 */

// feedback actuator 1
#define A1_RPWM 3            // Arduino pin 3 to power controller RPWM pin 1
#define A1_LPWM 5            // Arduino pin 5 to power controller LPWM pin 2
#define A1_POT_IN A0         // arduino pin A0 to actuator feedack potentiometer
#define A1_MAX_LIMIT 900     // maximum distance actuator can travel without binding
#define A1_MIN_LIMIT 20      // minimum distance actuator can travel without binding
#define A1_MAX_POT_VAL 900   // maximum pot value actuator can provide
#define A1_MIN_POT_VAL 20    // minimum pot value actuator can provide
#define A1_SLOP 25           // +/- range for close enough

// potentiometer controller for actuator 1
#define CONTROLLER_POT_FOR_A1 A3 // arduino analog pin for controller POT

// feedback actuator 2
#define A2_RPWM 6          // Arduino pin 6 to power controller RPWM pin 1
#define A2_LPWM 9          // Arduino pin 9 to power controller LPWM pin 2
#define A2_POT_IN A1       // arduino pin A1 to actuator feedack potentiometer
#define A2_MAX_LIMIT 700   // maximum distance actuator can travel without binding
#define A2_MIN_LIMIT 300   // minimum distance actuator can travel without binding
#define A2_SLOP 30         // +/- range for close enough

// potentiometer controller for actuator 2
#define CONTROLLER_POT_FOR_A2 A4 // arduino analog pin for controller POT

// feedback actuator 3
#define A3_RPWM 10         // Arduino pin 10 to power controller RPWM pin 1
#define A3_LPWM 11         // Arduino pin 11 to power controller LPWM pin 2
#define A3_POT_IN A2       // arduino pin A2 to actuator feedack potentiometer
#define A3_MAX_LIMIT 700   // maximum distance actuator can travel without binding
#define A3_MIN_LIMIT 300   // minimum distance actuator can travel without binding
#define A3_SLOP 30         // +/- range for close enough

// potentiometer controller for actuator 3
#define CONTROLLER_POT_FOR_A3 A5 // arduino analog pin for controller POT

#define DEBUG true
#define MOTOR_SPEED 225

void setup() {

  // actuator 1
  pinMode(A1_RPWM, OUTPUT);
  pinMode(A1_LPWM, OUTPUT); 
  pinMode(A1_POT_IN, INPUT);

  // pot for actuator 1
  pinMode(CONTROLLER_POT_FOR_A1, INPUT);
  
  Serial.begin(9600);
}

void updateActuatorPosition(int actuatorPotIn,
                            int actuatorRPWM,
                            int actuatorLPWM,
                            int actuatorSLOP,
                            int actuatorCalibratedMax,
                            int actuatorCalibratedMin,
                            int actuatorMaxPotValue,
                            int actuatorMinPotValue,
                            int controllerPotIn) {

  // todo: lower resolution so actuator doesn't grind to get to exact position? normalize to a scale of 1 to 100 or something instead of 0 to 1023                              
  
  int actuatorPotValue = analogRead(actuatorPotIn);
  int controllerPotValue = analogRead(controllerPotIn);

  // controller is reqeusting position that is beyond max
  if (controllerPotValue >= actuatorCalibratedMax && actuatorPotValue >= actuatorCalibratedMax) {

    if (DEBUG) {
      Serial.println("controller is reqeusting position that is beyond max...");
      Serial.println(actuatorPotValue);
      Serial.print("controllerPotValue: ");
      Serial.println(controllerPotValue);
    }

    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, 0);
  }

  // controller is reqeusting position that is beyond min
  else if (controllerPotValue <= actuatorCalibratedMin && actuatorPotValue <= actuatorCalibratedMin) {

    if (DEBUG) {
      Serial.println("controller is reqeusting position that is beyond min...");
      Serial.println(actuatorPotValue);
      Serial.print("controllerPotValue: ");
      Serial.println(controllerPotValue);
    }
    
    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, 0);
  }
  
  // close enough, don't do anything
  else if (actuatorPotValue >= (controllerPotValue -actuatorSLOP) && actuatorPotValue <= (controllerPotValue +actuatorSLOP)) {

    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, 0);
  }

  // retract (extended too far)
  // todo: exponential response position response
  /*
   * For input and output ranges that go from -1 to +1, the equation is: output = ( (1 - factor) x input3 ) + ( factor x input )
   * Source https://www.physicsforums.com/threads/equation-required-to-calculate-exponential-rate.524002/
   */  
  else if (actuatorPotValue > controllerPotValue) /*&& actuatorPotValue < (controllerPotValue +actuatorSLOP)*/ {

    if (DEBUG) {
      Serial.print("retracting, actuatorPotValue: ");
      Serial.println(actuatorPotValue);
      Serial.print("controllerPotValue: ");
      Serial.println(controllerPotValue);
    }    
    
    analogWrite(actuatorRPWM, MOTOR_SPEED);
    analogWrite(actuatorLPWM, 0);

    // testing... stay here until retraction is completed
    //    while (actuatorPotValue >= controllerPotValue && actuatorPotValue >= actuatorMinPotValue) {
    //
    //      Serial.print("retracting, actuatorPotValue: ");
    //      Serial.println(actuatorPotValue);
    //      Serial.print("controllerPotValue: ");
    //      Serial.println(controllerPotValue);
    //      
    //      analogWrite(actuatorRPWM, MOTOR_SPEED);
    //      analogWrite(actuatorLPWM, 0);
    //      
    //      actuatorPotValue = analogRead(actuatorPotIn);
    //      delay(0);
    //    }

    // tesing... delay so it will overshoot a bit to prevent grinding
    // delay(1);
  }

  // extend (not extended far enough)
  // todo: exponential response position response
  else if (actuatorPotValue <= controllerPotValue) {

    if (DEBUG) {
      Serial.print("extending, actuatorPotValue: ");
      Serial.println(actuatorPotValue);
      Serial.print("controllerPotValue: ");
      Serial.println(controllerPotValue);
    }

    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, MOTOR_SPEED);

    // testing... stay here until extension is complete
    //    while (actuatorPotValue <= controllerPotValue && actuatorPotValue <= actuatorMaxPotValue) {
    //
    //      Serial.print("extending, actuatorPotValue: ");
    //      Serial.println(actuatorPotValue);
    //      Serial.print("controllerPotValue: ");
    //      Serial.println(controllerPotValue);
    //
    //      analogWrite(actuatorRPWM, 0);
    //      analogWrite(actuatorLPWM, MOTOR_SPEED);
    //    
    //      actuatorPotValue = analogRead(actuatorPotIn);
    //      delay(0);
    //    }

    //tesing... delay so it will overshoot a bit to prevent grinding
    // delay(1);
  }

  // stop, not sure how we got here?
  else {

   Serial.println("***ERROR!!! UNHANDLED CONDITION!***");
    
    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, 0); 
  }
}

void loop() {

  // todo: ***STARTUP CALIBRATION***  

  // ****NORNMAL LOOP****
  {

    updateActuatorPosition(A1_POT_IN,
                           A1_RPWM,
                           A1_LPWM,
                           A1_SLOP,
                           A1_MAX_LIMIT,
                           A1_MIN_LIMIT,
                           A1_MAX_POT_VAL,
                           A1_MIN_POT_VAL,
                           CONTROLLER_POT_FOR_A1);
  }

  /* ***sinulate preset location.. which is a value from 0 to 512, the positive side of the range of the potentiometer
  int presetLocation = 330;
  /*

  // ***simulate bale pickup switch is on...
  int pickupActivated = false;

  // handle pickup activated
  if (pickupActivated == true) {

    int normalizedPreviousPostiion = a1PotValue - 512;
    while (true) {  // and some timer is not exceeded
    
      // extend
      analogWrite(A1_RPWM, 0);
      analogWrite(A1_LPWM, 100/); // todo: 256
  
      feedbacka1PotValue = analogRead(CONTROLLER_POT_FOR_A1);
      int normalizedFeedbacka1PotValue = feedbacka1PotValue - 512;

      Serial.print("feedbacka1PotValue: ");
      Serial.println(feedbacka1PotValue);  
      
      Serial.print("normalizedFeedbacka1PotValue: ");
      Serial.println(normalizedFeedbacka1PotValue);  

      // are we there yet?
      if (normalizedFeedbacka1PotValue >= presetLocation) {
        break;
      }

      // don't overwhelm Serial.print()
      delay(200);
    }
  }
  */

  /* don't think this is needed
    // restore to to previous position
    {
      if (handledPickedupActivted == true) {
  
        handledPickedupActivted = false;
  
        // retract
        while (true) { // and some other timer is not exceeded
          
          analogWrite(A1_RPWM, 100 ); // todo: 256
          analogWrite(A1_LPWM, 0);
  
          // are we there yet?
          int normalizedFeedbacka1PotValue = analogRead(CONTROLLER_POT_FOR_A1) - 512;
        
          Serial.print("normalizedFeedbacka1PotValue: ");
          Serial.println(normalizedFeedbacka1PotValue);  
  
          if (normalizedFeedbacka1PotValue <= previousPostiion) {
            break;
          }
        }
  
        // don't overwhelm Serial.print()
        delay(200);
      }    
    }
  */  
}  
