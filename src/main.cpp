#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#define inEN 34     //CH1 end-stop
#define inX1 35
#define inX2 32
#define inX3 33
#define inX4 25

#define inLS1_OS 13
#define inLS2_DS 12
#define inLS3_OS 14
#define inLS4_DS 27

#define CH1_STEP 18  // CH1 step
#define CH1_DIR 5
#define CH2_STEP 15     // má být 2, je to pin D2
#define CH2_DIR 4

#define inLED 2

boolean bENABLE = false;
boolean bX1 = false;
boolean bX2 = false;
boolean bX3 = false;
boolean bX4 = false;
boolean bLS1 = false;
boolean bLS2 = false;
boolean bLS3 = false;
boolean bLS4 = false;
int customDelay = 450;


boolean offtimer_T1 = false;
unsigned long T1_millis = 0;    //debounce pushbuttons, selectors
unsigned long T1_delay = 20;    // 20ms
void timerT1(){

    if( millis() >= T1_millis + T1_delay){
        T1_millis = millis();
        offtimer_T1 = true;
    }
}

boolean offtimer_T2 = false;    // CLOCK 2Hz
unsigned long T2_millis = 0;
unsigned long T2_delay = 500;
void timerT2(){
  if( millis() >= T2_millis + T2_delay){
    T2_millis = millis();
    offtimer_T2 = !offtimer_T2;
  }
}

boolean offtimer_T3 = false;      // refresh speed ramps timer
unsigned long T3_millis = 0;
unsigned long T3_delay = 2;       // every 5ms
void timerT3(){
  if(millis() >= T3_millis + T3_delay){
    T3_millis = millis();
    offtimer_T3 = true;
  }
}

unsigned long last_rampMillis = 0;
unsigned long rampMillis = 0;         // miliseconds from last ramp update
const int MIN_SPEED_for_percent = 5;
const int MIN_SPEED_rev_perc = -10;
const int MAX_SPEED_for_perc = 100;   //max forward speed (%)
const int MAX_SPEED_rev_perc = -100;  //max reverse speed (%)
const int SLOW_SPEED_perc = 80;       // speed when not synchronisated (%)
double motor1_cur_speed = 0.0;             // motor 1 - current speed (%)
int motor1_cur_rpm = 0;               // motor 1 - current rpm
int motor1_m_min = 0;                 // motor 1 - velocity - meters per minute
int motor1_step_sec = 0;              // motor 1 - steps per second
int motor1_custom_delay = 1500;         // motor 1 - custom delay between steps / 2 (us)
double motor1_vel_add = 0.0;               // velocity addition (% speed/ms)
double motor1_vel_sub = 0.0;               // velocity substraction (% speed/ms)
int accel_ramp = 8000;
int decel_ramp = 5000;
int max_rpm = 3000;
double steps_per_round = 200;
double AX1_pos_steps = 0;
double AX1_pos_mm = 0;
double AX1_MIN = 0;
double AX1_MAX = 1788;
double AX1_UP_H_SPEED_LIMIT = 1600;
double AX1_LO_H_SPEED_LIMIT = 200;
double PULLEY_DIAMETER = 12.7414 ;  // diameter of pulley on motor side (mm)
int telegram_counter = 0;
boolean AX1_REF_SYNCHRO = false;

void rampCH1(){
  boolean forward = false;
  boolean reverse = false;
  boolean LS_DS = false;
  boolean LS_OS = true;
  forward = bX1; reverse = bX2; LS_DS = bLS1;
  boolean slowing_down = false;
  boolean low_speed = false;
  boolean low_speed_for = false;
  boolean low_speed_rev = false;
  
  if(forward && !LS_DS){
    AX1_REF_SYNCHRO = true;
    AX1_pos_steps = 0;
  }
  
  AX1_pos_mm = (AX1_pos_steps/steps_per_round)*(PI*PULLEY_DIAMETER);

  if(AX1_pos_mm > AX1_UP_H_SPEED_LIMIT){
    low_speed_rev = true;
  }
  if(AX1_pos_mm < AX1_LO_H_SPEED_LIMIT){
    low_speed_for = true;
  }
  if(AX1_pos_mm > AX1_MAX){
    LS_OS = false;
  }

  if(!AX1_REF_SYNCHRO || (forward && low_speed_for) || (reverse && low_speed_rev)){
    low_speed = true;
  }
  else{
    low_speed = false;
  }

  rampMillis = millis() - last_rampMillis;
  last_rampMillis = millis();
  motor1_vel_add = rampMillis * (100/double(accel_ramp));
  motor1_vel_sub = rampMillis * (100/double(decel_ramp));

  if((motor1_cur_speed > 0 && !forward) || (motor1_cur_speed < 0 && ! reverse)){
    slowing_down = true;
  }
  else{
    slowing_down = false;
  }
  
  if(forward){
    if((motor1_cur_speed < MAX_SPEED_for_perc) && (!low_speed || (low_speed && (motor1_cur_speed < SLOW_SPEED_perc)))){
      motor1_cur_speed = motor1_cur_speed + motor1_vel_add;
    }
  }

  if(reverse){
    if( (motor1_cur_speed > MAX_SPEED_rev_perc) && (!low_speed || (low_speed && (abs(motor1_cur_speed) < SLOW_SPEED_perc))) ){
      motor1_cur_speed = motor1_cur_speed - motor1_vel_add;
    }
  }

  if((!forward && !reverse) || slowing_down || (low_speed && abs(motor1_cur_speed) > SLOW_SPEED_perc) ){
    if(motor1_cur_speed > 0){
      if(motor1_cur_speed < 30){
        motor1_cur_speed = 0;
      }
      else{
        motor1_cur_speed = motor1_cur_speed - motor1_vel_sub;
      }
    }
    if(motor1_cur_speed < 0){
      if(motor1_cur_speed > (-30)){
        motor1_cur_speed = 0;
      }
      else{
        motor1_cur_speed = motor1_cur_speed + motor1_vel_sub;
      }
    }
  }

  if(!LS_OS && reverse && AX1_REF_SYNCHRO){
    motor1_cur_speed = 0;
  }

  motor1_cur_rpm = (double(motor1_cur_speed)/100) * max_rpm;
  motor1_step_sec = (double(motor1_cur_rpm)/60) * steps_per_round;
  if(abs(motor1_cur_speed) >= MIN_SPEED_for_percent){
    motor1_custom_delay = ((1000/double(motor1_step_sec))/2)*1000;
  }
  else{
    motor1_custom_delay = 1000;
  }
  motor1_custom_delay = abs(motor1_custom_delay);

  /*
  telegram_counter++;
  if(telegram_counter >= 40){  
  
  Serial.print("bin: ");
  Serial.print(forward);
  Serial.print("-");
  Serial.print(reverse);
  Serial.print(" - ");  
  Serial.print(" - AX1_pos_step:");
  Serial.print(AX1_pos_steps);  
  Serial.print(" - AX1_pos_mm: ");
  Serial.println(AX1_pos_mm); 
  
    telegram_counter = 0;
  }
  */
}

void setup() {
  // put your setup code here, to run once:
  pinMode(inEN,INPUT);
  pinMode(inX1,INPUT);
  pinMode(inX2,INPUT);
  pinMode(inX3,INPUT);
  pinMode(inX4,INPUT);
  pinMode(inLS1_OS,INPUT);
  pinMode(inLS2_DS,INPUT);
  pinMode(inLS3_OS,INPUT);
  pinMode(inLS4_DS,INPUT);

  pinMode(CH1_STEP,OUTPUT);
  pinMode(CH1_DIR,OUTPUT);
  pinMode(CH2_STEP,OUTPUT);
  pinMode(CH2_DIR,OUTPUT);
  pinMode(inLED,OUTPUT);

  Serial.begin(115200);
  delay(5000);
}

void loop() {
  timerT1(); timerT2(); timerT3();
    digitalWrite(inLED,offtimer_T2);

  if(offtimer_T1){                    // pushbuttons and limit switches read
    bENABLE = !digitalRead(inEN);     // SPARE    
    bX1 = !digitalRead(inX1);
    bX2 = !digitalRead(inX2);
    bX3 = !digitalRead(inX3);
    bX4 = !digitalRead(inX4);
    bLS1 = !digitalRead(inLS1_OS);
    bLS2 = !digitalRead(inLS2_DS);
    bLS3 = !digitalRead(inLS3_OS);
    bLS4 = !digitalRead(inLS4_DS);
  }

  if(offtimer_T3){                    //ramps
    rampCH1();
  }

  if( bLS1 && motor1_cur_speed > 0){  // RUN CV
    digitalWrite(CH1_DIR,LOW);
    digitalWrite(CH1_STEP,HIGH);
    delayMicroseconds(motor1_custom_delay);
    digitalWrite(CH1_STEP, LOW);
    delayMicroseconds(motor1_custom_delay);
    AX1_pos_steps --;    
  }

  if( motor1_cur_speed < 0 ){       //RUN CCV
    digitalWrite(CH1_DIR,HIGH);
    digitalWrite(CH1_STEP,HIGH);
    delayMicroseconds(motor1_custom_delay);
    digitalWrite(CH1_STEP,LOW);
    delayMicroseconds(motor1_custom_delay);
    AX1_pos_steps ++;
  }

  offtimer_T1 = false; offtimer_T3 = false;
}