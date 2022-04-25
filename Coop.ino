/*
 * Open and close a chicken cooop door, using an Arduino Uno, a MotorShield-L298N and a DS1307RTC
 * 
 * documentation:
 *  "D:\Documents\Mijn PaperPort-documenten\Elektronica\Arduino\Shields\MotorShield-L298N-handleiding.pdf"
 *  "Using a Real Time Clock with Arduino" ( https://dronebotworkshop.com/real-time-clock-arduino/ )
 *  "Controlling DC Motors with the L298N H Bridge and Arduino" ( https://dronebotworkshop.com/dc-motors-l298n-h-bridge/ )
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

// TODO: set appropriate offset parameters:
  // TODO: subtract 1 hour (tm.Hour - summerTimeOffset) when the DS1307RTC is running in summertime modus (daylight saving time values)(zomertijd)
    byte summerTimeOffset = 1;

  //make sure you leave enough time for the chickens to go to bed
  // TODO: increase sunSetNow with a safety offset time (e.g. 30 or 60 minutes)
    byte sunSetOffset = 60;
  // TODO: increase sunRiseNow with a safety offset time (e.g. 30 or 60 minutes)
    byte sunRiseOffset = 30;

  // TODO: the HIGH or LOW status depends on the switch type (NORMAL_OPEN or NORMAL_CLOSE), the wiring and the configuration of the switch INPUT PINS
  // set the desired configuration here:
    const bool SWITCH_IS_ACTIVATED = LOW;
    // const bool SWITCH_IS_ACTIVATED = HIGH;
    const bool SWITCH_NOT_ACTIVATED = HIGH;
    // const bool SWITCH_NOT_ACTIVATED = LOW;

 // include libraries
// built-in library <wire.h> serves the communication with the I2C bus
  #include <Wire.h>

// TimeLib.h (author Paul Stoffregen)
// https://github.com/PaulStoffregen/Time/releases
  #include <TimeLib.h>

//DS1307RTC.h (author Paul Stoffregen)
// https://github.com/PaulStoffregen/DS1307RTC/releases
  #include <DS1307RTC.h>

// flag to indicate the state of the switches
  byte upperSwitchState = SWITCH_NOT_ACTIVATED; 
  byte lowerSwitchState = SWITCH_NOT_ACTIVATED;
  
// Arduino pins for motor 1 
  const byte EN1 = 6; // PWM
  const byte IN1 = 7; // MOTOR1 UP
  const byte IN2 = 8; // MOTOR1 DOWN

//Arduino pins for the switches
  const byte upperSwitch = 11; // SWITCH_IS_ACTIVATED when the door is fully open
  const byte lowerSwitch = 12; // SWITCH_IS_ACTIVATED when the door is fully closed

  // Values based on NOAA_Solar_Calculations_year ( https://gml.noaa.gov/grad/solcalc/calcdetails.html )
  // Calculations for Belgium-Bellegem latitude 50,50° / longitude 3,16°  Timezone GMT +1
  // Times are GMT+1 ; no summertime (daylight saving time) 
  // Values for day 8 of every month (day 8 is roughly medium between the season pivots at day 21)  

  int sun_rise[12]={
 //Jan   Feb   Mar   Apr   May   Jun   Jul   Aug   Sep   Oct   Nov   Dec
   526, 491, 436, 368, 310, 277, 286, 325, 372, 419, 470, 515};
 // 08:46, 08:11, 07:16, 06:08, 05:09, 04:36, 04:45, 05:25, 06:12, 06:58, 07:50, 08:34     - Times are GMT+1 no summer saving as clock isn't adjusted

  int sun_set[12] ={
 //Jan   Feb   Mar   Apr   May   Jun   Jul   Aug   Sep   Oct   Nov   Dec
  1022, 1072, 1120, 1170, 1218, 1256, 1259, 1221, 1158, 1091, 1032, 1004};
 // 17:01, 17:51, 18:40, 19:30, 20:17, 20:56, 20:59, 20:20, 19:17, 18:11, 17:11, 16:43      - Times are GMT+1 no summer saving as clock isn't adjusted


// store the sunrise and sunset 'minutes after midnight'
  int sunRiseNow = 0;
  int sunSetNow = 0;
// store the clocktime (value in minutes)
  int clockTimeNow = 0;

// flag to indicate daytime or nighttime
  // 'nighTime = false' when it is day ; 'nighTime = true' when it is night
  bool nightTime = false;

// Security
// set flag 'Alarm = true' if runtimeLimit is exceeded and no switch is activated (both switches are SWITCH_NOT_ACTIVATED)
  bool Alarm = false;
// limit the runtime to protect damage when a switch is never activated
// runtimeLimit depends on the motorspeed, the diameter of the spool and the door eleavtion height.
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
 
    // // test print
    // Serial.print("Ok, Time = ");
    // print2digits(tm.Hour);
    // Serial.write(':');
    // print2digits(tm.Minute);
    // Serial.write(':');
    // print2digits(tm.Second);
    // Serial.print(", Date (D/M/Y) = ");
    // Serial.print(tm.Day);
    // Serial.write('/');
    // Serial.print(tm.Month);
    // Serial.write('/');
    // Serial.print(tmYearToCalendar(tm.Year));
    // Serial.println();

	
    nightTime = checkNightTime(tm); // check if it is nighttime OR daytime
    // // test print
	  // Serial.print("the value of nightTime is now ");
	  // Serial.println(nightTime);
    
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
	  // todo GOTO ALARM
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
	  // todo GOTO ALARM	
  }

 
  upperSwitchState = digitalRead(upperSwitch);
  lowerSwitchState = digitalRead(lowerSwitch);


  // if it is daytime AND the upperSwitchstate is SWITCH_NOT_ACTIVATED
  if (nightTime == false && upperSwitchState == SWITCH_NOT_ACTIVATED) {
    runMotor1Up();     // call function runMotor1Up()
	
	   // // test print
    // TEST Serial.print("runMotor1Up running :");
    // TEST Serial.println(upperSwitchState); 

  }  
  // else if it is nighttime AND the lowerSwitchstate is SWITCH_NOT_ACTIVATED
  else if (nightTime == true && lowerSwitchState == SWITCH_NOT_ACTIVATED) {
    runMotor1Down();     // call function runMotor1Down()

	  // // test print
    // Serial.print("runMotor1Down running :");
    // Serial.println(lowerSwitchState); 

  }
  else {
    runMotor1Stop(); // stop the motor

	   // // test print
    // Serial.print("runMotor1Up stopped :");
    // Serial.println(upperSwitchState);
    // Serial.print("runMotor1Down stopped :");
    // Serial.println(lowerSwitchState);
  }
}


bool checkNightTime(tmElements_t tm){
  
  sunRiseNow = sun_rise[(tm.Month -1)]; // ! (tm.Month-1) because the first element of the array is at index 0 (the array positions count from [0] to [11])
  sunSetNow = sun_set [(tm.Month -1)] + sunSetOffset;  //   // Add a safety offset time to avoid "locked-out" chickens.
  clockTimeNow = (tm.Hour-summerTimeOffset) * 60 + tm.Minute; // substract the summerTimeOffset (1 Hour) when the clock runs in summertime modus
    
  // //test print
  // Serial.print("sunRiseNow is ");  
  // Serial.print(sunRiseNow);
  // Serial.print("       sunSetNow is ");   
  // Serial.print(sunSetNow);
  // Serial.print("       clockTimeNow is ");  
  // Serial.println(clockTimeNow);

  bool nightTime = true; // it is nighttime
  if ((clockTimeNow >= sunRiseNow) && (clockTimeNow <= sunSetNow)){
    nightTime = false; // it is daytime if the clocktime is between sunrise and sunset
  }
  
  // // test print
  // Serial.print("the value of nightTime is now ");
  // Serial.println(nightTime);

  return nightTime;
}


// this function activates runMotor1Up
void runMotor1Up() {  
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2, LOW);
  Serial.println("Run Motor Up"); 
}


// this function activates runMotor1Down
void runMotor1Down() {  
  digitalWrite(IN1,LOW);
  digitalWrite(IN2, HIGH);
  Serial.println("Run Motor Down"); 
}


// this function stops motor1
void runMotor1Stop() {
   digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
 // test print
  Serial.println("Run Motor Stop");
 // TODO wait 60 seconden before returning to the main loop
 // delay(60000);
 // TEST delay(5000)
  delay(5000);  
}


/*
// print number as 2-digits in case number is only 1-digit 
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
*/
