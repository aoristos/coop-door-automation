# coop-door-automation

Open and close a chicken coop door, using an Arduino Uno, a MotorShield-L298N and a Real Time Clock DS1307RTC

Hardware:
*	Arduino Uno
*	MotorShield-L298N
*	DS1307 Tiny RTC
*	12V motor with a reduction gearbox and a pully and a rope to lift the coop door
 
 included libraries:
*	<wire.h> the built-in library to communicate with the I2C bus
*	<TimeLib.h> library ( https://github.com/PaulStoffregen/Time/releases )
*	<DS1307RTC.h> library ( https://github.com/PaulStoffregen/DS1307RTC/releases )
*	<Bounce2.h> library ( https://github.com/thomasfredericks/Bounce2)

After our chickens were killed a second time by a fox, I decided to install a diy Arduino-controlled-motorized trap door.
The main function of this program is to close the chicken coop door at sunset and to open it at sunrise.
The Arduino controller uses a real-time-clock and an array with sunrise/sunset ephemerides to open/close the door at the right time.
You can use the NOAA_Solar_Calculations_year to calculate the appropriate values for your geographic location ( https://gml.noaa.gov/grad/solcalc/calcdetails.html )

An upperswitch and a lowerswitch are used to limit the course of the door. 
The door can activate either one of the two switches: the UPPERSWITCH is activated when the door is fully open, the LOWERSWITCH is activated when the door is fully closed.
	
The correct time-of-the-day has to set in the "DS1307 Tiny RTC"  before running this Arduino sketch. 

Some variables need to be adjusted once before the sketch is uploaded to the Arduino Uno.
They are indicated with // ATTENTION-ADJUST in the code:

summerTimeOffset:
The ephemerides in the array are expressed according UTC (Greenwich Meantime)(no Daylight Saving Time).
Subtract1 hour from the time reading (tm.Hour) in case the DS1307 RTC time was initialized in Daylight Saving Time (Summer Time) 

sunSetOffset:
make sure you leave enough time for the chickens to go to bed
increase sunSetNow with a safety offset time (e.g. 60 minutes)

sunRiseOffset:
do not wake up the chickens too early
increase sunRiseNow with a safety offset time (e.g. 30 minutes)

SWITCH_IS_ACTIVATED /  SWITCH_NOT_ACTIVATED:
the HIGH or LOW status depends on the switch type (NORMAL_OPEN or NORMAL_CLOSE) and on the wiring and the configuration of the switch INPUT PINS.

DownTimeLimit:
limit the runtime to avoid keeping the motor running forever when a switch is never activated (e.g. due to a blocked door or a detached rope ...)
runtime depends on the motorspeed, the diameter of the spool and the door elevation height.
Set a security runtime limit for door closing cycle (appropriated for your situation).

UpTimeLimit:
Sometimes an obstruction during the door closing event may may cause some slack in the suspension rope.
Therefore the UptimeLimit should be a little bit higher than the DownTimeLimit.

 
 
further documentation:
*	"Using a Real Time Clock with Arduino" ( https://dronebotworkshop.com/real-time-clock-arduino/ )
*	"Controlling DC Motors with the L298N H Bridge and Arduino" ( https://dronebotworkshop.com/dc-motors-l298n-h-bridg>
*	"The L298N H-bridge motor controller module - basics" ( https://www.youtube.com/watch?v=-ikmDMW6tEw )
