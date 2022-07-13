 version 2.2.3
	replace variable 'runTimeLimit' with dedicated variables 'UpTimeLimit' and 'DownTimeLimit'
 version 2.2.2
	increase the runTimeCounter with a safety margin when the door opens (runMotor1Up()) to eliminate slack in the rope
 version 2.2.1
	reset Alarm (Alarm = false) when nightTime Changes
 version 2.2.0
	debug: check Alarm-conditon in the while-loops: code O.K.
	synchronize codes coop.ino  v2.1.1 with 'cooptest.ino' v2.1.1
 version 2.1.1
	add a runTimeCounter and a runTimeLimit to avoid damage when the switches fail
 version 2.0.1
	add a pressDownButton and a buttonPressedFlag to create manual modus
	(to set the buttonPressedFlag: press the pressDownButton (= switch to manual modus))
	(to clear the buttonPressedFlag: press the pressDownButton while the upperSwitch AND the lowerSwitch are manually activated)

