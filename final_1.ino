#include <Servo.h>
#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(2, 4);
SoftwareSerial ESP32Serial(0,1);
#define trigPin 7
#define echoPin 8  
#define servoPin 10  
#define pipeServoPin 11  
#define IN1 3  
#define IN2 5  
#define IN3 6  
#define IN4 9  
#define flameNorth A2

#define flameEast  A3
#define flameWest  A5
#define MQ2_PIN A0
#define pumpPin 12

Servo servo_motor;


int baseline = 0;  
int threshold;  
bool smokeDetected = false;
bool fireDetected = false;
bool manualMode = false;

unsigned long lastBTUpdate = 0;  
const unsigned long BTUpdateInterval = 1000;  

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(flameNorth, INPUT);
  pinMode(flameEast, INPUT);
  pinMode(flameWest, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  servo_motor.attach(servoPin);
 

  // Calibrate gas sensor
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(MQ2_PIN);
    delay(200);
  }
  baseline = sum / 10;  
  threshold = baseline + 10;
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (BTSerial.available()) {
    char command = BTSerial.read();
    if (command == 'M') {
      moveStop();
      manualMode = true;
      BTSerial.println("Manual Mode Activated");
    } else if (command == 'A') {
      manualMode = false;
      BTSerial.println("Autonomous Mode Activated");
    }
    if (manualMode) {
      controlCar(command);
    }
  } else if (!BTSerial.available() && !manualMode) {
    autonomousMode(currentMillis);
  }
  
  checkFireAndGas(currentMillis);
}

void autonomousMode(unsigned long currentMillis) {
  int distance = readPing();     

  if (!fireDetected) {
    activateExtinguisher(false);
    if (distance <= 30) {  
      avoidObstacle();
    } else {
      moveForward();
    }
  }
}

void checkFireAndGas(unsigned long currentMillis) {
  int detectedFlame = checkFlame();
  bool detectedSmoke = checkGas();

  if (detectedFlame && currentMillis - lastBTUpdate >= BTUpdateInterval) {
    String flameDirection;
    switch (detectedFlame) {
      case 1: flameDirection = "NORTH"; break;
      
      case 3: flameDirection = "EAST"; break;
      case 4: flameDirection = "WEST"; break;
    }

    Serial.print("🔥 Fire Detected at "); Serial.println(flameDirection);
    BTSerial.print("🔥 Fire Detected at "); BTSerial.println(flameDirection);
    ESP32Serial.println("F");
    lastBTUpdate = currentMillis;
  }

  if (detectedSmoke && currentMillis - lastBTUpdate >= BTUpdateInterval) {
    Serial.println("⚠ Smoke Detected!");
    BTSerial.println("⚠ Smoke Detected!");
    ESP32Serial.println("S"); 
    lastBTUpdate = currentMillis;
  }

  if (!manualMode && detectedFlame) { 
    int distance = readPing();
    
    if (distance > 15) { 
      Serial.println("🔥 Fire Detected! Moving towards fire...");
      moveForward();
    } else {
      Serial.println("🔥 Fire within 10 cm! Stopping and Extinguishing...");
      moveStop();
      activateExtinguisher(true);
      delay(5000);  
      activateExtinguisher(false);
      fireDetected = false;
    }
  }
}

int readPing() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  return (distance == 0) ? 250 : distance;
}

void avoidObstacle() {
  moveStop();
  delay(300);
  moveBackward();
  delay(400);
  moveStop();
  delay(300);
  int distanceRight = lookRight();
  delay(300);
  int distanceLeft = lookLeft();
  delay(300);
  if (distanceRight >= distanceLeft) {
    turnRight();
  } else {
    turnLeft();
  }
  moveStop();
}

int checkFlame() {
  
  if (digitalRead(flameNorth) == HIGH){return 1;}
  if (digitalRead(flameEast) == HIGH) {return 3;}
  if (digitalRead(flameWest) == HIGH) {return 4;}
  return 0;  
}

bool checkGas() {
  return digitalRead(MQ2_PIN) == LOW;
}

int lookRight() {
  servo_motor.write(0);
  delay(500);
  int distance = readPing();
  servo_motor.write(90);
  return distance;
}

int lookLeft() {
  servo_motor.write(180);
  delay(500);
  int distance = readPing();
  servo_motor.write(90);
  return distance;
}

void moveStop() { 
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); 
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); 
}

void moveForward() { 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
}

void moveBackward() { 
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); 
}

void turnLeft() { 
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
  delay(500); 
}

void turnRight() { 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); 
  delay(500); 
}

void driveMotor(int direction) {
  switch (direction) {
    case 1: moveForward(); break;
    case 2: moveBackward(); break;
    case 3: turnRight(); break;
    case 4: turnLeft(); break;
    case 5: moveStop(); break;
  }
}

void activateExtinguisher(bool turnOn) {
  if (turnOn) {
    digitalWrite(pumpPin, HIGH);
    rotatePipeServo();
  } else {
    digitalWrite(pumpPin, LOW);
  }
}

void rotatePipeServo() {
  Serial.println("Rotating pipe servo...");
  for (int pos = 0; pos <= 180; pos += 5) {
    servo_motor.write(pos);
    delay(15);
  }
  for (int pos = 180; pos >= 0; pos -= 5) {
    servo_motor.write(pos);
    delay(15);
  }
}

void controlCar(char command) {
  switch (command) {
    case 'F': moveForward();BTSerial.println("FORWARD"); break;
    case 'B': moveBackward();BTSerial.println("BACKWARD"); break;
    case 'R': turnRight();BTSerial.println("RIGHT TURN"); break;
    case 'L': turnLeft();BTSerial.println("LEFT TURN"); break;
    case 'S': moveStop();BTSerial.println("STOP"); break;
    case 'W': activateExtinguisher(true);BTSerial.println("Extinguish START"); break;
    case 'P': servo_motor.write(90);activateExtinguisher(false);BTSerial.println("Extinguish STOP"); break;
  }
}