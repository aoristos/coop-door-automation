/*
 * Open and close a chicken coop door, using an Arduino Uno, a MotorShield-L298N and a Real Time Clock DS1307RTC
 *
 * Version = 1.2.2
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
 *  <Bounce2.h> library ( https://github.com/thomasfredericks/Bounce2)
 *
 */


// ATTENTION-ADJUST: adjust appropriate offset parameters:

  /* because the sunRise[] and sunSet[] ephemerides are expressed all in wintertime ...
   * we have to subtract 1 hour from the clock-reading (tm.Hour - summerTimeOffset) ...
   * when the sketch is uploaded Arduino during the summer ...
   * because the DS1307RTC is then running in summertime modus (daylight saving time values)(zomertijd) ...
   * while the sunRise[] and sunSet[] ephemerides are expressed all in wintertime values ...
  */
// ATTENTION-ADJUST:
	byte summerTimeOffset = 1; // when the RTC clock-time is running in summer-time
	//byte summerTimeOffset = 0; // when the RTC clock-time is running in winter-time

// make sure you leave enough time for the chickens to go to bed
	// increase sunSetNow with a safety offset time (e.g. 60 minutes)
// ATTENTION-ADJUST:
	byte sunSetOffset = 60;

// do not wake up the chickens too early
	// increase sunRiseNow with a safety offset time (e.g. 10 minutes)
// ATTENTION-ADJUST:
	byte sunRiseOffset = 0;
  
// set the values for SWITCH_IS_ACTIVATED and SWITCH_NOT_ACTIVATED
	// the HIGH or LOW status depends on the switch type (NORMAL_OPEN or NORMAL_CLOSE), the wiring and the configuration of the switch INPUT PINS
// ATTENTION-ADJUST: uncomment the desired values here:
	//const bool SWITCH_IS_ACTIVATED = LOW; //= NormalClosed switch
	const bool SWITCH_IS_ACTIVATED = HIGH;  //= NormalOpen switch

// ATTENTION-ADJUST: uncomment the desired values here:
	//const bool SWITCH_NOT_ACTIVATED = HIGH;  //= NormalClosed switch
	const bool SWITCH_NOT_ACTIVATED = LOW;  //= NormalOpen switch
  
// Security:
// set flag 'Alarm = true' if runTimeLimit is exceeded and no switch is activated (both switches are SWITCH_NOT_ACTIVATED)
	bool Alarm;

// Security:
// limit the runtime to avoid the motor keep running forever when a switch is never activated 
	// (e.g. due to a blocked door or a detached rope ...)
	// runtime depends on the motorspeed, the diameter of the spool and the door elevation height.
// ATTENTION-ADJUST: 
		const byte DownTimeLimit = 210; // Security runtime limit for door closing cycle (appropriated for my situation).
  
// Security:
  // Any obstruction during the door closing may avoid activation of the lowerSwitch. 
	// The runTimeCounter will reach zero and this will trigger the Alarm flag. There could be some slack in the suspension rope.
	// So the door lifting may once again exceed the runTimeLimit, trigger the Alarm and never activate the upperSwitch.
	// Therefore the UpTimeLimit should be a little bit higher than the DownTimeLimit.
// ATTENTION-ADJUST: 
    const byte UpTimeLimit = 225; // Security runTime limit for door opening cycle.

// motorspeed security timecounter step delay regulator 
// Use a step-delay time regulation (in miliseconds) between each couter step
// (this value is appropriated for my coop door closing cycle)
   const byte stepDelay = 25;

// store today's sunrise and sunset 'minutes after midnight'
	//int sunRiseNow;
	//int sunSetNow;

// store the clocktime (value in minutes) after reading the DS1307RTC
	//int clockTimeNow;

// flag to indicate daytime or nightTime
	bool nightTime; // 'nightTime = false' when it is day ; 'nightTime = true' when it is night
// Use this boolean flag to detect if nightTime has changed  
	bool oldNightTime;

	// set flag to indicate when the button is pressed
	// set buttonPressedFlag = true when Button.pressed()== true;
	bool buttonPressedFlag;

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
  // Arduino pins for the switches
    const byte upperSwitch = 7;
    const byte lowerSwitch = 6;
  // Arduino pins for button
    const byte pressDownButton = 2;
  // use LEDS to indicate the program flow
    const byte nightLED = 11;
    const byte manualModeLED = 12;
    const byte alarmLED = 13;
  // use an output-pin to set nightTime (for test purposes) // FOR TEST_WITHOUT_CLOCK
  	const byte nightTimePin = 5; // FOR TEST_WITHOUT_CLOCK

// Arrays with the sunRise and sunSet ephemerides for each month
// Calculations for Belgium-Bellegem latitude 50,50° / longitude 3,16°  Timezone GMT +1  (no daylight saving time)
// Values for day 8 of every month (day 8 is roughly medium between the season pivots at day 21)
// Jan   Feb   Mar   Apr   May   Jun   Jul   Aug   Sep   Oct   Nov   Dec
// Values based on NOAA_Solar_Calculations_year ( https://gml.noaa.gov/grad/solcalc/calcdetails.html )

  int sun_rise[12]={ 
  //Jan   Feb   Mar   Apr   May   Jun   Jul   Aug   Sep   Oct   Nov   Dec
  526, 491, 436, 368, 310, 277, 286, 325, 372, 419, 470, 515};
  // 08:46, 08:11, 07:16, 06:08, 05:09, 04:36, 04:45, 05:25, 06:12, 06:58, 07:50, 08:34     - Times are GMT+1 ; no Daylight Saving Time (no summer time)
  
  int sun_set[12] ={
  //Jan   Feb   Mar   Apr   May   Jun   Jul   Aug   Sep   Oct   Nov   Dec
  1022, 1072, 1120, 1170, 1218, 1256, 1259, 1221, 1158, 1091, 1032, 1004};
  // 17:01, 17:51, 18:40, 19:30, 20:17, 20:56, 20:59, 20:20, 19:17, 18:11, 17:11, 16:43      - Times are GMT+1 ; no Daylight Saving Time (no summer time)


// -----------------------------------------------------------------------------------------------

void setup()
{
   
  //Serial.println("01.00 Dit is de functie void setup()"); // TEST_PRINT
  // delay(10000); // TEST_PRINT
  
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
  pinMode(nightTimePin, INPUT_PULLUP); // FOR TEST_WITHOUT_CLOCK
  // the nightTimePin can emulate the state of nightTime: HIGH=daytime, LOW=nighttime

  // Set the Bounce2 connection parameters
    // IF YOUR INPUT HAS AN INTERNAL PULL-UP
      button.attach( pressDownButton, INPUT_PULLUP ); // USE INTERNAL PULL-UP
    // DEBOUNCE INTERVAL IN MILLISECONDS
      button.interval(10); // interval in ms
    // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
      button.setPressedState(LOW); 

// dit is het einde van void setup()
  // Serial.println("01.99 Dit is het einde van void setup()"); // TEST_PRINT
  // delay(10000); // TEST_PRINT
  
}


// -----------------------------------------------------------------------------------------------

void loop()
{ 

	// Serial.println("02.00 Dit is de functie void loop()"); // TEST_PRINT
  // delay(10000); // TEST_PRINT

  // Print the version number
  Serial.print("version = 1.2.2");
  Serial.println(" - summertime");
 
  tmElements_t tm;

  if (RTC.read(tm)) {

		// Serial.println("02.10.11 if (RTC.read(tm)) then ..."); // TEST_PRINT
  	// delay(10000); // TEST_PRINT
 
    // Read the DS1307 RTC and print to the serial monitor
 
    Serial.print("02.10.11 Date (D/M/Y) = "); // TEST_PRINT
    Serial.print(tm.Day); // TEST_PRINT
    Serial.write('/'); // TEST_PRINT
    Serial.print(tm.Month); // TEST_PRINT
    Serial.write('/'); // TEST_PRINT
    Serial.println(tmYearToCalendar(tm.Year)); // TEST_PRINT
    Serial.print("02.10.11 Time = "); // TEST_PRINT
    print2digits(tm.Hour); // TEST_PRINT
    Serial.write(':'); // TEST_PRINT
    print2digits(tm.Minute); // TEST_PRINT
    //Serial.write(':'); // TEST_PRINT
    //print2digits(tm.Second); // TEST_PRINT
    Serial.println(); // TEST_PRINT
 

    // prepare Reset Alarm
    oldNightTime = nightTime;

		// check if it is nighttime OR daytime
    nightTime = checkNightTime(tm); // check if it is nighttime OR daytime

		// MODX // FOR TEST_WITHOUT_CLOCK
		// It is 'night' when nightTimePin = HIGH. It is 'day' when nightTimePin = LOW // FOR TEST_WITHOUT_CLOCK
    // nightTime = digitalRead(nightTimePin); // FOR TEST_WITHOUT_CLOCK

	   Serial.print("02.10.11 nightTime is now "); // TEST_PRINT
	   Serial.println(nightTime); // TEST_PRINT

    // Clear Alarm and buttonPressedFlag when nightTime has changed

    if (nightTime != oldNightTime) {

			Serial.println("02.10.11.11 nightTime has changed !"); // TEST_PRINT

      Alarm = false; // Reset Alarm
      digitalWrite(alarmLED, LOW); // Reset alarmLED (= LOW)
			// Serial.println("02.10.11.11 Alarm = false"); // TEST_PRINT
			// delay(10000); // TEST_PRINT

			// set buttonPressedFlag = false
			buttonPressedFlag = false; // Reset buttonPressedFlag
			// Serial.println("02.10.11.11 buttonPressedFlag = 'false'"); // TEST_PRINT
			// delay(10000); // TEST_PRINT

			// set 'manualModeLED = LOW' to indicate that the buttonPressedFlag is cleared
			digitalWrite(manualModeLED, LOW); //Reset manualModeLED
			// Serial.println("02.10.11.11 Set manualModeLED = 'LOW'"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

    }

	} 

	else if (RTC.chipPresent()) {

		// Serial.println("02.10.12 else if RTC is present but it has stopped"); // TEST_PRINT
  	// delay(10000); // TEST_PRINT
 
	  	Alarm = true;
      flashAlarm(300, 900);

      // Serial.println("02.10.12 The DS1307 is stopped.  Please run the SetTime");
      // Serial.println("         example to initialize the time and begin running.");
      // Serial.println();

    } 

		else {
			
			Alarm = true;

			// Serial.println("02.10.13 else ..."); // TEST_PRINT
			// delay(10000); // TEST_PRINT
 
			flashAlarm(300, 300);

			// Serial.println("02.10.13 DS1307 read error!  Please check the circuitry.");
			// Serial.println();
		}

//-----------------------------------------

	// Return to automatic mode when both upperSwitch AND lowerSwitch are activated
    if ((digitalRead(upperSwitch) == SWITCH_IS_ACTIVATED) && (digitalRead(lowerSwitch) == SWITCH_IS_ACTIVATED)) {

			// Serial.println("02.50.11 BOTH upperSwitch AND lowerSwitch are activated)"); // TEST_PRINT
			// delay(10000); // TEST_PRINT

			// reset buttonPressedFlag
			buttonPressedFlag = false;
			 Serial.print("02.50.11 buttonPressedFlag = "); // TEST_PRINT
			 Serial.println(buttonPressedFlag); // TEST_PRINT
  		 // delay(3000); // TEST_PRINT

      // set 'manualModeLED = LOW  to indicate that we are in automatic modus modus
			digitalWrite(manualModeLED, LOW); // reset manualModeLED (= LOW)
			 Serial.println("02.50.11 Set manualModeLED = 'LOW'"); // TEST_PRINT
  		 // delay(10000); // TEST_PRINT

		  // reset Alarmflag 
      Alarm = false; // Reset Alarm
			 Serial.println("02.50.11 Alarm = false"); // TEST_PRINT
  		 delay(3000); // TEST_PRINT

      // set 'alarmLed = LOW' to indicate that the buttonPressedFlag and the Alarmflag are cleared
      digitalWrite(alarmLED, LOW); // Reset alarmLED (= LOW)
			// Serial.println("02.50.11 Set alarmLED = 'LOW'"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

	}



//-----------------------------------------

	// Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
	button.update();

			// Serial.println("02.30 Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)"); // TEST_PRINT
	 		 Serial.print("02.30 Alarm = "); // TEST_PRINT
	 		 Serial.println(Alarm); // TEST_PRINT


	// set buttonPressedFlag each time when the button is pressed. 
	 if (button.pressed()) {

		 Serial.println("02.30.11 Button is pressed"); // TEST_PRINT
		// delay(10000); // TEST_PRINT
	
		// set buttonPressedFlag = true
		buttonPressedFlag = true;

		 Serial.println("02.30.11 buttonPressedFlag = 'true'"); // TEST_PRINT
		 delay(3000); // TEST_PRINT

		// set 'manualModeLED = HIGH' to indicate that the buttonPressedFlag is set and we are in manual modus
		digitalWrite(manualModeLED, HIGH);

	 }


//-----------------------------------------


	if (buttonPressedFlag == true) {

		  Serial.println("02.40.11 (buttonPressedFlag == 'true'"); // TEST_PRINT
		  delay(3000); // TEST_PRINT

    // if the door is fully open
    if (digitalRead(upperSwitch) == SWITCH_IS_ACTIVATED) {

			 Serial.println("02.40.11.11 upperSwitch is activated: close the door"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

      // upperSwitch is activated: close the door
      runMotor1Down(DownTimeLimit);

      // Alarm = true;
      // digitalWrite(alarmLED, HIGH);

			// Serial.println("02.40.11.11 status after runMotor1Down"); // TEST_PRINT
			// delay(10000); // TEST_PRINT

    } 

    // else only the lowerSwitch is activated OR none of the door switches are activated: open the door
		// else if (digitalRead(lowerSwitch) == SWITCH_IS_ACTIVATED ){
			else {

			 Serial.println("02.40.11.12 the door was closed or halfway: open the door"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

      // only the lowerSwitch is activated OR none of the door switches are activated: open the door
	    runMotor1Up(UpTimeLimit);

      // Alarm = true;
      // digitalWrite(alarmLED, HIGH);

			// Serial.println("02.40.11.12 status after runMotor1Up"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

    }

	//	buttonPressedFlag = false; // reset buttonPressedFlag after any SWITCH_IS_ACTIVATED

	}

// -----------------------------------------------------------------------------------------------

	// Only allow automatic motor activation when the buttonPressedFlag == false AND Alarm == false
  if (buttonPressedFlag == false && Alarm == false) {
			
		// Serial.println("02.60.11 (buttonPressedFlag == 'false AND Alarm == false'"); // TEST_PRINT
  	// delay(10000); // TEST_PRINT

    // run Motor1Up if it is daytime AND the upperSwitch is not activated
    if (nightTime == false && digitalRead(upperSwitch) == SWITCH_NOT_ACTIVATED) {

			 Serial.println("02.60.11.11 (nightTime == false AND upperSwitch == SWITCH_NOT_ACTIVATED)"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

      runMotor1Up(UpTimeLimit); // call function runMotor1Up(UpTimeLimit)

			// Serial.println("02.60.11.11 status after runMotor1Up"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

    }

    // else runMotor1Down if it is nightTime AND the lowerSwitch is not activated
    else if (nightTime == true && digitalRead(lowerSwitch) == SWITCH_NOT_ACTIVATED) {

			 Serial.println("02.60.11.12 else if (nightTime == true and lowerSwitch == SWITCH_NOT_ACTIVATED"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

      runMotor1Down(DownTimeLimit); // call function runMotor1Down(DownTimeLimit)

			// Serial.println("02.60.11.12 status after runMotor1Down"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT


    }

  	// else runMotor1Stop
    else {
    	 Serial.println("02.60.11.13 runMotorStop(}"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

      runMotor1Stop(); // stop the motor
      
			// Serial.println("02.60.11.13 status after runMotor1Stop"); // TEST_PRINT
  		// delay(10000); // TEST_PRINT

    }

  }

// -----------------------------------------------------------------------------------------------

   // Serial.print("02.60 Alarm = "); // TEST_PRINT
   // Serial.println(Alarm); // TEST_PRINT

	// dit is het einde van void loop()
   Serial.println("02.99 Dit is het einde van void loop()"); // TEST_PRINT 
	 Serial.println(); // TEST_PRINT

}


// -----------------------------------------------------------------------------------------------

// function to check if it's day or night
// this function uses the type 'tmElements_t' object 'tm' from the 'DS1307RTC.h' library
bool checkNightTime(tmElements_t tm) {

// finetuning sunRiseNow & sunSetNow per day
// int sunRiseNow = sun_rise[thisMonth] - ((sun_rise[thisMonth] - sun_rise[nextMonth]) * Today / 30) + sunRiseOffset;
// e.g sunRiseNow for 18 januari
// sunRiseNow = 526 - ((526 - 491) * 18 / 30) + 0

  byte Today = tm.Day;

  byte thisMonth = (tm.Month - 1);
  byte nextMonth = tm.Month;
  if (thisMonth == 11) {
      nextMonth = 0;
    }
  // ! thisMonth=(tm.Month-1) because the first element of an array is at index 0 (the array positions count from [0] to [11])
  int sunRiseNow = sun_rise[thisMonth] - ((sun_rise[thisMonth] - sun_rise[nextMonth]) * Today / 30) + sunRiseOffset;
  //(old algorithm) int sunRiseNow = sun_rise[(monthNumber -1)] + sunRiseOffset;  // Find the appropriate time in the array sun_rise[] and add a safety offset time (e.g. 30 minutes) to avoid a premature wake up.
  int sunSetNow = sun_set[thisMonth] - ((sun_set[thisMonth] - sun_set[nextMonth]) * Today / 30) + sunSetOffset;
  //(old algorithm) int sunSetNow = sun_set[(monthNumber -1)] + sunSetOffset;  // Find the appropriate time in the array sun_set[] and add a safety offset time (e.g. 60 minutes) to avoid "locked-out" chickens.

  int clockTimeNow = (tm.Hour-summerTimeOffset) * 60 + tm.Minute; // clockTimeNow in minutes: substract 1 Hour (summerTimeOffset=1) when the clock runs in summertime 

	// Serial.println("510.00 Dit is de functie checkNightTime"); // TEST_PRINT
/*
  Serial.print("510.10 sunRiseNow finetuning is "); // TEST_PRINT 
  Serial.println((sun_rise[thisMonth] - sun_rise[nextMonth]) * Today / 30); // TEST_PRINT
  Serial.print("510.10 sunSetNow finetuning is "); // TEST_PRINT 
  Serial.println((sun_set[thisMonth] - sun_set[nextMonth]) * Today / 30); // TEST_PRINT
  Serial.print("510.10 sunRiseNow is "); // TEST_PRINT 
  Serial.print(sunRiseNow); // TEST_PRINT
  Serial.print("       sunSetNow is "); // TEST_PRINT   
  Serial.print(sunSetNow); // TEST_PRINT
  Serial.print("       clockTimeNow is "); // TEST_PRINT  
  Serial.println(clockTimeNow); // TEST_PRINT
 */
// it is always nighttime except when the clocktime is between sunrise and sunset
  bool nightTime = true; // it is always nighttime except when the clocktime is between sunrise and sunset
  if ((clockTimeNow >= sunRiseNow) && (clockTimeNow <= sunSetNow)) {
    nightTime = false; // it is daytime because the clocktime is between sunrise and sunset
  }

		// Serial.print("510.11 nightTime = "); // TEST_PRINT
    // Serial.println(nightTime); // TEST_PRINT
		// delay(10000); // TEST_PRINT

  return nightTime;
}



// -----------------------------------------------------------------------------------------------

// this function activates runMotor1Up
void runMotor1Up(byte runTimeCounter) {  

  // Serial.println("520.00 Dit is de functie void runMotor1Up(UpTimeLimit)"); // TEST_PRINT
  // delay(10000); // TEST_PRINT

  // runMotor1Up as long as the upperSwitch is not activated
  while((digitalRead(upperSwitch) == SWITCH_NOT_ACTIVATED) && Alarm == false) {
    digitalWrite(motor1UP, HIGH);
    digitalWrite(motor1DOWN, LOW);

    Serial.print("runTimeCounter = "); // TEST_PRINT
    Serial.println(runTimeCounter); // TEST_PRINT

			 delay(stepDelay); 

    // Security
    runTimeCounter --;

    if (runTimeCounter <= 0) {
      Alarm = true;
      digitalWrite(alarmLED, HIGH);
      // run motor1Stop() as soon as the runTimeCounter is zero or below
      runMotor1Stop();
    }
    
		// Serial.print("Alarm = "); // TEST_PRINT
		// Serial.println(Alarm); // TEST_PRINT
		// delay(10000); // TEST_PRINT

  }

  // run motor1Stop() as soon as the upperSwitch is activated : the door is open.
  runMotor1Stop();

}



// -----------------------------------------------------------------------------------------------

// this function activates runMotor1Down
void runMotor1Down(byte runTimeCounter) {

  // Serial.println("530.00 Dit is de functie void runMotor1Down(DownTimeLimit)"); // TEST_PRINT
  // delay(10000); // TEST_PRINT

  // runMotor1Down as long as the lowerSwitch is not activated
  while((digitalRead(lowerSwitch) == SWITCH_NOT_ACTIVATED) && Alarm == false) {
    digitalWrite(motor1UP, LOW);
    digitalWrite(motor1DOWN, HIGH);

  	Serial.print("runTimeCounter = "); // TEST_PRINT
    Serial.println(runTimeCounter); // TEST_PRINT

		delay(stepDelay); 

    // Security
    runTimeCounter --;

    if (runTimeCounter <= 0) {
      Alarm = true;
      digitalWrite(alarmLED, HIGH);
      // run motor1Stop() as soon as the runTimeCounter is zero or below
      runMotor1Stop();
    }
    
		// Serial.print("Alarm = "); // TEST_PRINT
		// Serial.println(Alarm); // TEST_PRINT
		// delay(10000); // TEST_PRINT

  }

  // run motor1Stop() as soon as the lowerSwitch is activated : the door is closed.
  runMotor1Stop();

}



// -----------------------------------------------------------------------------------------------

// this function stops motor1
void runMotor1Stop() {
  
	// Serial.println("540.00 Dit is de functie void runMotor1Stop()"); // TEST_PRINT
	// delay(10000); // TEST_PRINT

  digitalWrite(motor1UP, LOW);
  digitalWrite(motor1DOWN, LOW);

}


// "flash" alarmLED
void flashAlarm(int LEDon, int LEDoff) {

	// Serial.println("550.00 Dit is de functie flashAlarm()"); // TEST_PRINT
	// delay(10000); // TEST_PRINT

		digitalWrite(alarmLED, HIGH);
		delay(LEDon);
		digitalWrite(alarmLED, LOW);
		delay(LEDoff);
}


// print number as 2-digits in case number is only 1-digit 
void print2digits(int number) {

 // Serial.println("560.00 Dit is de functie print2digits()"); // TEST_PRINT
 // delay(10000); // TEST_PRINT

  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
