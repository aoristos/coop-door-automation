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
 
// built-in library <wire.h> serves the communication with the I2C bus
#include <Wire.h>

// https://github.com/PaulStoffregen/Time/releases
#include <TimeLib.h>

// https://github.com/PaulStoffregen/DS1307RTC/releases
#include <DS1307RTC.h>

// Arduino pins for motor 1 
const byte EN1 = 6; // PWM
const byte IN1 = 7; // MOTOR1 UP
const byte IN2 = 8; // MOTOR1 DOWN

//Arduino pins for the switches
const byte upperSwitch = 11; // upperSwitch is HIGH when the door is fully open
const byte lowerSwitch = 12; // lowerSwitch is HIGH when the door is fully closed

// Flags
// set nighTime = false if it is daytime
bool nightTime = false;
// flag to indicate the state of the switches
// switches are 'LOW' when activated
byte upperSwitchState = LOW; 
byte lowerSwitchState = LOW;
// set flag Alarm = true if runtimeLimit is exceeded and no switch is activated
bool Alarm = false;

// Security
// limit the runtime to protect damage when a switch is never activated
// runtimeLimit depends on the motorspeed and the diameter of the spool
int RuntimeLimit = 5000; // Security runtime limit

void setup()
{

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // initialize pinmodes
  pinMode(EN1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(upperSwitch, INPUT_PULLUP);
  pinMode(lowerSwitch, INPUT_PULLUP);
}

void loop()
{ 
    tmElements_t tm;

  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
  


  
  upperSwitchState = digitalRead(upperSwitch);

  // if it is daytime AND the upperSwitch is not activated
  if (nightTime == false && upperSwitchState == HIGH) {
    motor1Up();     // call function motor1Up()
    Serial.print("motor1Up running :");
    Serial.println(upperSwitchState); 
  }
  else {
    motor1Stop(); // stop the motor
    Serial.print("motor1Up stopped :");
    Serial.println(upperSwitchState);
  }

  delay(10000);
}

// void functions do not return values
// this function activates motor1Up
void motor1Up() {  
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2, LOW); 
}

// void functions do not return values
// this function stops the motor
void motor1Stop() {
 digitalWrite(IN1, LOW);
 digitalWrite(IN2, LOW);  
}

// print number as 2-digits in case number is only 1-digit 
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
