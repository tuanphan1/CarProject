#include <Wire.h>
#include <VL6180X.h>
#define RANGE 1
//*****Laser Sensors*****//
/* List of adresses for each sensor - after reset the address can be configured */
#define address0 0x20
#define address1 0x22
#define address2 0x24
/* These Arduino pins must be wired to the IO0 pin of VL6180x */
int enablePin0 = 2; //Left
int enablePin1 = 3; //Right
int enablePin2 = 4; //Front
/* Create a new instance for each sensor */
VL6180X sensor0;
VL6180X sensor1;
VL6180X sensor2;
//*****DC Motor*****//
int pinI1=8;//define I1 interface
int pinI2=11;//define I2 interface 
int speedpinLeft = 10;  //enable motor LEFt
int pinI3=12;//define I3 interface 
int pinI4=13;//define I4 interface 
int speedpinRight = 9;//enable motor RIGHt
int speedRight = 160; // speed of right motor
int speedLeft = 165;  // speed of left motor
int turnSpeed = 200;
int turnDelay = 500; // how long to turn for
int adjustDiff = 20;
// other variables
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  
  //*****Lasor Sensor*****//
  // Reset all connected sensors
  pinMode(enablePin0,OUTPUT);
  pinMode(enablePin1,OUTPUT);
  pinMode(enablePin2,OUTPUT);
  
  digitalWrite(enablePin0, LOW);
  digitalWrite(enablePin1, LOW);
  digitalWrite(enablePin2, LOW);
  
  delay(1000);
  
  // Sensor0 
  Serial.println("Start Sensor Left");
  digitalWrite(enablePin0, HIGH);
  delay(500);
  sensor0.init();
  sensor0.configureDefault();
  sensor0.setAddress(address0);
  Serial.println(sensor0.readReg(0x212),HEX); // read I2C address
  sensor0.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor0.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
  sensor0.setTimeout(500);
  sensor0.stopContinuous();
  sensor0.setScaling(RANGE); // configure range or precision 1, 2 oder 3 mm
  delay(300);
  sensor0.startInterleavedContinuous(100);
  delay(100);
  
  // Sensor1 
  Serial.println("Start Sensor Right");
  digitalWrite(enablePin1, HIGH);
  delay(500);
  sensor1.init();
  sensor1.configureDefault();
  sensor1.setAddress(address1);
  Serial.println(sensor1.readReg(0x212),HEX);
  sensor1.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor1.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
  sensor1.setTimeout(500);
  sensor1.stopContinuous();
  sensor1.setScaling(RANGE);
  delay(300);
  sensor1.startInterleavedContinuous(100);
  delay(100);
  
  // sensor 2 left
  Serial.println("Start Sensor Front");
  digitalWrite(enablePin2, HIGH);
  delay(50);
  sensor2.init();
  sensor2.configureDefault();
  sensor2.setAddress(address2);
  Serial.println(sensor2.readReg(0x212),HEX);
  sensor2.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor2.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
  sensor2.setTimeout(500);
  sensor2.stopContinuous();
  sensor2.setScaling(RANGE);
  delay(300);
  sensor2.startInterleavedContinuous(100);
  delay(1000);
  
  //*****DC Motor*****//
  pinMode(pinI1,OUTPUT);
  pinMode(pinI2,OUTPUT);
  pinMode(speedpinLeft,OUTPUT);
  pinMode(pinI3,OUTPUT);
  pinMode(pinI4,OUTPUT);
  pinMode(speedpinRight,OUTPUT);
  Serial.println("Sensors ready! Reading sensors..!");
  delay(2000);

//  // Solves starting at the corner
//  if(frontCurrent() == 255){
     forward();
     delay(1000);
//  }
}
void loop()
{
  Serial.println();
  // Abstand in mm
  Serial.print("\tLeft: ");
  Serial.print(sensor0.readRangeContinuousMillimeters());
  if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  Serial.print("\tRight: ");
  Serial.print(sensor1.readRangeContinuousMillimeters());
  if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  Serial.print("\tFront: ");
  Serial.print(sensor2.readRangeContinuousMillimeters());
  if (sensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  
  runCar();
}

void runCar()
{
  int state = 0;
  int sensorLeft = sensor0.readRangeContinuousMillimeters();
  int sensorRight = sensor1.readRangeContinuousMillimeters();
  int sensorFront = sensor2.readRangeContinuousMillimeters();

  state = getState(sensorLeft, sensorRight, sensorFront);

  switch (state)
  {
    case 1:
      forward();
      break;
    case 2:
      leftT();
      break;
    case 3:
      rightT();
      break;
    case 4:
      //adjust();
      break;
    case 5:
      stopCar();
      break;
    case 6:
      backward();
      delay(350);
      break;
    case 7:
      adjust( speedLeft, (speedRight - adjustDiff));  // adjust slight to right
      break;
    case 8:
      adjust( (speedLeft - adjustDiff), speedRight);  // adjust slight to left
      break;
    case 9:
      left90();
      break;
    case 10:
      right90();
      break;
    case 11:
      backwardLeft90();
      break;
  }   
  
}

int getState(int left, int right, int front){
  
    if((left < 50) || (right < 50) || (front < 30)){ // Any sensor too close to wall
      if(left < 50){
        backward();
        delay(500);
        rightT();
        delay(100);
      } 
      else if(right < 50){
        backward();
        delay(500);
        leftT();
        delay(100);
      } 
      else if(front < 30){
        backward();
        delay(500);
      }
      return 5; // stop car
    }
        
    // If there's an opening pull foward to middle then stop the car
    if((left == 255) || (right == 255)){
      forward();
      delay(750);
      stopCar();

      // Intersection detected
      // calculate current openings and proceed
                  
      // 4 way intersection
      if ((leftCurrent() == 255) && (rightCurrent() == 255) && (frontCurrent() == 255)){
        if(front < 30){ // Front sensor too close to wall
          return 6; // backward
        }
        forward();
        delay(1500);
        return 0; 
      }
      
      // 3 way intersection left and right openings, front blocked
      else if((leftCurrent() == 255) && (rightCurrent() == 255) && (frontCurrent() < 140)){
        return 10; // Right 90 degrees   
      }
      
      // 3 way intersection left and front opening, right blocked
      else if((leftCurrent() == 255) && (rightCurrent() < 255) && (frontCurrent() == 255)){
        return 9; // Left 90 degrees
      }

      //*****// Extra credit section //*****//
      // 3 way intersection right and front opening, left blocked
      else if((leftCurrent() < 255) && (rightCurrent() == 255) && (frontCurrent() == 255)){
        return 10; // Right 90 degrees
      }

      // 2 way intersection left openning only, right and front blocked
      else if((leftCurrent() == 255) && (rightCurrent() < 255) && (frontCurrent() < 255)){
        return 9; // Left 90 degrees
      }

      // 2 way intersection right openning only, left and front blocked
      else if((leftCurrent() < 255) && (rightCurrent() == 255) && (frontCurrent() < 255)){
        return 10; // Right 90 degrees
      }


    }

    //*****// Special CASE //*****//
    // Dead end
    if ((leftCurrent() < 200) && (rightCurrent() < 200) && (frontCurrent() < 40)){
      while((leftCurrent() < 255) && (rightCurrent() < 255)){
        backward();
      }
      delay(1000);
      stopCar();
      delay(500);
      return 11;
    }
    
    // Left sensor closer to wall
    if((left - right) < 0){
      if(front < 30){ // Front sensor too close to wall
          return 6; // backward
        }
//      else if(left < 40){
//        return 6; // backward
//      }
      return 7; // slightly adjust to right
    }
    
    // Right sensor closer to wall
    if((left - right) > 0){
      if(front < 30){ // Front sensor too close to wall
          return 6; // backward
        }

      return 8; // slightly adjust to left
    }
    return 1; //DEFAULT return forward();
}

int leftCurrent(){
  return sensor0.readRangeContinuousMillimeters();
}
int rightCurrent(){
  return sensor1.readRangeContinuousMillimeters();
}
int frontCurrent(){
  return sensor2.readRangeContinuousMillimeters();
}

// To change speed, analogWrite(speedpin<Side>, <Speed>);
// Side: Left/Right Speed:1-255
void backward(){ 
     analogWrite(speedpinLeft, speedLeft);
     analogWrite(speedpinRight,speedRight);
     digitalWrite(pinI4,HIGH); //turn DC Motor B move clockwise
     digitalWrite(pinI3,LOW);
     digitalWrite(pinI2,LOW); //turn DC Motor A move anticlockwise
     digitalWrite(pinI1,HIGH);
}
void forward(){
     analogWrite(speedpinLeft, speedLeft);
     analogWrite(speedpinRight,speedRight);
     digitalWrite(pinI4,LOW); //turn DC Motor B move anticlockwise
     digitalWrite(pinI3,HIGH);
     digitalWrite(pinI2,HIGH); //turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
}

// *****left wheel slower***** need to increase turn speed of left wheel
void right90(){  
     analogWrite(speedpinLeft, (turnSpeed + 15));
     analogWrite(speedpinRight,turnSpeed);
     digitalWrite(pinI4,LOW);//turn DC Motor B move anticlockwise
     digitalWrite(pinI3,HIGH);
     digitalWrite(pinI2,LOW);//turn DC Motor A move clockwise
     digitalWrite(pinI1,HIGH);
     delay(turnDelay); // Change delay depending on battery life/ turn speed

     // Might need this
     // Edge case for first right turn
     forward();
     delay(1000);
}
void left90(){  
     analogWrite(speedpinLeft, turnSpeed);
     analogWrite(speedpinRight,turnSpeed);
     digitalWrite(pinI4,HIGH);//turn DC Motor B move clockwise
     digitalWrite(pinI3,LOW);
     digitalWrite(pinI2,HIGH);//turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
     delay(turnDelay); // Change delay depending on battery life/ turn speed
     
     forward();
     delay(1000);
}
//*****// Special Case for dead end //*****//
void backwardLeft90(){
     analogWrite(speedpinLeft, turnSpeed);
     analogWrite(speedpinRight,turnSpeed);
     digitalWrite(pinI4,HIGH);//turn DC Motor B move clockwise
     digitalWrite(pinI3,LOW);
     digitalWrite(pinI2,HIGH);//turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
     delay(turnDelay); // Change delay depending on battery life/ turn speed

     forward();
     delay(1000);
}
void leftT(){
     analogWrite(speedpinLeft, turnSpeed);
     analogWrite(speedpinRight,turnSpeed);
     digitalWrite(pinI4,HIGH);//turn DC Motor B move clockwise
     digitalWrite(pinI3,LOW);
     digitalWrite(pinI2,HIGH);//turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
}
void rightT(){
     analogWrite(speedpinLeft, (turnSpeed + 15));
     analogWrite(speedpinRight,turnSpeed);
     digitalWrite(pinI4,LOW);//turn DC Motor B move anticlockwise
     digitalWrite(pinI3,HIGH);
     digitalWrite(pinI2,LOW);//turn DC Motor A move clockwise
     digitalWrite(pinI1,HIGH);
}
void adjust(int newSpeedLeft, int newSpeedRight){
     analogWrite(speedpinLeft, newSpeedLeft);
     analogWrite(speedpinRight,newSpeedRight);
     digitalWrite(pinI4,LOW); //turn DC Motor B move anticlockwise
     digitalWrite(pinI3,HIGH);
     digitalWrite(pinI2,HIGH); //turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
}
void stopCar(){
     digitalWrite(speedpinLeft, LOW);// Unenble the pin, to stop the motor. this should be done to avid damaging the motor. 
     digitalWrite(speedpinRight, LOW);
     delay(1000);
}
