/* 
 *  Copyright 2021 by: Gary Gurney
 *  
 */

// feedback actuator 1
#define A1_RPWM 3            // Arduino pin 3 to power controller RPWM pin 1
#define A1_LPWM 5            // Arduino pin 5 to power controller LPWM pin 2
#define A1_POT_IN A0         // arduino pin A0 to actuator feedack potentiometer
#define A1_MAX_LIMIT 900     // maximum distance actuator can travel without binding (must be less than or equal to A1_MAX_POT_VAL) 
#define A1_MIN_LIMIT 30      // minimum distance actuator can travel without binding (must be greater than or equal to A1_MIN_POT_VAL)
#define A1_MAX_POT_VAL 900   // maximum position value actuator can provide
#define A1_MIN_POT_VAL 30    // minimum position value actuator can provide
#define A1_SLOP 5            // +/- range for close enough
#define LATCH_POINT 250      // latch point for retracting

// potentiometer controller for actuator 1
#define CONTROLLER_POT_FOR_A1 A3 // arduino analog pin for controller POT

#define DEBUG false
#define MOTOR_SPEED 255

void setup() {

  // actuator 1
  pinMode(A1_RPWM, OUTPUT);
  pinMode(A1_LPWM, OUTPUT); 
  pinMode(A1_POT_IN, INPUT);

  // pot for actuator 1
  pinMode(CONTROLLER_POT_FOR_A1, INPUT);
  
  Serial.begin(9600);
}

boolean latchFlag = false;

void updateActuatorPosition(int actuatorPotIn,
                            int actuatorRPWM,
                            int actuatorLPWM,
                            int actuatorSLOP,
                            int actuatorCalibratedMax,
                            int actuatorCalibratedMin,
                            int actuatorMaxPotValue,
                            int actuatorMinPotValue,
                            int controllerPotIn) {
  
  int actuatorPotValue = analogRead(actuatorPotIn);
  int controllerPotValue = analogRead(controllerPotIn);

  //Serial.print("controllerPotValue: ");
  //Serial.println(controllerPotValue);

  // map controller range to actuator range, example: 1023 on controller maps to 755 on actuator, that way we can use the full motion of the controller
  // note that we can't use the full range of motion on the actuator, the motion range on the 1033 pick-up lever matches the actuator range from 115 to 755
  //
  // total motion range range for 1033 pick-up is: 640. The min value provided by actuator is: 30 and the value provided by atuator is: 900 
  // therefore: 870/2 is center (which is at 435) on the actuator, then add -/+ 320 to both sides of center then we have 435+320=755 max for 
  // actuator and 435-320=115 min for actuator. Map the controller's full range (0,1023) over the range of the actuator (115 to 755)
  controllerPotValue = map(controllerPotValue,0,1023,115,755);

  // close enough, don't do anything
  if (actuatorPotValue >= (controllerPotValue -actuatorSLOP) && actuatorPotValue <= (controllerPotValue +actuatorSLOP)) {

    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, 0);
  }

  // retract (extended too far)
  // todo: exponential position response?
  /*
   * For input and output ranges that go from -1 to +1, the equation is: output = ( (1 - factor) x input3 ) + ( factor x input )
   * Source https://www.physicsforums.com/threads/equation-required-to-calculate-exponential-rate.524002/
   */  
  else if (actuatorPotValue > controllerPotValue) {

    if (DEBUG) {
      Serial.print("retracting, actuatorPotValue: ");
      Serial.println(actuatorPotValue);
      Serial.print("controllerPotValue: ");
      Serial.println(controllerPotValue);
    }    

    // make speed a function of how close we are to the target value
    // if we don't do this then the actuator will overshoot the target
    long speed = MOTOR_SPEED;
    long diff = actuatorPotValue - controllerPotValue;
    if (diff < 75) {
      speed = map(diff,1,75,120,MOTOR_SPEED);
    }

    //Serial.print("retract speed: ");
    //Serial.println(speed);

    // set latch to true if retracted to latch point
    if (actuatorPotValue < LATCH_POINT) {
      Serial.println("set latchFlag TRUE");
      latchFlag = true;
    }
    
    analogWrite(actuatorRPWM, speed);
    analogWrite(actuatorLPWM, 0);
  }

  // extend (not extended far enough)
  else if (actuatorPotValue < controllerPotValue) {

    // unlatch
    if (latchFlag == true && controllerPotValue > 512) {
      latchFlag = false;
      //Serial.print("controllerPotValue: ");
      //Serial.println(controllerPotValue);
    }

    // don't extend actuator beyond LATCH_POINT
    if (latchFlag == true && actuatorPotValue > LATCH_POINT) {

      // stop!!!
      analogWrite(actuatorRPWM, 0);
      analogWrite(actuatorLPWM, 0);
    }
    else {
      
      if (DEBUG) {
        Serial.print("extending, actuatorPotValue: ");
        Serial.println(actuatorPotValue);
        Serial.print("controllerPotValue:          ");
        Serial.println(controllerPotValue);
      }
  
      // make speed a function of how close we are to the target value
      // if we don't do this then the actuator will overshoot the target
      long speed = MOTOR_SPEED;
      long diff = controllerPotValue - actuatorPotValue;;
      if (diff < 75) {
        speed = map(diff,1,75,120,MOTOR_SPEED);
      } 
  
      //Serial.print("retract speed: ");
      //Serial.println(speed);
  
      analogWrite(actuatorRPWM, 0);
      analogWrite(actuatorLPWM, speed);
    }
  }

  // stop, not sure how we got here?
  else {
    Serial.println("***ERROR!!! UNHANDLED CONDITION!***");    
    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, 0); 
  }
}

void loop() {

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
}  
