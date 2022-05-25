 /*
 * Open and close a chicken cooop door, using an Arduino Uno, a MotorShield-L298N and a Real Time Clock DS1307RTC
 * 
*version 2.2.0
 * debug: check Alarm-conditon in the while-loops: code O.K.
 * synchronize codes coop.ino  v2.1.1 with 'cooptest.ino' v2.1.1
 * version 2.1.1
 * add a runTimeCounter and a runTimeLimit to avoid damage when the switches fail 
 * version 2.0.1
 * add a pressDownButton and a buttonPressedFlag to create manual modus
 * (to set the buttonPressedFlag: press the pressDownButton (= switch to manual modus))
 * (to clear the buttonPressedFlag: press the pressDownButton while the upperSwitch AND the lowerSwitch are manually activated. (= switch to automatic modus (using the DS1307RTC and the sunRise[] and sunSet[] arrays)))
 *
 * documentation:
 *  "Using a Real Time Clock with Arduino" ( https://dronebotworkshop.com/real-time-clock-arduino/ )
 *  "Controlling DC Motors with the L298N H Bridge and Arduino" ( https://dronebotworkshop.com/dc-motors-l298n-h-bridge/ )
 *  "The L298N H-bridge motor controller module - basics" ( https://www.youtube.com/watch?v=-ikmDMW6tEw )
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
 * <Bounce2.h> library ( https://github.com/thomasfredericks/Bounce2)
 * 
 */

// ATTENTION: adjust appropriate offset parameters:

  // ATTENTION:
  // subtract 1 hour (tm.Hour - summerTimeOffset) when the DS1307RTC is running in summertime modus (daylight saving time values)(zomertijd)
    byte summerTimeOffset = 1; // when the clock-time is equal with the summer-time
    // byte summerTimeOffset = 0; // when the clock-time is equal with the winter-time

  // ATTENTION: 
  // make sure you leave enough time for the chickens to go to bed
  // increase sunSetNow with a safety offset time (e.g. 60 minutes)
    byte sunSetOffset = 60;
  // ATTENTION: 
  // do not wake up the chickens too early
  //increase sunRiseNow with a safety offset time (e.g. 30 minutes)
    byte sunRiseOffset = 30;
  
  // set the values for SWITCH_IS_ACTIVATED and SWITCH_NOT_ACTIVATED
  // the HIGH or LOW status depends on the switch type (NORMAL_OPEN or NORMAL_CLOSE), the wiring and the configuration of the switch INPUT PINS
  // ATTENTION:
  // uncomment the desired values here:

    //const bool SWITCH_IS_ACTIVATED = LOW; //= NormalClosed switch
    const bool SWITCH_IS_ACTIVATED = HIGH;  //= NormalOpen switch

    //const bool SWITCH_NOT_ACTIVATED = HIGH;  //= NormalClosed switch
    const bool SWITCH_NOT_ACTIVATED = LOW;  //= NormalOpen switch
  
// ATTENTION: 
// limit the runtime to protect damage when a switch is never activated
// runTimeLimit depends on the motorspeed, the diameter of the spool and the door elevation height.
  const byte runTimeLimit = 200; // Security runtime limit

  int runTimeCounter;

// flags to indicate the state of the switches
  bool upperSwitchState; // 'upperSwitchState = SWITCH_IS_ACTIVATED' when the door is fully open
  bool lowerSwitchState; // 'lowerSwitchState = SWITCH_IS_ACTIVATED' when the door is fully closed

// include libraries
  // built-in library <wire.h> serves the communication with the I2C bus
    #include <Wire.h>
  // TimeLib.h (author Paul Stoffregen)
  // https://github.com/PaulStoffregen/Time/releases
    #include <TimeLib.h>
  //DS1307RTC.h (author Paul Stoffregen)
  // https://github.com/PaulStoffregen/DS1307RTC/releases
    #include <DS1307RTC.h>
  // https://github.com/thomasfredericks/Bounce2
    #include <Bounce2.h>

  // INSTANTIATE A Button OBJECT FROM THE Bounce2 NAMESPACE
    Bounce2::Button button = Bounce2::Button();

// Arduino pin configuration  
  // Arduino pins for motor 1 
    const byte IN1 = 8; // MOTOR1 UP
    const byte IN2 = 9; // MOTOR1 DOWN
  //Arduino pins for the switches
    const byte upperSwitch = 7;
    const byte lowerSwitch = 6;
  // Arduino pins for button
    const byte pressDownButton = 2;  
  // use LEDS to indicate the program flow
    const byte nightLED = 11;
    const byte manualModeLED = 12;
    const byte alarmLED = 13;
  // use a output-pin to set nightTime (for test purposes) // TEST_WITHOUT_CLOCK
  //  const byte nightTimePin = 5; // TEST_WITHOUT_CLOCK

// Arrays with the sunRise and sunSet ephemerides for each month
// Calculations for Belgium-Bellegem latitude 50,50° / longitude 3,16°  Timezone GMT +1  (no daylight saving time)
// Values for day 8 of every month (day 8 is roughly medium between the season pivots at day 21)
// Jan   Feb   Mar   Apr   May   Jun   Jul   Aug   Sep   Oct   Nov   Dec
// Values based on NOAA_Solar_Calculations_year ( https://gml.noaa.gov/grad/solcalc/calcdetails.html )

  int sun_rise[12]={ 
  526, 491, 436, 368, 310, 277, 286, 325, 372, 419, 470, 515};
  // 08:46, 08:11, 07:16, 06:08, 05:09, 04:36, 04:45, 05:25, 06:12, 06:58, 07:50, 08:34     - Times are GMT+1 no summer saving as clock isn't adjusted
  
  int sun_set[12] ={
  //Jan   Feb   Mar   Apr   May   Jun   Jul   Aug   Sep   Oct   Nov   Dec
  1022, 1072, 1120, 1170, 1218, 1256, 1259, 1221, 1158, 1091, 1032, 1004};
  // 17:01, 17:51, 18:40, 19:30, 20:17, 20:56, 20:59, 20:20, 19:17, 18:11, 17:11, 16:43      - Times are GMT+1 no summer saving as clock isn't adjusted

// store today's sunrise and sunset 'minutes after midnight'
  int sunRiseNow;
  int sunSetNow;
// store the clocktime (value in minutes) after reading the DS1307RTC
  int clockTimeNow;

// flag to indicate daytime or nighttime
// 'nighTime = false' when it is day ; 'nighTime = true' when it is night
  bool nightTime;


// set buttonPressedFlag when Button.pressed()== true;
  bool buttonPressedFlag;

// Security
// set flag 'Alarm = true' if runTimeLimit is exceeded and no switch is activated (both switches are SWITCH_NOT_ACTIVATED)
  bool Alarm;

void setup()
{
 
  // // Serial.println("Dit is de functie void setup()"); // TEST_PRINT
  // delay(3000); // TEST_PRINT
  
// initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
// initialize pinmodes
  pinMode(IN1, OUTPUT);
  digitalWrite(IN1,LOW);
  pinMode(IN2, OUTPUT);
  digitalWrite(IN2,LOW);
  pinMode(upperSwitch, INPUT_PULLUP);
  pinMode(lowerSwitch, INPUT_PULLUP);
  pinMode(manualModeLED, OUTPUT);
  digitalWrite(manualModeLED,LOW);
  pinMode(nightLED, OUTPUT);
  digitalWrite(nightLED,LOW);
  pinMode(alarmLED, OUTPUT);
  digitalWrite(alarmLED,LOW);
  //pinMode(nightTimePin, INPUT_PULLUP); // TEST_WITHOUT_CLOCK

// Set the Bounce2 connection parameters
  // IF YOUR INPUT HAS AN INTERNAL PULL-UP
    button.attach( pressDownButton, INPUT_PULLUP ); // USE INTERNAL PULL-UP
  // DEBOUNCE INTERVAL IN MILLISECONDS
    button.interval(10); // interval in ms
  // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
    button.setPressedState(LOW); 

  Alarm = false;
  
  runTimeCounter = runTimeLimit;

}


void loop()
{ 

  // // Serial.println("Dit is de functie void loop()"); // TEST_PRINT
  // delay(3000); // TEST_PRINT

  tmElements_t tm;

  if (RTC.read(tm)) {
 
 /*
    // Serial.print("Ok, Time = "); // TEST_PRINT
    print2digits(tm.Hour); // TEST_PRINT
    Serial.write(':'); // TEST_PRINT
    print2digits(tm.Minute); // TEST_PRINT
    // Serial.write(':'); // TEST_PRINT
    // print2digits(tm.Second); // TEST_PRINT
    // Serial.print(", Date (D/M/Y) = "); // TEST_PRINT
    // Serial.print(tm.Day); // TEST_PRINT
    Serial.write('/'); // TEST_PRINT
    // Serial.print(tm.Month); // TEST_PRINT
    Serial.write('/'); // TEST_PRINT
    // Serial.print(tmYearToCalendar(tm.Year)); // TEST_PRINT
    // Serial.println(); // TEST_PRINT
*/	
    nightTime = checkNightTime(tm); // check if it is nighttime OR daytime
    
	  // // Serial.print("the value of nightTime is now "); // TEST_PRINT
	  // // Serial.println(nightTime); // TEST_PRINT
    
  } else {
    if (RTC.chipPresent()) {
      // Serial.println("The DS1307 is stopped.  Please run the SetTime");
      // Serial.println("example to initialize the time and begin running.");
      // Serial.println();
      flashAlarm(300, 1000);
    } else {
      // Serial.println("DS1307 read error!  Please check the circuitry.");
      // Serial.println();
      flashAlarm(500, 500);
    }
  }




  // nightTime = digitalRead(nightTimePin); // TEST_WITHOUT_CLOCK
  // Serial.print("nightTime = "); // TEST_WITHOUT_CLOCK
  // Serial.println(nightTime); // TEST_WITHOUT_CLOCK

 /* 
  // signal LEDs tos show the nightTime value
  if (nightTime) {
    // it is nighttime
    digitalWrite(manualModeLED, LOW);    
    digitalWrite(nightLED, HIGH);
  }
  else {
    // it is daytime
    digitalWrite(nightLED, LOW);    
    digitalWrite(manualModeLED, HIGH);    
  }
*/

  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
    button.update();
  
    // Serial.print("buttonPressedFlag = "); // TEST_PRINT
    // Serial.println(buttonPressedFlag); // TEST_PRINT
    // Serial.println(digitalRead(pressDownButton)); // TEST_PRINT

  // <Button>.pressed() RETURNS true IF THE STATE CHANGED
  // AND THE CURRENT STATE MATCHES <Button>.setPressedState(<HIGH or LOW>);
  // WHICH IS LOW IN THIS EXAMPLE AS SET WITH button.setPressedState(LOW); IN setup()

  if(button.pressed()) {

    if((digitalRead(upperSwitch) == SWITCH_IS_ACTIVATED) && (digitalRead(lowerSwitch) == SWITCH_IS_ACTIVATED)){
      // (manually) activate both upperSwitch AND lowerSwitch to clear the buttonPressedFlag
      buttonPressedFlag = false;
      Alarm = false;   
      // set 'alarmLed = LOW' to indicate that the pressedButtonFlag is cleared
      digitalWrite(alarmLED, LOW);
      // set 'manualModeLED = LOW  to indicate that we are in automatic modus modus
      digitalWrite(manualModeLED, LOW); 
      // reset the runTimeCounter
      runTimeCounter = runTimeLimit;    
        // Serial.print("2) buttonPressedFlag = "); // TEST_PRINT
        // Serial.println(buttonPressedFlag); // TEST_PRINT
      runMotor1Stop();
    }

    else if(digitalRead(upperSwitch) == SWITCH_IS_ACTIVATED){
      // upperSwitch is activated: close the door
      buttonPressedFlag = true;
      // set 'manualModeLED = HIGH' to indicate that the pressedButtonFlag is set and we are in manual modus
      digitalWrite(manualModeLED, HIGH);
      // Serial.print("1) buttonPressedFlag = "); // TEST_PRINT
      // Serial.println(buttonPressedFlag); // TEST_PRINT 

      // reset the runTimeCounter
      runTimeCounter = runTimeLimit;
      runMotor1Down();
    }
          // Activate both upperSwitch AND lowerSwitch to clear the buttonPressedFlag
    else{
      // only the lowerSwitch is activated OR none of the door switches are activated: open the door
      buttonPressedFlag = true;
      // set 'manualModeLED = HIGH' to indicate that the pressedButtonFlag is set and we are in manual modus
      digitalWrite(manualModeLED, HIGH);
        // Serial.print("3) buttonPressedFlag = "); // TEST_PRINT
        // Serial.println(buttonPressedFlag); // TEST_PRINT

      // reset the runTimeCounter
      runTimeCounter = runTimeLimit;
      runMotor1Up();
    }

  }

  // read the state of the upperSwitch and place it in the variable upperSwitchState
    upperSwitchState = digitalRead(upperSwitch);
  // read the state of the lowerSwitch and place it in the variable lowerSwitchState
    lowerSwitchState = digitalRead(lowerSwitch);

  /*
   * Avoid the normal open-close routine when in manual modus (when the button is pressed)
   * Only allow motor activation when the buttonPressedFlag == false
   */
  if(buttonPressedFlag == false){

    // run Motor1Up if it is daytime AND the upperSwitchstate is SWITCH_NOT_ACTIVATED
    if (nightTime == false && upperSwitchState == SWITCH_NOT_ACTIVATED) {
      runMotor1Up();     // call function runMotor1Up()
      // reset the runTimeCounter when runTimeLimit has not been exceeded
      runTimeCounter = runTimeLimit;

      // // Serial.print("runMotor1Up running :"); // TEST_PRINT
      // // Serial.println(upperSwitchState); // TEST_PRINT

    } 

    // else runMotor1Down if it is nighttime AND the lowerSwitchstate is SWITCH_NOT_ACTIVATED
    else if (nightTime == true && lowerSwitchState == SWITCH_NOT_ACTIVATED) {
      runMotor1Down();     // call function runMotor1Down()
      // reset the runTimeCounter when runTimeLimit has not been exceeded
      runTimeCounter = runTimeLimit;

      // // Serial.print("runMotor1Down running :"); // TEST_PRINT
      // // Serial.println(lowerSwitchState); // TEST_PRINT

    }

  // else runMotor1Stop
    else {
      runMotor1Stop(); // stop the motor
      // reset the runTimeCounter
      runTimeCounter = runTimeLimit;

      // // Serial.print("runMotor1Up stopped :"); // TEST_PRINT
      // // Serial.println(upperSwitchState); // TEST_PRINT
      // // Serial.print("runMotor1Down stopped :"); // TEST_PRINT
      // // Serial.println(lowerSwitchState); // TEST_PRINT
    }
  }

  Serial.print("Alarm = "); // TEST_PRINT
  Serial.println(Alarm); // TEST_PRINT
  if(Alarm == true){
    flashAlarm(100,100);
  }
}

// function to check if it's day or night
// this function uses the type 'tmElements_ts' object 'tm' from the 'DS1307RTC.h' library
bool checkNightTime(tmElements_t tm){

  
  // // Serial.println("Dit is CheckNighTime()");
  // delay(3000);

  // ! (tm.Month-1) because the first element of an array is at index 0 (the array positions count from [0] to [11])
  sunRiseNow = sun_rise[(tm.Month -1)] + sunRiseOffset;  // Find the appropriate time in the array sun_rise[] and add a safety offset time (e.g. 30 minutes) to avoid a premature wake up.
  sunSetNow = sun_set [(tm.Month -1)] + sunSetOffset;  // Find the appropriate time in the array sun_set[] and add a safety offset time (e.g. 60 minutes) to avoid "locked-out" chickens.
  clockTimeNow = (tm.Hour-summerTimeOffset) * 60 + tm.Minute; // clockTimeNow in minutes: substract 1 Hour (summerTimeOffset) when the clock runs in summertime 

  /*
    // Serial.print("sunRiseNow is "); // TEST_PRINT 
    // Serial.print(sunRiseNow); // TEST_PRINT
    // Serial.print("       sunSetNow is "); // TEST_PRINT   
    // Serial.print(sunSetNow); // TEST_PRINT
    // Serial.print("       clockTimeNow is "); // TEST_PRINT  
    // Serial.println(clockTimeNow); // TEST_PRINT
  */
  
  bool nightTime = true; // it is nighttime except if the clocktime is between sunrise and sunset
  if ((clockTimeNow >= sunRiseNow) && (clockTimeNow <= sunSetNow)){
    nightTime = false; // it is daytime if the clocktime is between sunrise and sunset
  }
    
  // // Serial.print("the value of nightTime is now "); // TEST_PRINT
  // // Serial.println(nightTime); // TEST_PRINT

  return nightTime;
}


// this function activates runMotor1Up
void runMotor1Up() {  

  
    // Serial.println("Dit is de functie void runMotor1Up()"); // TEST_PRINT
    // delay(3000); // TEST_PRINT

  // runMotor1Up as long as the upperSwitch is not activated
  while((digitalRead(upperSwitch) == SWITCH_NOT_ACTIVATED) && Alarm == false) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    // Security
    runTimeCounter --;
    // Serial.print("runTimeCounter = "); // TEST_PRINT
    // Serial.println(runTimeCounter); // TEST_PRINT
    if(runTimeCounter < 0){
      Alarm = true;
      //buttonPressedFlag = true;     
    }    

    //// Serial.println("Dit is de while() loop in runMotor1Up()"); // TEST_PRINT
    // delay(3000); // TEST_PRINT
  
  }

  // run motor1Stop() as soon as the upperSwitch is activated : the door is open.
  runMotor1Stop();
}


// this function activates runMotor1Down
void runMotor1Down() { 

  //// Serial.println("Dit is de functie void runMotor1Down()"); // TEST_PRINT
  // delay(3000); // TEST_PRINT

  // runMotor1Up as long as the lowerSwitch is not activated
  while((digitalRead(lowerSwitch) == SWITCH_NOT_ACTIVATED) && Alarm == false) { 
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    // Security
    runTimeCounter --;
    // Serial.print("runTimeCounter = "); // TEST_PRINT
    // Serial.println(runTimeCounter); // TEST_PRINT
    if(runTimeCounter < 0){
      Alarm = true;
      //buttonPressedFlag = true;  
    }    

    // Serial.println("Dit is de while() loop in runMotor1Down()"); // TEST_PRINT
    // delay(3000); // TEST_PRINT
  
  }

  // run motor1Stop() as soon as the lowerSwitch is activated : the door is closed.
  runMotor1Stop();  
}


// this function stops motor1
void runMotor1Stop() {

  // Serial.println("Dit is de functie void runMotor1Stop()"); // TEST_PRINT
  // delay(3000); // TEST_PRINT
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

}

// "flash" alarmLED
void flashAlarm(int LEDon, int LEDoff) {
      digitalWrite(alarmLED, HIGH);
      delay(LEDon);
      digitalWrite(alarmLED, LOW);
      delay(LEDoff);
}


// print number as 2-digits in case number is only 1-digit 
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
