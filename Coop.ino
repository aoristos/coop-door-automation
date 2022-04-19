/*
 * Open and close a chicken cooop door, using an Arduino, a MotorShield-L298N and a DS1307RTC
 * 
 * documentation:
 *  "D:\Documents\Mijn PaperPort-documenten\Elektronica\Arduino\Shields\MotorShield-L298N-handleiding.pdf"
 *  Using a Real Time Clock with Arduino ( https://dronebotworkshop.com/real-time-clock-arduino/ )
 *  Controlling DC Motors with the L298N H Bridge and Arduino ( https://dronebotworkshop.com/dc-motors-l298n-h-bridge/ )
 *  
 * Hardware:
 *  Arduino Uno
 *  MotorShield-L298N
 *  DS1307 Tiny RTC
 *  
 * include libraries:
 *  <wire.h> the built-in library to communicate with the I2C bus
 *  <TimeLib.h> library ( https://github.com/PaulStoffregen/Time/releases )
 *  <DS1307RTC.h> library ( https://github.com/PaulStoffregen/DS1307RTC/releases )
 * 
 */
 
// * built-in library <wire.h> serves the communication with the I2C bus
#include <Wire.h>

// * https://github.com/PaulStoffregen/Time/releases
#include <TimeLib.h>

// * https://github.com/PaulStoffregen/DS1307RTC/releases
#include <DS1307RTC.h>

// motor 1
const byte EN1 = 6; // PWM
const byte IN1 = 7; // MOTOR1 UP
const byte IN2 = 8; // MOTOR1 DOWN

// clock
bool nightTime = false;

//switches
const byte upperSwitch = 11; // upperSwitch is HIGH when the door is fully open
const byte lowerSwitch = 12; // lowerSwitch is HIGH when the door is fully closed
byte upperSwitchState = 0; 
byte lowerSwitchState = 0;

// Security
int Runtime = 3000; // Security Runtime Limit

void setup()
{

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // initialize pinmodes
  pinMode(EN1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(upperSwitch, INPUT);
  pinMode(lowerSwitch, INPUT);
}

void loop()
{ 
  upperSwitchState = digitalRead(upperSwitch);
  Serial.println(upperSwitchState);
  
  if (nightTime == false && upperSwitchState == HIGH) {
    // call function motor1Up()
    Serial.print("in the loop :");
    Serial.println(upperSwitchState);
    motor1Up();   
  }
  else {
    motor1Stop();
  }

}


// void functions do not return values
// this function activates motor1Up
void motor1Up() {  
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2, LOW); 
}

// void functions do not return values
// this function activates motor1Down
void motor1Down(int Runtime) {  
  digitalWrite(IN1,LOW);
  digitalWrite(IN2, HIGH);
  delay (Runtime);
  digitalWrite(IN2, LOW);    
}

// void functions do not return values
// this function stops the motor
void motor1Stop() {
 digitalWrite(IN1, LOW);
 digitalWrite(IN2, LOW);  
}
