/*
 *  Copyright 2021, 2025 by: Gary Gurney
 *
 */

// feedback actuator 1
#define A1_RPWM 3            // Arduino pin 3 to power controller RPWM pin 1
#define A1_LPWM 5            // Arduino pin 5 to power controller LPWM pin 2
#define A1_POT_IN A0         // arduino pin A0 to actuator feedack potentiometer
#define A1_MAX_LIMIT 630     // maximum distance actuator can travel without binding
#define A1_MIN_LIMIT 270     // minimum distance actuator can travel without binding
#define A1_MAX_POT_VAL 943   // maximum position value actuator can provide (not used below, for reference only)
#define A1_MIN_POT_VAL 15    // minimum position value actuator can provide (not used below, for reference only)
#define A1_SLOP 5            // +/- range for close enough

// potentiometer controller for actuator 1
#define CONTROLLER_POT_FOR_A1 A3 // arduino analog pin for controller POT

#define DEBUG_CONTROLLER false
#define DEBUG_ACTUATOR false
#define DEBUG_MOTOR_SPEED false

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
  int unmappedControlerPotValue = controllerPotValue;

  // map the controller range to actuator range, for example: 1023 on controller maps to A1_MAX_LIMIT on actuator,
  // that way we can use the full motion of the controller without causing the actuator to bind
  controllerPotValue = map(controllerPotValue,0,1023,actuatorCalibratedMin,actuatorCalibratedMax);

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

    if (DEBUG_CONTROLLER) {
      Serial.print("unmappedControlerPotValue: ");
      Serial.println(unmappedControlerPotValue);
    }

    if (DEBUG_ACTUATOR) {
      Serial.print("retracting, actuatorPotValue: ");
      Serial.println(actuatorPotValue);
    }

    // make speed a function of how close we are to the target value
    // if we don't do this then the actuator will overshoot the target
    long speed = MOTOR_SPEED;
    long diff = actuatorPotValue - controllerPotValue;
    if (diff < 75) {
      speed = map(diff,1,75,50,MOTOR_SPEED);
    }

    if (DEBUG_MOTOR_SPEED) {
      Serial.print("retract motor speed: ");
      Serial.println(speed);
    }

    analogWrite(actuatorRPWM, speed);
    analogWrite(actuatorLPWM, 0);
  }

  // extend (not extended far enough)
  else if (actuatorPotValue < controllerPotValue) {

    if (DEBUG_CONTROLLER) {
      Serial.print("unmappedControlerPotValue: ");
      Serial.println(unmappedControlerPotValue);
    }

    if (DEBUG_ACTUATOR) {
      Serial.print("extending, actuatorPotValue: ");
      Serial.println(actuatorPotValue);
    }

    // make speed a function of how close we are to the target value
    // if we don't do this then the actuator will overshoot the target
    long speed = MOTOR_SPEED;
    long diff = controllerPotValue - actuatorPotValue;;
    if (diff < 75) {
      speed = map(diff,1,75,50,MOTOR_SPEED);
    }

    if (DEBUG_MOTOR_SPEED) {
      Serial.print("extend motor speed: ");
      Serial.println(speed);
    }

    analogWrite(actuatorRPWM, 0);
    analogWrite(actuatorLPWM, speed);
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
    //delay(10);
  }
}