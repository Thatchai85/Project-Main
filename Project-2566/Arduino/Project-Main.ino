#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Servo.h>
Servo servo;

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
#define EN_PIN_Z 34
#define STEP_PIN_Z 36
#define DIR_PIN_Z 38
AccelStepper stepperZ(1, STEP_PIN_Z, DIR_PIN_Z);

//Define Limitswtich Pin
//const int limitX1 = 26;
const int limitX2 = 26;
const int limitY1 = 28; 
const int limitZ = 30;
int relay = 22;
const int motorspeed = 200;
const int motorspeedz = 300;
const int ac_speed = 200;
bool motorStopped = false; // Flag to track if the motor has been stopped

const int BUFFER_SIZE = 256; // Adjust based on the expected package size

void setup() {
  Serial.begin(9600);
  
  pinMode(EN_PIN_X1, OUTPUT);
  digitalWrite(EN_PIN_X1, LOW);
  pinMode(EN_PIN_X2, OUTPUT);
  digitalWrite(EN_PIN_X2, LOW);
  
  pinMode(EN_PIN_Y, OUTPUT);
  digitalWrite(EN_PIN_Y, LOW);  // Enable Stepper

  pinMode(EN_PIN_Z, OUTPUT);
  digitalWrite(EN_PIN_Z, LOW);

  stepperX1.setMaxSpeed(motorspeed);
  stepperX1.setAcceleration(ac_speed);
  
  stepperX2.setMaxSpeed(motorspeed);
  stepperX2.setAcceleration(ac_speed);
  
  stepperY.setMaxSpeed(motorspeed);
  stepperY.setAcceleration(ac_speed);

  stepperZ.setMaxSpeed(motorspeedz);
  stepperZ.setAcceleration(ac_speed);
  
  servo.attach(24);
  servo.write(0);
  //pinMode(limitX1, INPUT);
  pinMode(limitX2, INPUT);
  pinMode(limitY1, INPUT);
  //pinMode(limitY2, INPUT);
  pinMode(limitZ, INPUT);
  pinMode(relay, OUTPUT);  
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
        int z = doc["data"]["z"];
        int time_value = doc["data"]["t"];
        moveRobot(x, y, z, time_value);
        //moveZDown(time_value);
        //imerse(time_value);
        //moveZUp();
      }
      else if (doc["command"] == "Homeposition") {
        HomepositionZ();
        HomepositionX();
        HomepositionY();
        resetmotor();
      }
      else if (doc["command"] == "Homestart") {
        int x = doc["data"]["x"];
        int y = doc["data"]["y"];
        int z = doc["data"]["z"];
        int time_value = doc["data"]["t"];
        //HomepositionZ();
        //HomepositionX();
        //HomepositionY();
        //resetmotor();
        moveRobot(x, y, z, time_value);
      }
      else if (doc["command"] == "settoready") {
        settoready();
      }
      else if (doc["command"] == "stop") {
        stopRobot();
      }
      else if (doc["command"] == "pause") {
        // Process the pause command
        //pauseRobot();
      }
      
    } 
  }
}

void resetmotor(){ 
  delay(3000);   
  digitalWrite(relay, 1);   // Close relay
  delay(1000); 
  digitalWrite(relay, 0);   // Open relay
  stepperX1.setCurrentPosition(0);
  stepperX2.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);
  stepperZ.setCurrentPosition(0);
  }
///////////////////////Move to each position////////////////////////////
void moveRobot(int x, int y, int z, int time_value) {
  delay(500);
  servo.write(142);  // set servo to lock rack
  delay(500);
  //Move stepper motorX1 motor X2
  stepperX1.setSpeed(motorspeed);
  stepperX2.setSpeed(motorspeed);
  stepperY.setSpeed(motorspeed);
  stepperX1.moveTo(-x);
  stepperX2.moveTo(-x);
  stepperY.moveTo(-y);
  while (stepperX1.distanceToGo() !=0 || stepperX2.distanceToGo() !=0 || stepperY.distanceToGo() !=0){
      stepperX1.run();
      stepperX2.run();
      stepperY.run();
    }
  delay(2000);
  Serial.println("Finish move x & y");
  if (x == 0 && y == 0 && time_value == 0){
    Serial.println("request");
  }
  else{
    Serial.println("moving down");
    //Move stepper Motor Z Up-down
    Serial.println("Time to dipping:");
    Serial.println(time_value);
    stepperZ.setSpeed(motorspeedz);
    stepperZ.moveTo(-2850); //3000
    while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
    Serial.println("imersing");
    delay(time_value * 1000);
  
    Serial.println("moving up");
    stepperZ.setSpeed(600);
    stepperZ.moveTo(-1500);
    while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
    delay(300);
    /////Shaking//////
    Serial.println("Shaking the rack!");
    delay(300);
    stepperZ.setSpeed(600);
    stepperZ.moveTo(-900);
    while (stepperZ.distanceToGo() !=0){
        stepperZ.run();
      }
    stepperZ.setSpeed(600);
    stepperZ.moveTo(-1500);
    while (stepperZ.distanceToGo() !=0){
        stepperZ.run();
      }
    stepperZ.setSpeed(600);
    stepperZ.moveTo(-900);
    while (stepperZ.distanceToGo() !=0){
        stepperZ.run();
      }
    stepperZ.setSpeed(600);
    stepperZ.moveTo(-1500);
    while (stepperZ.distanceToGo() !=0){
        stepperZ.run();
      }
    stepperZ.setSpeed(600);
    stepperZ.moveTo(-900);
    while (stepperZ.distanceToGo() !=0){
        stepperZ.run();
      }
    //////Request new position//////
    stepperZ.setSpeed(motorspeedz);
    stepperZ.moveTo(-500);
    while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
    Serial.println("request");
  }
}
void settoready(){
  stepperX1.setSpeed(motorspeed);
  stepperX2.setSpeed(motorspeed);
  stepperY.setSpeed(motorspeed);
  stepperZ.setSpeed(motorspeedz);
  stepperX1.moveTo(0);
  stepperX2.moveTo(0);
  stepperY.moveTo(0);
  stepperZ.moveTo(0);
  servo.write(0);
  while (stepperX1.distanceToGo() !=0 || stepperX2.distanceToGo() !=0 || stepperY.distanceToGo() !=0 || stepperZ.distanceToGo()!=0){
        stepperX1.run();
        stepperX2.run();
        stepperY.run();
        stepperZ.run();
    }
    Serial.println("Homeposition already");
  
  }

///////////////////Home Position X//////////////////////////
void HomepositionX(){
  Serial.println("Homing started for X axis");
  runmotorXforward();
}

void runmotorXforward(){
  stepperX1.setSpeed(motorspeed);
  stepperX2.setSpeed(motorspeed);
  stepperX1.moveTo(3000);
  stepperX2.moveTo(3000);
  Serial.println("Motor running forward");
  while (stepperX1.distanceToGo() != 0 && stepperX2.distanceToGo() != 0) {
    if (digitalRead(limitX2) == HIGH) {
      stepperX1.stop();
      stepperX2.stop();
      Serial.println("Limit switchX activated. Homing complete for X axis");
      delay(3000);
      Serial.println("Move back a little bit");
      stepperX1.setAcceleration(500);
      stepperX2.setAcceleration(500);
      stepperX1.setSpeed(-motorspeed);
      stepperX2.setSpeed(-motorspeed);
      stepperX1.move(-15);
      stepperX2.move(-20);
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
  stepperY.moveTo(2000);
  Serial.println("Motor Y running forward");
  while (stepperY.distanceToGo() != 0) {
    if (digitalRead(limitY1) == HIGH) {
      stepperY.stop();
      Serial.println("Limit switchY activated. Homing complete for Y axis");
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
  servo.write(0);
  //Serial.println("start");
  }
  
void runmotorZforward(){
  stepperZ.setSpeed(motorspeedz);
  stepperZ.moveTo(3800);
  Serial.println("Motor Z running forward");
  while (stepperZ.distanceToGo() != 0) {
    if (digitalRead(limitZ) == HIGH) {
      stepperZ.stop();
      Serial.println("Limit switchZ activated. Homing complete for Y axis");
      delay(3000);
      Serial.println("Move back a little bit");
      stepperZ.setAcceleration(500);
      stepperZ.setSpeed(-motorspeedz);
      stepperZ.move(-35); // Move back 4 steps
      stepperZ.runToPosition();
      Serial.println("Homing complete. Motor at home position.");
      break;
    }
    stepperZ.run();
  }
}
/////////////////////////////////////////////////////////////////

void stopRobot() {
  Serial.println("Paused");
  stepperX1.stop();
  stepperX2.stop();
  stepperY.stop();
  stepperZ.stop();
}

void moveZDown(int x, int y, int time_value){
  if (x == 0 && y ==0){
    stepperZ.setSpeed(motorspeedz);
    stepperZ.moveTo(-2000);
    while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
    Serial.println("imersing");
    stepperZ.stop();
    delay(5000);
  
    Serial.println("moving up");
    stepperZ.moveTo(500);
    while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
    Serial.println("request");
  }
  else{
    Serial.println("moving down");
    //Move stepper Motor Z Up-down
    Serial.println("Time to dipping:");
    Serial.println(time_value);
    stepperZ.setSpeed(motorspeedz);
    stepperZ.moveTo(-2000);
    while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
    delay(1000);
    //////////Shaking////////
    Shaking();
  }
}

void Shaking(){
  Serial.println("Shaking the rack!");
  delay(300);
  stepperZ.setSpeed(400);
  stepperZ.moveTo(-1000);
  while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
  stepperZ.setSpeed(400);
  stepperZ.moveTo(500);
  while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
  stepperZ.setSpeed(400);
  stepperZ.moveTo(-1000);
  while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
  stepperZ.setSpeed(400);
  stepperZ.moveTo(500);
  while (stepperZ.distanceToGo() !=0){
      stepperZ.run();
    }
}
/*
void imerse(int time_value){
  // Implement your imersing task
  Serial.println("imersing");
  delay(time_value * 1000);
}

void moveZUp(){
  Serial.println("moving up");
  stepperZ.moveTo(200);
  while (stepperZ.distanceToGo() !=0){
    stepperZ.run();
    }
  delay(1000);
  Serial.println("request");
}
*/

///////////////Servo//////////////////
void HomepositionServo(){
  servo.write(0);
  }