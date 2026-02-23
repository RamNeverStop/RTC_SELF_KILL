#include "RTClib.h"

RTC_DS3231 rtc;
#define DONE 1
#define CLOCK_INTERRUPT_PIN 2

char daysOfTheWeek[7][12] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

// ✅ Declare the ISR function first!
void IRAM_ATTR onAlarm() {
  Serial.println("Alarm triggered! INT/SQW pin pulled LOW.");
  rtc.clearAlarm(1);  // Clear Alarm1 → INT/SQW goes HIGH again
}

void setup() {
  Serial.begin(115200);
  pinMode(DONE, OUTPUT);

  digitalWrite(DONE, HIGH);
  Serial.println("");
  Serial.println("");
  Serial.println("power up");
  Serial.println("");
  Serial.println("");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1) delay(10);
  }

  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  delay(1000);

  rtc.writeSqwPinMode(DS3231_OFF);
  delay(1000);

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // ✅ Set Alarm1: trigger in 30 sec
  DateTime now = rtc.now();
  DateTime alarmTime = now + TimeSpan(0, 0, 0, 30); // 30 seconds later

  rtc.setAlarm1(
    alarmTime,
    DS3231_A1_Second
  );
  

  Serial.print("Alarm 1 set for: ");
  Serial.print(alarmTime.hour()); Serial.print(":");
  Serial.print(alarmTime.minute()); Serial.print(":");
  Serial.println(alarmTime.second());

  // ✅ INT/SQW will be pulled LOW when alarm matches

  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  Serial.println("FALLING start ");
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
  Serial.println("FALLING end");
  Serial.println("");
  Serial.println("");
  Serial.println("low kill pin");
  delay(1000);
  digitalWrite(DONE, LOW);

}

void loop() {

}
