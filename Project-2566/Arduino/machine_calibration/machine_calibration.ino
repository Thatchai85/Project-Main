#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>

// Define the stepper motor pin
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

// RX & TX & Address
const int RX_PIN = 12; // Define RX_PIN
const int TX_PIN = 13; // Define TX_PIN
const byte UART_ADDRESS = 0xe0; // Define Motor Address
SoftwareSerial mksServo42CSerial(RX_PIN, TX_PIN);

//Define Limitswtich Pin
const int limitX1 = 28;
const int motorspeed = 100;
const int ac_speed = 100;
int Laser = 34; // Define Laser pin
bool motorStopped = false; // Flag to track if the motor has been stopped

bool receivedInput = false; // Flag to indicate if input has been received

void setup() {
  Serial.begin(115200);
  pinMode(limitX1, INPUT);
  pinMode(Laser, OUTPUT);

  // Motor setting
  pinMode(EN_PIN_X1, OUTPUT);
  digitalWrite(EN_PIN_X1, LOW);
  pinMode(EN_PIN_X2, OUTPUT);
  digitalWrite(EN_PIN_X2, LOW);
  pinMode(EN_PIN_Y, OUTPUT);
  digitalWrite(EN_PIN_Y, LOW);  // Enable Stepper
  
  stepperX1.setMaxSpeed(motorspeed);
  stepperX1.setAcceleration(ac_speed);
  
  stepperX2.setMaxSpeed(motorspeed);
  stepperX2.setAcceleration(ac_speed);

  stepperY.setMaxSpeed(motorspeed);
  stepperY.setAcceleration(ac_speed);
}

void loop() {
  // Check if input has not been received yet
  digitalWrite(Laser, HIGH);
  
  if (!receivedInput && Serial.available() > 0) {
    // Read the entire line from Serial Monitor
    String inputString = Serial.readStringUntil('\n');

    // Perform an action based on the received input
    if (inputString.length() > 0) {
      int spaceIndex = inputString.indexOf(' ');
      if (spaceIndex != -1) {
        String inputXString = inputString.substring(0, spaceIndex);
        String inputYString = inputString.substring(spaceIndex + 1);

        // Parse the strings into integers
        int inputXSteps = inputXString.toInt();
        int inputYSteps = inputYString.toInt();

        Serial.print("Received values: ");
        Serial.print(inputXSteps);
        Serial.print(", ");
        Serial.println(inputYSteps);

      stepperX1.moveTo(-inputXSteps);
      stepperX2.moveTo(-inputXSteps);
      stepperY.moveTo(-inputYSteps);
      receivedInput = true;
      } else {
        Serial.println("Invalid input format. Please provide two values separated by a space.");
      }
    } else {
      Serial.println("Not activated.");
    }
  }

  // Move the stepper motor until it reaches the desired position
  stepperX1.run();
  stepperX2.run();
  stepperY.run();

  // Reset the receivedInput flag after all motors have finished moving
  if (receivedInput && stepperX1.distanceToGo() == 0 && stepperX2.distanceToGo() == 0 && stepperY.distanceToGo() == 0) {
  //if (receivedInput && stepperX1.distanceToGo() == 0 && stepperY.distanceToGo() == 0) {
  //if (receivedInput && stepperY.distanceToGo() == 0) {  
    receivedInput = false;
    //digitalWrite(Laser, HIGH);
  }
}
