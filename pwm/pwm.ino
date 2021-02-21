// pwm.ino - Sketch to test PWM

// Flash LED settings
#define FLED_PIN         4    // The GPIO pin for the high-power LED.
#define FLED_CHANNEL     0    // I just picked first PWM channel (of the 16).
#define FLED_FREQUENCY   4096 // Some arbirary PWM frequemcy, high enough to not see it.
#define FLED_RESOLUTION  8    // 8 bit resolution for the duty-cycle.

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\n\n\n");
  Serial.printf("Welcome to pwm\n");

  ledcSetup(FLED_CHANNEL, FLED_FREQUENCY, FLED_RESOLUTION); // Setup a PWM channel
  ledcAttachPin(FLED_PIN, FLED_CHANNEL); // Route the PWM channel to the LED pin

  for( int duty=0; duty<(1<<FLED_RESOLUTION); duty+=8 ) {
    Serial.printf("duty=%d/255\n",duty);
    ledcWrite(FLED_CHANNEL, duty);
    while( Serial.available()==0 ) /*skip*/;
    while( Serial.available()>0 ) Serial.read();
  }
}

void loop() {
  Serial.printf("done\n");
  delay(5000);
}
