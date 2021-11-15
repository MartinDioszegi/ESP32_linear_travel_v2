#include <Arduino.h>
#include <Wire.h>
#include <math.h>

hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
volatile uint8_t led0stat = 0;

hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;
volatile uint8_t led1stat = 0;

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
#define CH2_STEP 2     // má být 2, je to pin D2
#define CH2_DIR 4

boolean bENABLE = false;
boolean bX1 = false;
boolean bX2 = false;
boolean bX3 = false;
boolean bX4 = false;
boolean bLS1 = false;
boolean bLS2 = false;
boolean bLS3 = false;
boolean bLS4 = false;

boolean offtimer_T1 = false;// debounce pushbuttons, selectors - 20ms a teď velkej test  . . . na řádek 39 doplnim koment
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

boolean offtimer_T3 = false;      // refresh speed ramps timer 10ms
unsigned long T3_millis = 0;
unsigned long T3_delay = 10;       // every 10ms
void timerT3(){
  if(millis() >= T3_millis + T3_delay){
    T3_millis = millis();
    offtimer_T3 = true;
  }
}

unsigned long AX1_last_rampMillis = 0;
unsigned long AX1_rampMillis = 0;         // miliseconds from last ramp update
const int AX1_MIN_SPEED_for = 1;
const int AX1_MIN_SPEED_rev = -1;
const int AX1_MAX_SPEED_for = 100;   //max forward speed (%)
const int AX1_MAX_SPEED_rev = -100;  //max reverse speed (%)
const int AX1_SLOW_SPEED = 100;       // speed when not synchronisated (%)
double AX1_cur_speed = 0.0;             // motor 1 - current speed (%)
int AX1_cur_rpm = 0;               // motor 1 - current rpm
int AX1_m_min = 0;                 // motor 1 - velocity - meters per minute
int AX1_step_sec = 0;              // motor 1 - steps per second
int AX1_custom_delay = 1500;         // motor 1 - custom delay between steps / 2 (us)
double AX1_vel_add = 0.0;               // velocity addition (% speed/ms)
double AX1_vel_sub = 0.0;               // velocity substraction (% speed/ms)
int AX1_accel_ramp = 800;
int AX1_decel_ramp = 800;
int AX1_max_rpm = 400;
double AX1_steps_per_round = 3200;
double AX1_pos_steps = 0;
double AX1_pos_mm = 0;
double AX1_MIN_mm = 0;
double AX1_MAX_mm = 1788;
double AX1_UP_H_SPEED_LIMIT = 1600;
double AX1_LO_H_SPEED_LIMIT = 200;
double AX1_PULLEY_DIAMETER = 12.7414 ;  // diameter of pulley on motor side (mm)
boolean AX1_REF_SYNCHRO = false;

unsigned long AX2_last_rampMillis = 0;
unsigned long AX2_rampMillis = 0;         // miliseconds from last ramp update
const int AX2_MIN_SPEED_for = 1;
const int AX2_MIN_SPEED_rev = -1;
const int AX2_MAX_SPEED_for = 100;   //max forward speed (%)
const int AX2_MAX_SPEED_rev = -100;  //max reverse speed (%)
const int AX2_SLOW_SPEED = 100;       // speed when not synchronisated (%)
double AX2_cur_speed = 0.0;             // motor 1 - current speed (%)
int AX2_cur_rpm = 0;               // motor 2 - current rpm
int AX2_m_min = 0;                 // motor 2 - velocity - meters per minute
int AX2_step_sec = 0;              // motor 2 - steps per second
int AX2_custom_delay = 1500;         // motor 2 - custom delay between steps / 2 (us)
double AX2_vel_add = 0.0;               // velocity addition (% speed/ms)
double AX2_vel_sub = 0.0;               // velocity substraction (% speed/ms)
int AX2_accel_ramp = 800;
int AX2_decel_ramp = 800;
int AX2_max_rpm = 400;
double AX2_steps_per_round = 3200;
double AX2_pos_steps = 0;
double AX2_pos_mm = 0;
double AX2_MIN_mm = 0;
double AX2_MAX_mm = 1788;
double AX2_UP_H_SPEED_LIMIT = 1600;
double AX2_LO_H_SPEED_LIMIT = 200;
double AX2_PULLEY_DIAMETER = 12.7414 ;  // diameter of pulley on motor side (mm)
boolean AX2_REF_SYNCHRO = false;

void ramp_AX1(){
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
  
  AX1_pos_mm = (AX1_pos_steps/AX1_steps_per_round)*(PI*AX1_PULLEY_DIAMETER);

  if(AX1_pos_mm > AX1_UP_H_SPEED_LIMIT){
    low_speed_rev = true;
  }
  if(AX1_pos_mm < AX1_LO_H_SPEED_LIMIT){
    low_speed_for = true;
  }
  if(AX1_pos_mm > AX1_MAX_mm){
    LS_OS = false;
  }

  if(!AX1_REF_SYNCHRO || (forward && low_speed_for) || (reverse && low_speed_rev)){
    low_speed = true;
  }
  else{
    low_speed = false;
  }

  AX1_rampMillis = millis() - AX1_last_rampMillis;
  AX1_last_rampMillis = millis();
  AX1_vel_add = AX1_rampMillis * (100/double(AX1_accel_ramp));
  AX1_vel_sub = AX1_rampMillis * (100/double(AX1_decel_ramp));

  if((AX1_cur_speed > 0 && !forward) || (AX1_cur_speed < 0 && ! reverse)){
    slowing_down = true;
  }
  else{
    slowing_down = false;
  }
  
  if(forward){
    if((AX1_cur_speed < AX1_MAX_SPEED_for) && (!low_speed || (low_speed && (AX1_cur_speed < AX1_SLOW_SPEED)))){
      AX1_cur_speed = AX1_cur_speed + AX1_vel_add;
    }
  }

  if(reverse){
    if( (AX1_cur_speed > AX1_MAX_SPEED_rev) && (!low_speed || (low_speed && (abs(AX1_cur_speed) < AX1_SLOW_SPEED))) ){
      AX1_cur_speed = AX1_cur_speed - AX1_vel_add;
    }
  }

  if((!forward && !reverse) || slowing_down || (low_speed && abs(AX1_cur_speed) > AX1_SLOW_SPEED) ){
    if(AX1_cur_speed > 0){
      if(AX1_cur_speed < AX1_MIN_SPEED_for){
        AX1_cur_speed = 0;
      }
      else{
        AX1_cur_speed = AX1_cur_speed - AX1_vel_sub;
      }
    }
    if(AX1_cur_speed < 0){
      if(AX1_cur_speed > (AX1_MIN_SPEED_rev)){
        AX1_cur_speed = 0;
      }
      else{
        AX1_cur_speed = AX1_cur_speed + AX1_vel_sub;
      }
    }
  }

  if(!LS_OS && reverse && AX1_REF_SYNCHRO){
    AX1_cur_speed = 0;
  }

  AX1_cur_rpm = (double(AX1_cur_speed)/100) * AX1_max_rpm;
  AX1_step_sec = (double(AX1_cur_rpm)/60) * AX1_steps_per_round;
  if(abs(AX1_cur_speed) >= AX1_MIN_SPEED_for){
    AX1_custom_delay = (1/double(AX1_step_sec))*1000000;
  }
  else{
    AX1_custom_delay = 1000;
  }
  AX1_custom_delay = abs(AX1_custom_delay);
}

void ramp_AX2(){
  boolean forward = false;
  boolean reverse = false;
  boolean LS_DS = false;
  boolean LS_OS = true;
  forward = bX3; reverse = bX4; LS_DS = bLS1;
  boolean slowing_down = false;
  boolean low_speed = false;
  boolean low_speed_for = false;
  boolean low_speed_rev = false;
  
  if(forward && !LS_DS){
    AX2_REF_SYNCHRO = true;
    AX2_pos_steps = 0;
  }
  
  AX2_pos_mm = (AX2_pos_steps/AX2_steps_per_round)*(PI*AX2_PULLEY_DIAMETER);

  if(AX2_pos_mm > AX2_UP_H_SPEED_LIMIT){
    low_speed_rev = true;
  }
  if(AX2_pos_mm < AX2_LO_H_SPEED_LIMIT){
    low_speed_for = true;
  }
  if(AX2_pos_mm > AX2_MAX_mm){
    LS_OS = false;
  }

  if(!AX2_REF_SYNCHRO || (forward && low_speed_for) || (reverse && low_speed_rev)){
    low_speed = true;
  }
  else{
    low_speed = false;
  }

  AX2_rampMillis = millis() - AX2_last_rampMillis;
  AX2_last_rampMillis = millis();
  AX2_vel_add = AX2_rampMillis * (100/double(AX2_accel_ramp));
  AX2_vel_sub = AX2_rampMillis * (100/double(AX2_decel_ramp));

  if((AX2_cur_speed > 0 && !forward) || (AX2_cur_speed < 0 && ! reverse)){
    slowing_down = true;
  }
  else{
    slowing_down = false;
  }
  
  if(forward){
    if((AX2_cur_speed < AX2_MAX_SPEED_for) && (!low_speed || (low_speed && (AX2_cur_speed < AX2_SLOW_SPEED)))){
      AX2_cur_speed = AX2_cur_speed + AX2_vel_add;
    }
  }

  if(reverse){
    if( (AX2_cur_speed > AX2_MAX_SPEED_rev) && (!low_speed || (low_speed && (abs(AX2_cur_speed) < AX2_SLOW_SPEED))) ){
      AX2_cur_speed = AX2_cur_speed - AX2_vel_add;
    }
  }

  if((!forward && !reverse) || slowing_down || (low_speed && abs(AX2_cur_speed) > AX2_SLOW_SPEED) ){
    if(AX2_cur_speed > 0){
      if(AX2_cur_speed < AX2_MIN_SPEED_for){
        AX2_cur_speed = 0;
      }
      else{
        AX2_cur_speed = AX2_cur_speed - AX2_vel_sub;
      }
    }
    if(AX2_cur_speed < 0){
      if(AX2_cur_speed > (AX2_MIN_SPEED_rev)){
        AX2_cur_speed = 0;
      }
      else{
        AX2_cur_speed = AX2_cur_speed + AX2_vel_sub;
      }
    }
  }

  if(!LS_OS && reverse && AX2_REF_SYNCHRO){
    AX2_cur_speed = 0;
  }

  AX2_cur_rpm = (double(AX2_cur_speed)/100) * AX2_max_rpm;
  AX2_step_sec = (double(AX2_cur_rpm)/60) * AX2_steps_per_round;
  if(abs(AX2_cur_speed) >= AX2_MIN_SPEED_for){
    AX2_custom_delay = (1/double(AX2_step_sec))*1000000;
  }
  else{
    AX2_custom_delay = 1000;
  }
  AX2_custom_delay = abs(AX2_custom_delay);
}

void IRAM_ATTR onTimer0(){
  portENTER_CRITICAL_ISR(&timerMux0);
  led0stat = 1 - led0stat;
  if( bLS1 && AX1_cur_speed > 0){  // RUN CV
    digitalWrite(CH1_STEP, led0stat);    
    //AX1_pos_steps --;    
  }
  if( AX1_cur_speed < 0 ){       //RUN CCV
    digitalWrite(CH1_STEP, led0stat);
    //AX1_pos_steps ++;
  }  
  timerAlarmWrite(timer0, abs(AX1_custom_delay), true);
  portEXIT_CRITICAL_ISR(&timerMux0);
}

void IRAM_ATTR onTimer1(){
  portENTER_CRITICAL_ISR(&timerMux1);
  led1stat = 1 - led1stat;
  if( bLS1 && AX2_cur_speed > 0){
    digitalWrite(CH2_STEP, led1stat);
    //AX2_pos_steps --;
  }
  if( AX2_cur_speed < 0){
    digitalWrite(CH2_STEP, led1stat);
    //AX2_pos_steps ++;
  }
  timerAlarmWrite(timer1, abs(AX2_custom_delay), true);
  portEXIT_CRITICAL_ISR(&timerMux1);
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

  Serial.begin(115200);
  Serial.println("starting timer0");
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer0, true);
  timerAlarmWrite(timer0, 25000, true);
  timerAlarmEnable(timer0);

  Serial.println("starting timer1");
  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 1000000, true);
  timerAlarmEnable(timer1);

  delay(8000);
}

void loop() {
  timerT1(); timerT2(); timerT3();
    //digitalWrite(inLED,offtimer_T2);

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
    ramp_AX1();
    ramp_AX2();
    if( bLS1 && AX1_cur_speed > 0){   // RUN CV
      digitalWrite(CH1_DIR,LOW);      
    }
    if( AX1_cur_speed < 0 ){          // RUN CCV
      digitalWrite(CH1_DIR,HIGH);    
    }
    if( bLS1 && AX2_cur_speed > 0){   // RUN CV
      digitalWrite(CH2_DIR,LOW);
    }
    if( AX2_cur_speed < 0){           // RUN CCV
      digitalWrite(CH2_DIR,HIGH);
    }
  }  

  offtimer_T1 = false; offtimer_T3 = false;
}