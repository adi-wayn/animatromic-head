hw_timer_t *timer1 = NULL;
hw_timer_t *timer2 = NULL;

// Define distinct Interrupt Service Routines (ISRs)
void IRAM_ATTR onTimer1() {
  // Action for timer 1
}

void IRAM_ATTR onTimer2() {
  // Action for timer 2
}

void setup() {
  // Add Timer 1: 1 MHz frequency (1 tick = 1 microsecond)
  timer1 = timerBegin(1000000);
  timerAttachInterrupt(timer1, &onTimer1);
  timerAlarm(timer1, 500000, true, 0);  // Triggers every 500ms

  // Add Timer 2: 1 MHz frequency
  timer2 = timerBegin(1000000);
  timerAttachInterrupt(timer2, &onTimer2);
  timerAlarm(timer2, 1000000, true, 0); // Triggers every 1000ms
}

void loop() {}
