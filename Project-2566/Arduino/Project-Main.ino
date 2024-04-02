#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>

//Define the stepper motorX1
#define EN_PIN_X1 2
#define STEP_PIN_X1 3
#define DIR_PIN_X1 4
AccelStepper stepperX1(1,STEP_PIN_X1,DIR_PIN_X1);
//Define the stepper motorX2
#define EN_PIN_X2 5
#define STEP_PIN_X2 6
#define DIR_PIN_X2 7
AccelStepper stepperX2(1,STEP_PIN_X2,DIR_PIN_X2);

// Define the stepper motorY pin
#define EN_PIN_Y 8
#define STEP_PIN_Y 9
#define DIR_PIN_Y 10
AccelStepper stepperY(1, STEP_PIN_Y, DIR_PIN_Y); // 1 motor, STEP_PIN, DIR_PIN

//Define the stepper MotorZ pin
#define EN_PIN_Z 40
#define STEP_PIN_Z 42
#define DIR_PIN_Z 44
AccelStepper stepperZ(1, STEP_PIN_Z, DIR_PIN_Z);

//Define Limitswtich Pin
const int limitX1 = 28;
const int limitX2 = 30;
const int limitY1 = 32; 
const int limitZ = 34;

const int resetboard = 38;
int Laser = 36; // Define Laser pin

const int motorspeed = 200;
const int ac_speed = 200;
bool motorStopped = false; // Flag to track if the motor has been stopped

const int BUFFER_SIZE = 256; // Adjust based on the expected package size

void setup() {
  Serial.begin(115200);
  
  pinMode(EN_PIN_X1, OUTPUT);
  digitalWrite(EN_PIN_X1, LOW);
  pinMode(EN_PIN_X2, OUTPUT);
  digitalWrite(EN_PIN_X2, LOW);
  
  pinMode(EN_PIN_Y, OUTPUT);
  digitalWrite(EN_PIN_Y, LOW);

  pinMode(EN_PIN_Z, OUTPUT);
  digitalWrite(EN_PIN_Z, LOW);

  stepperX1.setMaxSpeed(motorspeed);
  stepperX1.setAcceleration(ac_speed);
  
  stepperX2.setMaxSpeed(motorspeed);
  stepperX2.setAcceleration(ac_speed);
  
  stepperY.setMaxSpeed(motorspeed);
  stepperY.setAcceleration(ac_speed);

  stepperZ.setMaxSpeed(motorspeed);
  stepperZ.setAcceleration(ac_speed);

  pinMode(limitX1, INPUT);
  pinMode(limitX2, INPUT);
  pinMode(limitY1, INPUT);
  //pinMode(limitY2, INPUT);
  pinMode(Laser, OUTPUT);
  digitalWrite(resetboard, HIGH);
  pinMode(resetboard, OUTPUT);
  
}

void loop() {
  if (Serial.available() > 0) {
    //String command = Serial.readStringUntil('\n');
    //processCommand(command);
    
    // Read the incoming data into a buffer
    char buffer[BUFFER_SIZE];
    int bytesRead = Serial.readBytesUntil('\n', buffer, BUFFER_SIZE);
    buffer[bytesRead] = '\0'; // Null-terminate the string

    // Parse the JSON package
    StaticJsonDocument<BUFFER_SIZE> doc;
    DeserializationError error = deserializeJson(doc, buffer);
    
    // Check for parsing errors
    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
    }
    else {
      // Check for the presence of different command types
      if (doc["command"] == "position") {
        // Extract values for position command
        int x = doc["data"]["x"];
        int y = doc["data"]["y"];
        int time_value = doc["data"]["t"];
        // Process the received values
        moveRobot(x, y, time_value);
        moveZDown(time_value);
        imerse(time_value);
        moveZUp();
      }
      else if (doc["command"] == "Homeposition") {
        HomepositionX();
        HomepositionY();
        HomepositionZ();
        delay(2000);
        resetBoard();
        Serial.println("Board reset complete");
        //laseron();
      }
      else if (doc["command"] == "HomepositionY") {
        HomepositionY();
      }
      else if (doc["command"] == "stop") {
        // Process the stop command
        stopRobot();
      }
      else if (doc["command"] == "pause") {
        // Process the pause command
        pauseRobot();
      }
      else if (doc["command"] == "laseron"){
        laseron(); // ถ้าค่า val เท่ากับ 1 สั่งให้ Laser ติด
      }
    } 
  }
}

void laseron(){
  digitalWrite(Laser, HIGH);
  }
  
void resetBoard() {
  delay(500);
  digitalWrite(resetboard, LOW); // Assuming resetPin is the pin connected to the reset line
  delay(500);
  Serial.println("Resetting the board");
}


///////////////////////Move to each position////////////////////////////
void moveRobot(int x, int y, int time_value) {
  Serial.println("request");
  delay(1000);
  //Move stepper motorX1 motor X2
  stepperX1.setSpeed(motorspeed);
  stepperX2.setSpeed(motorspeed);
  stepperX1.moveTo(-x);
  stepperX2.moveTo(-x);
  while (stepperX1.distanceToGo() !=0 || stepperX2.distanceToGo() !=0){
    stepperX1.run();
    stepperX2.run();
    }
  delay(2000); 
  //Move stepper motorYs
  stepperY.setSpeed(motorspeed);
  stepperY.moveTo(-y);
  while (stepperY.distanceToGo() !=0){
    stepperY.run();
    }
  delay(1000);
}

///////////////////Home Position X//////////////////////////
void HomepositionX(){
  Serial.println("Homing started for X axis");
  runmotorXforward();
  //runmotorX1forward();
  //runmotorX2forward();
}

void runmotorXforward(){
  stepperX1.setSpeed(motorspeed);
  stepperX2.setSpeed(motorspeed);
  stepperX1.moveTo(3000);
  stepperX2.moveTo(3000);
  Serial.println("Motor running forward");
  while (stepperX1.distanceToGo() != 0 && stepperX2.distanceToGo() != 0) {
    if (digitalRead(limitX1) == HIGH || digitalRead(limitX2) == HIGH) {
      stepperX1.stop();
      stepperX2.stop();
      Serial.println("Limit switchY1 activated. Homing complete for Y1 axis");
      delay(3000);
      Serial.println("Move back a little bit");
      stepperX1.setAcceleration(500);
      stepperX2.setAcceleration(500);
      stepperX1.setSpeed(-motorspeed);
      stepperX2.setSpeed(-motorspeed);
      stepperX1.move(-15);
      stepperX2.move(-15);
      stepperX1.runToPosition();
      stepperX2.runToPosition();
      Serial.println("Homing complete. Motor at home position.");
      break;
    }
    stepperX1.run();
    stepperX2.run();
  }
}
  
//////////////////////// Homing in Y axis ///////////////////////////
void HomepositionY() {
  Serial.println("Homing started for Y axis");
  runmotorYforward();
  }
  
void runmotorYforward() {
  //stepperY.setAcceleration(500);
  stepperY.setSpeed(motorspeed);
  stepperY.moveTo(1800);
  Serial.println("Motor running forward");
  while (stepperY.distanceToGo() != 0) {
    if (digitalRead(limitY1) == HIGH) {
      stepperY.stop();
      Serial.println("Limit switchY1 activated. Homing complete for Y axis");
      delay(3000);
      Serial.println("Move back a little bit");
      stepperY.setAcceleration(500);
      stepperY.setSpeed(-motorspeed);
      stepperY.move(-15); // Move back 4 steps
      stepperY.runToPosition();

      Serial.println("Homing complete. Motor at home position.");
      break;
    }
    stepperY.run();
  }
}

////////////////////////////////////////////////////////////////
void HomepositionZ(){
  Serial.println("Motor running forward");
  runmotorZforward();
  }
  
void runmotorZforward(){
  stepperZ.setSpeed(motorspeed);
  stepperZ.moveTo(3800);
  Serial.println("Motor Z running forward");
  while (stepperZ.distanceToGo() != 0) {
    if (digitalRead(limitZ) == HIGH) {
      stepperZ.stop();
      Serial.println("Limit switchZ activated. Homing complete for Y axis");
      delay(3000);
      Serial.println("Move back a little bit");
      stepperZ.setAcceleration(500);
      stepperZ.setSpeed(-motorspeed);
      stepperZ.move(-30); // Move back 4 steps
      stepperZ.runToPosition();

      Serial.println("Homing complete. Motor at home position.");
      break;
    }
    stepperZ.run();
  }
}
/////////////////////////////////////////////////////////////////
void moveZDown(int x, int y, int time_value){
  if (x == 0 && y ==0){
    Serial.println("request");
  }
  else{
    Serial.println("moving down");
    //Move stepper Motor Z Up-down
    Serial.println("Time to dipping:");
    Serial.println(time_value);
    stepperZ.setSpeed(motorspeed);
    stepperZ.moveTo(-3700);
    while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
    delay(1000);
  }
}
void imerse(int time_value){
  // Implement your imersing task
  Serial.println("imersing");
  delay(time_value * 1000);
}

void moveZUp(){
  Serial.println("moving up");
  stepperZ.moveTo(-2200);
  while (stepperZ.distanceToGo() !=0){
    stepperZ.run();
    }
  delay(1000);
  Serial.println("request");
}


void stopRobot() {
  Serial.println("Robot stopped");
}

void pauseRobot() {
  Serial.println("Paused");
  stepperX1.stop();
  stepperX2.stop();
  stepperY.stop();
}

void ShakeRack(){
  // Implement shaking method to spilt chemical from the rack
  Serial.println("Shaking the rack!");
}

///////////////Function//////////////////