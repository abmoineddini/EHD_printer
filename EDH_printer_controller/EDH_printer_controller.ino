/* Arduino Mega ElectroHydrodynamics Machine
 * by: Amirbahador Moineddini
 * date: August 08th, 2022
 * V2 version of the Arduino EHD Printer

/* Pin Setup
 * X-Axis stepper motor driver board:
 * - STEP  - pin 31
 * - DIR   - pin 33
 * - MS1   - pin 29
 * - MS2   - pin 27
 * - MS3   - pin 25
 * - En    - pin 23
 * 
 * Y-Axis stepper motor driver board:
 * - STEP - pin 43
 * - DIR  - pin 45
 * - MS1  - pin 41
 * - MS2  - pin 39
 * - MS3  - pin 37
 * - EN   - pin 35 

// MS1    MS2     MS3     Res
//  0      0       0       1
//  1      0       0      1/2
//  0      1       0      1/4
//  1      1       0      1/8
//  1      1       1      1/16
// Enable low => motors enabled
// xDir = HIGH => +X   xDir = LOW => -X
// yDir = HIGH => -Y   xDir = LOW => +Y
*/

// Declaring Controller Pins
const int xStep = 6;
const int xDir  = 7;
const int xMS3  = 3;
const int xMS2  = 4;
const int xMS1  = 5;

const int yStep = 11;
const int yDir  = 12;
const int yMS3  = 10;
const int yMS2  = 9;
const int yMS1  = 8;

const int enPin   = 2;

// Declaring microSwitches pins
const int xHome = 23;
const int yHome = 27;
int xVal;
int yVal;
// val = 1 => away from switch
// Val = 0 => Stage at home

// Counter and Homer
float xPos  = 0;
float yPos  = 0;
int Homer = 0;
int xStepSize = 0;
int yStepSize = 0;

int xStepsVal[]= {} // Array of x-direction waypoints obtained from the MATLAB Code
int yStepsVal[]= {} // Array of y-direction waypoints obtained from the MATLAB Code
int sz = 0;
int noXSteps = 0;
int noYSteps = 0;
int sumX = 0;
int sumY = 0;
int printStat = 0;

void hommingSequence(int xSpd, int ySpd);
void rotateMotors(int& noXSteps, int& noYSteps, int xSpd, int ySpd);
void StepSetx(int spd);
void StepSety(int spd);

void setup() {
  //Open serial communications
  Serial.begin(2000000);
  Serial.println("Starting Up....");
  
  //Define stepper pins as digital output pins
  pinMode(xStep,OUTPUT);
  pinMode(xDir,OUTPUT);
  pinMode(xMS1,OUTPUT);
  pinMode(xMS2,OUTPUT);
  pinMode(xMS3,OUTPUT);
  
  pinMode(yStep,OUTPUT);
  pinMode(yDir,OUTPUT);
  pinMode(yMS1,OUTPUT);

  pinMode(yMS2,OUTPUT);
  pinMode(yMS3,OUTPUT);
	
  pinMode(enPin,OUTPUT);
  
  // Setting input microswitches pins
  pinMode(xHome, INPUT);
  pinMode(yHome, INPUT);

  //Set microstepping mode for stepper driver boards.  Using 1.8 deg motor angle (200 steps/rev) NEMA 17 motors (12V)

  //X-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
  digitalWrite(xMS1,LOW);
  digitalWrite(xMS2,LOW);
  digitalWrite(xMS3,LOW);
  
  //Y-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
  digitalWrite(yMS1,LOW);
  digitalWrite(yMS2,LOW);
  digitalWrite(yMS3,LOW);
  
  delay(5000);
}
void loop() {
  //Homing Sequence
  if (Homer == 0){
    StepSetx(1);
    StepSety(1);
    hommingSequence(700, 700);
    sz = sizeof(xStepsVal);
    Serial.print("Total number of steps = ");
    Serial.println(sz);
    for(int i =0; i<=sz/16; i++){
      sumX = sumX + xStepsVal[i];
      sumY = sumY + yStepsVal[i];
    }
    Serial.print("Total Travel = ");
    Serial.print(sumX);
    Serial.print(" y= ");
    Serial.println(sumY);
    if (sumX<0) //Set X-Axis rotation +X
    {
      digitalWrite(xDir,HIGH); 
    } 
    else{
      digitalWrite(xDir,LOW);
    }
    for (int i=0; i<=sumX; i++){
      digitalWrite(xStep,HIGH);
      delayMicroseconds(300);
      digitalWrite(xStep,LOW);
      delayMicroseconds(300);
    }
    if (sumY<0) //Set X-Axis rotation +X
    {
      digitalWrite(yDir,LOW); 
    } 
    else{
      digitalWrite(yDir,HIGH);
    }
    for (int i=0; i<=sumY; i++){
        digitalWrite(yStep,HIGH);
        delayMicroseconds(700);
        digitalWrite(yStep,LOW);
        delayMicroseconds(700);
    }
    Homer = 1;
    Serial.println("Calibration Done!");
  }
  else{ 
    if (printStat == 0){  
      Serial.print(Homer);
      Serial.print("  ");
      Serial.print("x = ");
      Serial.print(xVal);
      Serial.print("  ");
      Serial.print("y = ");
      Serial.println(yVal);
      Serial.println("Platformed is homed");
      digitalWrite(enPin,LOW);

      for (int i=0; i<=sz/14; i++){
        StepSetx(1);
        StepSety(1);
        noXSteps = xStepsVal[i];
        noYSteps = yStepsVal[i];
        rotateMotors(noXSteps, noYSteps, 700, 700);
      }
      printStat = 1;
    }
    else {
      digitalWrite(enPin, HIGH);
      Serial.println("Print Done");
    }
  }
}
void hommingSequence(int xSpd, int ySpd){ 
  int xHomed = 0;
  int yHomed = 0;
  
  //Enable motor controllers
  digitalWrite(enPin, LOW);
  
  //Enable motor controllers
  digitalWrite(xDir,LOW);
  digitalWrite(yDir,LOW);
  while (xHomed == 0){
    xVal = digitalRead(xHome);
    
    if (xVal == 1){
      xHomed ++;
      digitalWrite(xDir,HIGH);
      for (int i=0; i<=5000; i++){
        digitalWrite(xStep,HIGH);
        delayMicroseconds(xSpd);
        digitalWrite(xStep,LOW);
        delayMicroseconds(xSpd);
      }
      xPos = 0;
      Serial.print("x-axis Homed");
    }
    else {
      digitalWrite(xStep,HIGH);
      delayMicroseconds(xSpd);
      digitalWrite(xStep,LOW);
      delayMicroseconds(xSpd);
    }
   }
   delay(1000);
  while (yHomed == 0){
    yVal = digitalRead(yHome);
    
    if (yVal == 1){
      yHomed ++;
      digitalWrite(yDir,HIGH);
      for (int i=0; i<=5000; i++){
        digitalWrite(yStep,HIGH);
        delayMicroseconds(ySpd);
        digitalWrite(yStep,LOW);
        delayMicroseconds(ySpd);
      }



      yPos = 0;
      Serial.print("y-axis Homed");
    }
    else {
      digitalWrite(yStep,HIGH);
      delayMicroseconds(ySpd);
      digitalWrite(yStep,LOW);
      delayMicroseconds(ySpd);
    }
   }
   Homer = 1;
   Serial.println("Platformed is Homed");
   delay(5000);
}

void StepSetx(int spd){
  switch (spd) {
  case 1:
    // Full step
    //X-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(xMS1,LOW);
    digitalWrite(xMS2,LOW);
    digitalWrite(xMS3,LOW);
    break;
  case 2:
    // 1/2 step
    //X-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(xMS1,HIGH);
    digitalWrite(xMS2,LOW);
    digitalWrite(xMS3,LOW);
    break;
  case 3:
    // 1/4 step
    //X-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(xMS1,LOW);
    digitalWrite(xMS2,HIGH);
    digitalWrite(xMS3,LOW);
    break;
  case 4:
    // 1/8 step
    //X-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(xMS1,HIGH);
    digitalWrite(xMS2,HIGH);
    digitalWrite(xMS3,LOW);

    break;
  case 5:
    // 1/2 step
    //X-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(xMS1,HIGH);
    digitalWrite(xMS2,HIGH);
    digitalWrite(xMS3,HIGH);
    break;
    default:
    // 1/2 step
    //X-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(xMS1,HIGH);
    digitalWrite(xMS2,HIGH);
    digitalWrite(xMS3,HIGH);
    break;
} 
}

void StepSety(int spd){
  switch (spd) {
  case 1:
    // Full step 
    //Y-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(yMS1,LOW);
    digitalWrite(yMS2,LOW);
    digitalWrite(yMS3,LOW);
    break;
  case 2:
    // 1/2 step 
    //Y-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(yMS1,HIGH);
    digitalWrite(yMS2,LOW);
    digitalWrite(yMS3,LOW);
    break;
  case 3:
    // 1/4 step
    //Y-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(yMS1,LOW);
    digitalWrite(yMS2,HIGH);
    digitalWrite(yMS3,LOW);
    break;
  case 4:
    // 1/8 step

    //Y-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(yMS1,HIGH);
    digitalWrite(yMS2,HIGH);
    digitalWrite(yMS3,LOW);
    break;
  case 5:
    // 1/2 step   
    //Y-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(yMS1,HIGH);
    digitalWrite(yMS2,HIGH);
    digitalWrite(yMS3,HIGH);
    break;
    default:
    // 1/2 step
    //Y-Axis motor: no micro stepping (MS1 Low, MS2 Low) = 1/16 deg/step (200 steps/rev)
    digitalWrite(yMS1,HIGH);
    digitalWrite(yMS2,HIGH);
    digitalWrite(yMS3,HIGH);
    break;
} 
}
void rotateMotors(int& noXSteps, int& noYSteps, int xSpd, int ySpd){
  //Initialize while loop counter
  int totalSteps=0;
  int stepDelay=10;
  float xIncrement=0;
  float yIncrement=0;
  switch (xStepSize) {
  case 1:
    // Full Steps
    xIncrement = (0.5/200);
    break;
  case 2:
    // 1/2 Steps
    xIncrement = (0.5/200)/2;
    break;
  case 3:
    // 1/4 Steps
    xIncrement = (0.5/200)/4;
    break;
  case 4:
    // 1/8 Steps
    xIncrement = (0.5/200)/8;
    break;
  case 5:

    // 1/16 Steps
    xIncrement = (0.5/200)/16;
    break;
  default:
    // 1/16 Steps
    xIncrement = (0.5/200)/16;
    break;
  }
  switch (yStepSize) {
  case 1:
    // Full Steps
    yIncrement = (0.5/200);
    break;
  case 2:
    // 1/2 Steps
    yIncrement = (0.5/200)/2;
    break;
  case 3:
    // 1/4 Steps
    yIncrement = (0.5/200)/4;
    break;
  case 4:
    // 1/8 Steps
    yIncrement = (0.5/200)/8;
    break;
  case 5:
    // 1/16 Steps
    yIncrement = (0.5/200)/16;
    break;
  default:
    // 1/16 Steps
    yIncrement = (0.5/200)/16;
    break;
  }
    
  //Set X-Axis motor rotation direction based on read value
  if (noXSteps<0) //Set X-Axis rotation +X
  {
    digitalWrite(xDir,LOW); 
  }
  else //Set X-Axis rotation -X
  {
    digitalWrite(xDir,HIGH);
    xIncrement = -xIncrement;
  }

  //Set Y-Axis motor rotation direction based on read value
  if (noYSteps<0) //Set Y-Axis rotation to CCW
  {
    digitalWrite(yDir,HIGH);
    yIncrement = -yIncrement;
  }
  else
  {
    digitalWrite(yDir,LOW);
  }

  //Calculate total number of steps for while loop indexing
  totalSteps=(abs(noXSteps)+abs(noYSteps))/16;
  
  //Get absolute value of steps
  noXSteps=abs(noXSteps)/16;
  noYSteps=abs(noYSteps)/16;
  
  //Move motors appropriate number of steps
  while (totalSteps>0){
    if (noXSteps>0) //Move X-Axis 
    {
      //Move X-Axis one step
      digitalWrite(xStep, LOW); //LOW to HIGH changes creates the "Rising Edge" so that the EasyDriver knows when to step.
      delayMicroseconds(xSpd);
      digitalWrite(xStep, HIGH); 
      delayMicroseconds(xSpd);
      noXSteps=noXSteps-1; //Decrement remaining number of X-Axis steps
      totalSteps=totalSteps-1; //Decrement remaining number of total steps
      xPos += xIncrement;
    }
  
    if (noYSteps>0) //Move Y-Axis 
    {
      //Move Y-Axis one step
      digitalWrite(yStep, LOW); //LOW to HIGH changes creates the "Rising Edge" so that the EasyDriver knows when to step.
      delayMicroseconds(ySpd);
      digitalWrite(yStep, HIGH); 
      delayMicroseconds(ySpd);
      yPos += yIncrement;

      noYSteps=noYSteps-1; //Decrement remaining number of Y-Axis steps
      totalSteps=totalSteps-1; //Decrement remaining number of total steps
    }
  }
}







