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
* 
After our chickens were killed a second time by a fox, I decided to install a diy Arduino-controlled-motorized trap door.
The main function of the program is to close the chicken coop door at sunset and to open it at sunrise.
The Arduino controller uses a real-time-clock and an array with sunrise/sunset ephemerides to open/close the door at the right time.
You can use the NOAA_Solar_Calculations_year to calculate the appropriate values for your geographic location ( https://gml.noaa.gov/grad/solcalc/calcdetails.html )

 
further documentation:
*	"Using a Real Time Clock with Arduino" ( https://dronebotworkshop.com/real-time-clock-arduino/ )
*	"Controlling DC Motors with the L298N H Bridge and Arduino" ( https://dronebotworkshop.com/dc-motors-l298n-h-bridg>
*	"The L298N H-bridge motor controller module - basics" ( https://www.youtube.com/watch?v=-ikmDMW6tEw )
*
*
