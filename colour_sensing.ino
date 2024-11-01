// imports!
#include "MeMCore.h"

// constants!
#define PRINT 1
#define TIMEOUT 2000 // Max microseconds to wait; choose according to max distance of wall
#define SPEED_OF_SOUND 340 // Update according to your own experiment
#define IR_LOW_READING 0
#define IR_HIGH_READING 1000
#define IR_LOW_DIST 0 //mm
#define IR_HIGH_DIST 100 //mm
#define ULTRASONIC_READING_INTERVAL 10 // millis

// pins!
#define ULTRASONIC 12
#define PUSH_BUTTON A7
#define IR_PIN_IN A1
#define PIN_A 2
#define PIN_B 3

// objects!
MeLineFollower lineFinder(PORT_2); // assigning lineFinder to RJ25 port 2
MeDCMotor leftMotor(M1); // assigning leftMotor to port M1
MeDCMotor rightMotor(M2);
uint8_t motorSpeed = 255;

enum MovementTypes {
  WALLTRACK,
  LEFT_TURN,
  LEFT_U_TURN,
  RIGHT_TURN,
  RIGHT_U_TURN,
  ON_THE_SPOT_U_TURN,
  COLOR_SENSE,
  STOP
};

Motion state = WALLTRACK; //default setting where it is moving forward with walltracking 
//NEWWW - Color Sensing
int balanceArray[3][3] = {
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
};

enum Color {
  RED, GREEN, ORANGE, PINK, LIGHT_BLUE, WHITE,
};
int current_Color[3] = {0, 0, 0};

bool is_at_line(){
  int sensor_state = lineFinder.readSensors();
  return sensor_state == S1_IN_S2_IN; //There are four possible cases but this function should only return true given that BOTH sensors are on the line
} 

// function headers!
void differentialSteer(MeDCMotor *leftMotor, MeDCMotor *rightMotor, double motorSpeed, double rotation);
void transmitUltrasonic();
float receiveUltrasonic();
double receiveIR();

// variables!
enum MovementTypes movement = STRAIGHT;
double rotation = 0;
double left, right = 0;
double kp = 0.07;
long long lastReadUltrasonic;

void stop_moving(){
  leftMotor.stop();
  rightMotor.stop();
}

void setup() { 
  pinMode(ULTRASONIC, OUTPUT);
  pinMode(ULTRASONIC, INPUT);
  pinMode(PUSH_BUTTON, INPUT); // Setup A7 as input for the push button
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  Serial.begin(9600); 
}

void loop() {
  switch (movement)
  {

  case WALLTRACK:
    // ultrasonic code (10 between each reading) //TODO optimise
    if (millis() - lastReadUltrasonic > ULTRASONIC_READING_INTERVAL) {
      transmitUltrasonic();
      double left = receiveUltrasonic(); //in cm
    }
    // ir receiver and transnmitter
    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, LOW);
    double right = receiveIR(); //in cm
    // calculate differential steer needed
    double rotation = (left == -1 || right == -1) ? ((left - right) * kp) : 0; //if out of bounds, just go straight
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, rotation);
    //TODO get line follower values
    //TODO change case according to color
    #if PRINT
      Serial.print('Left: ');
      Serial.print(left);
      Serial.print(' | Right: ');
      Serial.print(right);
      Serial.print(' | Rotation: ');
      Serial.print(rotation);
    #endif
    break;

  case LEFT_TURN:
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, -1);
    delay(1000);
    movement = WALLTRACK;
    break;

  case LEFT_U_TURN:
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, -1);
    delay(1000);
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, 0);
    delay(1000);
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, -1);
    delay(1000);
    movement = WALLTRACK;
    break;

  case RIGHT_TURN:
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, 1);
    delay(1000);
    movement = WALLTRACK;
    break;

  case RIGHT_U_TURN:
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, 1);
    delay(1000);
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, 0);
    delay(1000);
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, 1);
    delay(1000);
    movement = WALLTRACK;
    break;
  
  case ON_THE_SPOT_U_TURN:
    differentialSteer(&leftMotor, &rightMotor, motorSpeed, -1);
    delay(2000);
    movement = WALLTRACK;
    break;

  case STOP:
    leftMotor.stop();
    rightMotor.stop();
    while (1)
    {
      //play music
    }
    break;
  
  default:
    break;
  }

  #if PRINT
    Serial.print("Movement type: ");
    Serial.println(movement);
  #endif


  //Edits for the motion
  if (state == WALLTRACK and is_at_line()){
    stop_moving();
    state == COLOR_SENSE;
  }
  if (state == COLOR_SENSE){
    read_color();//Use the loop in led.ino to make it sense the colour
    int color_predict = match_color();
    /*RED, ORANGE, PINK: R > 200  
          ORANGE: G > 100 && B < 100
          RED: G < 100 && B < 100
          PINK: G > 150 && B < 200
      GREEN: G > 200
      BLUE: B > 200
    */
    if (color_predict == RED){
      state = LEFT_TURN;
    }
    else if (color_predict == GREEN){
      state = RIGHT_TURN;
    }
    else if (color_predict == LIGHT_BLUE){
      state = RIGHT_U_TURN;
    }
    else if (color_predict == ORANGE){
      state = U_TURN;
    }
    else if (color_predict == PINK){
      state = LEFT_U_TURN;
    }
    else{
      state = STOP;
    }
    delay(10);
}

void transmitUltrasonic() {
  digitalWrite(ULTRASONIC, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC, HIGH);
  delayMicroseconds(10); 
  digitalWrite(ULTRASONIC, LOW);
}

void differentialSteer(MeDCMotor *leftMotor, MeDCMotor *rightMotor, double motorSpeed, double rotation) {
  double slower = 1-2*fabs(rotation);
  if (rotation < 0) { //left
    leftMotor ->run(motorSpeed);
    rightMotor->run(motorSpeed*slower);
  } else {
    leftMotor->run(motorSpeed*slower);
    rightMotor->run(motorSpeed);
  }
}

float receiveUltrasonic() { // in cm
  long duration = pulseIn(ULTRASONIC, HIGH, TIMEOUT);
  if (duration > 0) {
    float distance = (duration / 2.0 / 1000000 * SPEED_OF_SOUND * 100) - 4.5; // 4.5 is the distance from ultrasonic to robot side
    #if PRINT
      Serial.print("distance(cm) = ");
      Serial.println(distance);
    #endif
    return min(distance, 15);
  }
  else {
    #if PRINT
      Serial.println("out of range");
    #endif
    return -1;
  }
}

double receiveIR() {
  return (val*(IR_HIGH_DIST-IR_LOW_DIST))/(IR_HIGH_READING-IR_LOW_READING)+IR_LOW_DIST;
}
