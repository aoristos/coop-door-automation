/*
 * Code for MotorShield-L298N
 * "D:\Documents\Mijn PaperPort-documenten\Elektronica\Arduino\Shields\MotorShield-L298N-handleiding.pdf"
 * Sample code using functions motorUp() and motorDown()
 */

// motor 1
byte EN1 = 6; // PWM
byte IN1 = 7; // MOTOR1 UP
byte IN2 = 8; // MOTOR1 DOWN
int Runtime = 3000; // Security Runtime Limit



void setup()
{
  //initialize serial monitor
  Serial.begin(9600); 
  // initialize pinmide
  pinMode(EN1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop()
{
  
  // call function motorUp()
  motor1Up(Runtime);
  delay (Runtime);

  motor1Down(Runtime);
  delay (Runtime);
}


// void functions do not return values
// this function activates motor1Up
void motor1Up(int Runtime)
{  
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2, LOW);
    delay (Runtime);
    digitalWrite(IN1, LOW);    
}

// void functions do not return values
// this function activates motor1Down
void motor1Down(int Runtime)
{  
    digitalWrite(IN1,LOW);
    digitalWrite(IN2, HIGH);
    delay (Runtime);
    digitalWrite(IN2, LOW);    
}
