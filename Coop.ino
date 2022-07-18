 /*
 * Open and close a chicken coop door, using an Arduino Uno, a MotorShield-L298N and a Real Time Clock DS1307RTC
 *
 * v3.0.0
 *
 * included libraries:
 *  <wire.h> the built-in library to communicate with the I2C bus
 *  <TimeLib.h> library ( https://github.com/PaulStoffregen/Time/releases )
 *  <DS1307RTC.h> library ( https://github.com/PaulStoffregen/DS1307RTC/releases )
 *  <Bounce2.h> library ( https://github.com/thomasfredericks/Bounce2)
 * 
 */

// ATTENTION-ADJUST: adjust appropriate offset parameters:

  // ATTENTION-ADJUST:
  // subtract 1 hour (tm.Hour - summerTimeOffset) when the DS1307RTC is running in summertime modus (daylight saving time values)(zomertijd)
  // because the sunRise[] and sunSet[] ephemerides are expressed all in wintertime
    byte summerTimeOffset = 1; // when the clock-time is running in summer-time
    // byte summerTimeOffset = 0; // when the clock-time is running in winter-time

  // ATTENTION-ADJUST: 
  // make sure you leave enough time for the chickens to go to bed
  // increase sunSetNow with a safety offset time (e.g. 60 minutes)
    byte sunSetOffset = 60;
  // ATTENTION-ADJUST: 
  // do not wake up the chickens too early
  //increase sunRiseNow with a safety offset time (e.g. 30 minutes)
    byte sunRiseOffset = 30;
  
  // set the values for SWITCH_IS_ACTIVATED and SWITCH_NOT_ACTIVATED
  // the HIGH or LOW status depends on the switch type (NORMAL_OPEN or NORMAL_CLOSE), the wiring and the configuration of the switch INPUT PINS
  // ATTENTION-ADJUST:
  // uncomment the desired values here:

    //const bool SWITCH_IS_ACTIVATED = LOW; //= NormalClosed switch
    const bool SWITCH_IS_ACTIVATED = HIGH;  //= NormalOpen switch

    //const bool SWITCH_NOT_ACTIVATED = HIGH;  //= NormalClosed switch
    const bool SWITCH_NOT_ACTIVATED = LOW;  //= NormalOpen switch
  
  // ATTENTION-ADJUST: 
    // limit the runtime to avoid keeping the motor running forever when a switch is never activated (e.g. due to a blocked door or a detached rope ...)
    // runtime depends on the motorspeed, the diameter of the spool and the door elevation height.
    const byte DownTimeLimit = 220; // Security runtime limit for door closing cycle (appropriated for my situation).
  
  // ATTENTION-ADJUST: 
    // increase the runTimeCounter with a extra buffer time
  // only apply this incremented runTimeCounter when lifting the door (runMotor1Up())
    // obstruction during the door closing will exceed the runTimeCounter, trigger the Alarm flag and may cause slack in the suspension rope. So the door lifting may once again exceed the runTimeLimit, trigger the Alarm and never activate the upperSwitch. Therefore the UptimeLimit should be a little bit higher than the DownTimeLimit.

    const byte UpTimeLimit = 230; // Security runTime limit for door opening cycle.
    int runTimeCounter;

// flags to indicate the state of the switches
  bool upperSwitchState; // 'upperSwitchState = SWITCH_IS_ACTIVATED' when the door is fully open
  bool lowerSwitchState; // 'lowerSwitchState = SWITCH_IS_ACTIVATED' when the door is fully closed

// store today's sunrise and sunset 'minutes after midnight'
  int sunRiseNow;
  int sunSetNow;
// store the clocktime (value in minutes) after reading the DS1307RTC
  int clockTimeNow;

// flag to indicate daytime or nighttime
// 'nightTime = false' when it is day ; 'nightTime = true' when it is night
  bool nightTime;
// Boolean flag to set when nightTime has changed  
  bool nightTimeHasChanged;


// set buttonPressedFlag when Button.pressed()== true;
  bool buttonPressedFlag;

// Security
// set flag 'Alarm = true' if runTimeLimit is exceeded and no switch is activated (both switches are SWITCH_NOT_ACTIVATED)
  bool Alarm;

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
    const byte motor1UP = 8; // MOTOR1 UP
    const byte motor1DOWN = 9; // MOTOR1 DOWN
  //Arduino pins for the switches
    const byte upperSwitch = 7;
    const byte lowerSwitch = 6;
  // Arduino pins for button
    const byte pressDownButton = 2;
  // use LEDS to indicate the program flow
    const byte nightLED = 11;
    const byte manualModeLED = 12;
    const byte alarmLED = 13;
  // use a output-pin to set nightTime (for test purposes) // FOR TEST_WITHOUT_CLOCK
  //  const byte nightTimePin = 5; // FOR TEST_WITHOUT_CLOCK

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

void setup()
{
   
// initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
// initialize pinmodes
  pinMode(motor1UP, OUTPUT);
  digitalWrite(motor1UP,LOW);
  pinMode(motor1DOWN, OUTPUT);
  digitalWrite(motor1DOWN,LOW);
  pinMode(upperSwitch, INPUT_PULLUP);
  pinMode(lowerSwitch, INPUT_PULLUP);
  pinMode(manualModeLED, OUTPUT);
  digitalWrite(manualModeLED,LOW);
  pinMode(nightLED, OUTPUT);
  digitalWrite(nightLED,LOW);
  pinMode(alarmLED, OUTPUT);
  digitalWrite(alarmLED,LOW);
  // pinMode(nightTimePin, INPUT_PULLUP); // FOR TEST_WITHOUT_CLOCK
  // the nightTimePin can emulate the state of nightTime: HIGH=daytime, LOW=nighttime

  // Set the Bounce2 connection parameters
    // IF YOUR INPUT HAS AN INTERNAL PULL-UP
      button.attach( pressDownButton, INPUT_PULLUP ); // USE INTERNAL PULL-UP
    // DEBOUNCE INTERVAL IN MILLISECONDS
      button.interval(10); // interval in ms
    // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
      button.setPressedState(LOW); 

    Alarm = false;

}


void loop()
{ 

  tmElements_t tm;

  if (RTC.read(tm)) {
 
 /*
    // Read the DS1307 RTC and print to the serial monitor
    Serial.print("Ok, Time = "); // TEST_PRINT
    print2digits(tm.Hour); // TEST_PRINT
    Serial.write(':'); // TEST_PRINT
    print2digits(tm.Minute); // TEST_PRINT
    // Serial.write(':'); // TEST_PRINT
    // print2digits(tm.Second); // TEST_PRINT
    Serial.print(", Date (D/M/Y) = "); // TEST_PRINT
    Serial.print(tm.Day); // TEST_PRINT
    Serial.write('/'); // TEST_PRINT
    Serial.print(tm.Month); // TEST_PRINT
    Serial.write('/'); // TEST_PRINT
    Serial.print(tmYearToCalendar(tm.Year)); // TEST_PRINT
    Serial.println(); // TEST_PRINT
    
*/	
    nightTime = checkNightTime(tm); // check if it is nighttime OR daytime
    
	  // Serial.print("the value of nightTime is now "); // TEST_PRINT
	  // Serial.println(nightTime); // TEST_PRINT
    
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
      flashAlarm(300, 1000);
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
      flashAlarm(500, 500);
    }
  }



// Reset Alarm to 'false' when nightTime has changed
  nightTimeHasChanged = nightTime;
  // nightTime = digitalRead(nightTimePin); // FOR TEST_WITHOUT_CLOCK
  if(nightTime != nightTimeHasChanged){
     Alarm = false;
  }
  
	  // Serial.print("nightTime = "); // FOR TEST_WITHOUT_CLOCK
	  // Serial.println(nightTime); // FOR TEST_WITHOUT_CLOCK



  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
  button.update();
  
  // Serial.print("buttonPressedFlag = "); // TEST_PRINT
  // Serial.println(buttonPressedFlag); // TEST_PRINT
  // Serial.println(digitalRead(pressDownButton)); // TEST_PRINT

  // <Button>.pressed() returns true if the state has changed and the current state matches <Button>.setPressedState(<HIGH or LOW>);
  // (which is low in this example  s set with <button.setPressedState(LOW)>; IN setup()

  if(button.pressed()) {

    // if the button is pressed AND both upperSwitch AND lowerSwitch are activated, then:
    // buttonPressedFlag is cleared and the program continues in automatic modus
    if((digitalRead(upperSwitch) == SWITCH_IS_ACTIVATED) && (digitalRead(lowerSwitch) == SWITCH_IS_ACTIVATED)){

      // if both upperSwitch AND lowerSwitch are activated, then:
      // reset buttonPressedFlag
      buttonPressedFlag = false;

      // reset Alarmflag 
      Alarm = false;

      // set 'alarmLed = LOW' to indicate that the buttonPressedFlag annd the Alarmflag are cleared
      // set 'manualModeLED = LOW  to indicate that we are in automatic modus modus
      digitalWrite(manualModeLED, LOW); 
   
      runMotor1Stop();
    }

    // else if the door is fully open
    else if(digitalRead(upperSwitch) == SWITCH_IS_ACTIVATED){

      // set 'manualModeLED = HIGH' to indicate that the pressedButtonFlag is set and we are in manual modus
      buttonPressedFlag = true;

      digitalWrite(manualModeLED, HIGH);

      // reset the runTimeCounter
      runTimeCounter = DownTimeLimit;

      // upperSwitch is activated: close the door
      runMotor1Down();
    }

    else{
      // only the lowerSwitch is activated OR none of the door switches are activated: open the door

      // set 'manualModeLED = HIGH' to indicate that the pressedButtonFlag is set and we are in manual modus
      buttonPressedFlag = true;

      // set 'manualModeLED = HIGH' to indicate that the pressedButtonFlag is set and we are in manual modus
      digitalWrite(manualModeLED, HIGH);

      // reset the runTimeCounter
      runTimeCounter = UpTimeLimit;
      // only the lowerSwitch is activated OR none of the door switches are activated: open the door
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

      runTimeCounter = UpTimeLimit; // reset the runTimeCounter
      runMotor1Up(); // call function runMotor1Up()

    } 

    // else runMotor1Down if it is nighttime AND the lowerSwitchstate is SWITCH_NOT_ACTIVATED
    else if (nightTime == true && lowerSwitchState == SWITCH_NOT_ACTIVATED) {

      runTimeCounter = DownTimeLimit; // reset the runTimeCounter
      runMotor1Down(); // call function runMotor1Down()

    }

  // else runMotor1Stop
    else {
      runMotor1Stop(); // stop the motor
      
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

  // ! (tm.Month-1) because the first element of an array is at index 0 (the array positions count from [0] to [11])
  sunRiseNow = sun_rise[(tm.Month -1)] + sunRiseOffset;  // Find the appropriate time in the array sun_rise[] and add a safety offset time (e.g. 30 minutes) to avoid a premature wake up.
  sunSetNow = sun_set [(tm.Month -1)] + sunSetOffset;  // Find the appropriate time in the array sun_set[] and add a safety offset time (e.g. 60 minutes) to avoid "locked-out" chickens.
  clockTimeNow = (tm.Hour-summerTimeOffset) * 60 + tm.Minute; // clockTimeNow in minutes: substract 1 Hour (summerTimeOffset) when the clock runs in summertime 

  /*
    Serial.print("sunRiseNow is "); // TEST_PRINT 
    Serial.print(sunRiseNow); // TEST_PRINT
    Serial.print("       sunSetNow is "); // TEST_PRINT   
    Serial.print(sunSetNow); // TEST_PRINT
    Serial.print("       clockTimeNow is "); // TEST_PRINT  
    Serial.println(clockTimeNow); // TEST_PRINT
  */
  
  bool nightTime = true; // it is nighttime except if the clocktime is between sunrise and sunset
  if ((clockTimeNow >= sunRiseNow) && (clockTimeNow <= sunSetNow)){
    nightTime = false; // it is daytime if the clocktime is between sunrise and sunset
  }

  return nightTime;
}


// this function activates runMotor1Up
void runMotor1Up() {  

  Serial.println("Dit is de functie void runMotor1Up()"); // TEST_PRINT
  // delay(3000); // TEST_PRINT


  // runMotor1Up as long as the upperSwitch is not activated
  while((digitalRead(upperSwitch) == SWITCH_NOT_ACTIVATED) && Alarm == false) {
    digitalWrite(motor1UP, HIGH);
    digitalWrite(motor1DOWN, LOW);

    // Security
    runTimeCounter --;

    if(runTimeCounter <= 0){
      Alarm = true;
      //buttonPressedFlag = true;
    }
    
  }

  // run motor1Stop() as soon as the upperSwitch is activated : the door is open.
  runMotor1Stop();

}


// this function activates runMotor1Down
void runMotor1Down() { 

  // runMotor1Down as long as the lowerSwitch is not activated
  while((digitalRead(lowerSwitch) == SWITCH_NOT_ACTIVATED) && Alarm == false) {
    digitalWrite(motor1UP, LOW);
    digitalWrite(motor1DOWN, HIGH);

    // Security
    runTimeCounter --;

    if(runTimeCounter <= 0){
      Alarm = true;
      //buttonPressedFlag = true;
    }
  
  }

  // run motor1Stop() as soon as the lowerSwitch is activated : the door is closed.
  runMotor1Stop();
  
}


// this function stops motor1
void runMotor1Stop() {
  
  digitalWrite(motor1UP, LOW);
  digitalWrite(motor1DOWN, LOW);

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
