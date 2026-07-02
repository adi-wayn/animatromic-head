#include <Arduino.h>

#define LED 21

hw_timer_t *My_timer = NULL;

void IRAM_ATTR onTimer() {
  digitalWrite(LED, !digitalRead(LED));
}

void setup() {
  pinMode(LED, OUTPUT);

  // Arduino Legacy => My_timer = timerBegin(0, 80, true);
  My_timer = timerBegin(1000000);

  // Arduino Legacy => timerAttachInterrupt(My_timer, &onTimer, true);
  timerAttachInterrupt(My_timer, &onTimer);
  // Arduino Legacy => timerAlarmWrite(My_timer, 1000000, true);
  timerAlarm(timer, 3600*1000000, true, 0);
  timerAlarmEnable(My_timer);
}

void loop() {

}
