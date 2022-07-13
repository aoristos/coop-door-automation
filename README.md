# coop-door-automation

 * Open and close a chicken coop door, using an Arduino Uno, a MotorShield-L298N and a Real Time Clock DS1307RTC

 * version 3.0.0
 *      eliminate the blocking effect of the Alarm-status
 * version 2.3.0
 *      replace variable 'runTimeLimit' with dedicated variables 'UpTimeLimit' and 'DownTimeLimit'
 * version 2.2.2
 *      increase the runTimeCounter with a safety margin when the door opens (runMotor1Up()) to eliminate slack in the rope
 * version 2.2.1
 *      reset Alarm (Alarm = false) when nightTime Changes
 * version 2.2.0
 *      debug: check Alarm-conditon in the while-loops: code O.K.
 *      synchronize codes coop.ino  v2.1.1 with 'cooptest.ino' v2.1.1
 * version 2.1.1
 *      add a runTimeCounter and a runTimeLimit to avoid damage when the switches fail
 * version 2.0.1
 *      add a pressDownButton and a buttonPressedFlag to create manual modus
 *      (to set the buttonPressedFlag: press the pressDownButton (= switch to manual modus))
 *      (to clear the buttonPressedFlag: press the pressDownButton while the upperSwitch AND the lowerSwitch are manua>
 *
 * documentation:
 *  "Using a Real Time Clock with Arduino" ( https://dronebotworkshop.com/real-time-clock-arduino/ )
 *  "Controlling DC Motors with the L298N H Bridge and Arduino" ( https://dronebotworkshop.com/dc-motors-l298n-h-bridg>
 *  "The L298N H-bridge motor controller module - basics" ( https://www.youtube.com/watch?v=-ikmDMW6tEw )
 *
 *
 * Hardware:
 *  Arduino Uno
 *  MotorShield-L298N
 *  DS1307 Tiny RTC
 *  12V motor
 *
 *  
 * included libraries:
 *  <wire.h> the built-in library to communicate with the I2C bus
 *  <TimeLib.h> library ( https://github.com/PaulStoffregen/Time/releases )
 *  <DS1307RTC.h> library ( https://github.com/PaulStoffregen/DS1307RTC/releases )
 *  <Bounce2.h> library ( https://github.com/thomasfredericks/Bounce2)
 * 

